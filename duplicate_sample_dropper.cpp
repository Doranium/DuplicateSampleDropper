#include "duplicate_sample_dropper.h"
#include <cstring>

ADTF_PLUGIN("Duplicate Sample Dropper", cDuplicateSampleDropperFilter);


cDuplicateSampleDropperFilter::cDuplicateSampleDropperFilter()
{
    CreateInputPin("sample", true, false);
    m_pWriter = CreateOutputPin("sampleOut");

    // logical combinations for active checks
    m_eOperator.SetValueList({ {OR, "OR"} , {AND, "AND"} });
    m_eOperator.SetDescription("Logical combination operator for active checks");
    RegisterPropertyVariable("Logical connective", m_eOperator);

    RegisterPropertyVariable("Check timestamps", m_checkTimestamp);
    RegisterPropertyVariable("Check size", m_checkSampleSize);
    RegisterPropertyVariable("Check raw sample data", m_checkEntireSampleData);

    SetDescription("If two consecutive samples are equal, only the first one is send to output pin.");
}


tResult cDuplicateSampleDropperFilter::ProcessInput(ISampleReader* /*pReader*/,
    const iobject_ptr<const ISample>& pSample)
{
    /// special case handling
    if (pSample == nullptr)             // no sample data received, maybe just a trigger
        RETURN_NOERROR;

    if (previous_sample == nullptr)     // this is the first sample -> always forward
        return forward_sample(pSample);

    if (!(m_checkTimestamp              // no checks active
        || m_checkSampleSize
        || m_checkEntireSampleData))
        return forward_sample(pSample);

    switch (m_eOperator)
    {
    case OR:
        if (
            (m_checkTimestamp && is_time_diff(pSample))
            ||
            (m_checkSampleSize && is_size_diff(pSample))
            ||
            (m_checkEntireSampleData && is_data_diff(pSample))
            )
        {
            return forward_sample(pSample);
        }
        return drop_sample(pSample);

    case AND:
        if (
            !(m_checkTimestamp && !is_time_diff(pSample))
            &&
            !(m_checkSampleSize && !is_size_diff(pSample))
            &&
            !(m_checkEntireSampleData && !is_data_diff(pSample))
            )
        {
            return forward_sample(pSample);
        }
        return drop_sample(pSample);

    default:
        return ERR_INVALID_ARG; // in case of misconfiguration
    }
}


tResult cDuplicateSampleDropperFilter::AcceptType(adtf::streaming::ISampleReader * pReader, const adtf::ucom::iobject_ptr<const adtf::streaming::IStreamType>& pType)
{
    return m_pWriter->ChangeType(pType);
}


// several helper functions

bool cDuplicateSampleDropperFilter::is_time_diff(const iobject_ptr<const ISample>& pSample)
{
    auto prev_sample_time = get_sample_time(previous_sample);
    auto current_sample_time = get_sample_time(pSample);
    // missing tNanoSeconds unequal operator in 3.6.3, fixed in 3.7
    return !(prev_sample_time == current_sample_time);
}

bool cDuplicateSampleDropperFilter::is_size_diff(const iobject_ptr<const ISample>& pSample)
{
    sample_data<tUInt8> current_raw_data(pSample);
    auto current_sample_size = current_raw_data.GetDataSize();

    sample_data<tUInt8> prev_raw_data(previous_sample);
    auto prev_sample_size = prev_raw_data.GetDataSize();

    return (current_sample_size != prev_sample_size);
}

bool cDuplicateSampleDropperFilter::is_data_diff(const iobject_ptr<const ISample>& pSample)
{
    typedef tUInt8 data_block_type;    // block wise compare of data
    sample_data<data_block_type> current_data(pSample);
    sample_data<data_block_type> prev_data(previous_sample);

    auto current_size = current_data.GetDataSize();
    auto prev_size = prev_data.GetDataSize();

    if (current_size != prev_size) return true;

    auto current_data_ptr = current_data.GetDataPtr();
    auto prev_data_ptr = prev_data.GetDataPtr();

    // C++ already has a compare function for bare data blocks
    int cmp_result = std::memcmp(current_data_ptr, prev_data_ptr, current_size);
    return (cmp_result != 0);
}

tResult cDuplicateSampleDropperFilter::forward_sample(const iobject_ptr<const ISample>& pSample)
{
    RETURN_IF_FAILED(m_pWriter->Write(pSample));
    RETURN_IF_FAILED(m_pWriter->ManualTrigger());
    previous_sample = pSample;
    RETURN_NOERROR;
}

tResult cDuplicateSampleDropperFilter::drop_sample(const iobject_ptr<const ISample>& pSample)
{
    previous_sample = pSample;
    RETURN_NOERROR;
}
