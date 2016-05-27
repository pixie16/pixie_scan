/** \file IS600Processor.hpp
 * \brief A class to process data from ISOLDE 599 and 600 experiments using
 * VANDLE
 *
 *\author S. V. Paulauskas
 *\date July 14, 2015
 */
#ifndef __IS600PROCESSOR_HPP_
#define __IS600PROCESSOR_HPP_
#include <fstream>

#include "EventProcessor.hpp"

#ifdef useroot
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH1D.h>
#endif

/// Class Analyze for ISOLDE experiments 599 and 600
class IS600Processor : public EventProcessor {
public:
    /** Default Constructor */
    IS600Processor();
    /** Default Destructor */
    ~IS600Processor();
    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    /** Process the event
    * \param [in] event : the event to process
    * \return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);
private:
};
#endif //#ifndef __IS600PROCESSOR_HPP__
