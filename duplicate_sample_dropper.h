#pragma once

#include <adtffiltersdk/adtf_filtersdk.h>

using namespace adtf::base;
using namespace adtf::streaming;
using namespace adtf::filter;

/*
 * Two walk in, maybe only one comes out.
 * Check if two consecutive samples are identical ..
 * .. and in case drop the second one (.. and third and..)
 *
 * If no check is enabled, no sample will be dropped
*/
class cDuplicateSampleDropperFilter : public cFilter
{
public:
    ADTF_CLASS_ID_NAME(cDuplicateSampleDropperFilter,
        "duplicate_sample_dropper.filter.ad.cid",
        "Duplicate sample dropper");

    // Is this a constructor? Have a guess what it does!
    cDuplicateSampleDropperFilter();

    /*
     * Active checks can be combined logically AND or OR to drop a sample
     * OR : at least one check has to pass
     * AND: all checks have to be true
     */
    enum tOperator : tUInt32
    {
        OR = 1,
        AND = 2,
    };

    // the main sample processing method
    tResult ProcessInput(ISampleReader* pReader,
        const iobject_ptr<const ISample>& pSample) override;

    // Type changes are always passed through.
    tResult AcceptType(adtf::streaming::ISampleReader* pReader,
        const adtf::ucom::iobject_ptr<const adtf::streaming::IStreamType>& pType) override;

private:

    // check if sample time of previous and current sample are different
    bool is_time_diff(const iobject_ptr<const ISample>&  pSample);

    // check if sample size of previous and current sample are different
    // this is for special cases like compressed data or network packages
    // NOTE: often all samples in a stream have the same size -> all but first are dropped
    bool is_size_diff(const iobject_ptr<const ISample>&  pSample);

    // check if sample raw data of previous and current sample are different
    bool is_data_diff(const iobject_ptr<const ISample>&  pSample);

    /*
     * The current sample is not a duplicate
     * * send to output pin
     * * keep sample for next iteration
     * A ProcessInput call should only end with forward_sample() or drop_sample().
     */
    tResult forward_sample(const iobject_ptr<const ISample>&  pSample);

    /*
     * The sample is a duplicate of its predecessor
     * * keep sample for next iteration but do NOT send out
     * A ProcessInput call should only end with drop_sample() or forward_sample().
     */
    tResult drop_sample(const iobject_ptr<const ISample>&  pSample);

    property_variable<tBool> m_checkTimestamp = true;
    property_variable<tBool> m_checkSampleSize = false;
    property_variable<tBool> m_checkEntireSampleData = false;
    property_variable<tOperator> m_eOperator = OR;

    // hold the last received data sample for checks
    adtf::ucom::object_ptr<const ISample>previous_sample;

    ISampleWriter* m_pWriter;
};
