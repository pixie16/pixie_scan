#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>

#include <cstring>
#include <ctime>

#include <unistd.h>
#include <sys/times.h>

#include "pixie16app_defs.h"

// PixieCore libraries
#include "Unpacker.hpp" 
#include "ScanMain.hpp"
#include "PixieEvent.hpp"

// Local files
#include "Scanner.hpp"
#include "RawEvent.hpp"
#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "TreeCorrelator.hpp"
#include "DammPlotIds.hpp"

using namespace std;
using namespace dammIds::raw;

// Contains event information, the information is filled in ScanList() and is
// referenced in DetectorDriver.cpp, particularly in ProcessEvent().
RawEvent rawev;

/** Enumeration defining points when we should do some basic plotting */
enum HistoPoints {BUFFER_START, BUFFER_END, EVENT_START = 10, EVENT_CONTINUE};

// Define a pointer to an OutputHisFile for later use.
OutputHisFile *output_his = NULL;


/**
 * At various points in the processing of data in ScanList(), HistoStats() is
 * called to increment some low level pixie16 informational and diagnostic
 * spectra.  The list of spectra filled includes runtime in second and
 * milliseconds, the deadtime, time between events, and time width of an event.
 * \param [in] id : the id of the channel
 * \param [in] diff : The difference between current clock and last one
 * \param [in] clock : The current clock
 * \param [in] event : The type of event we are dealing with
 */
void HistoStats(unsigned int id, double diff, double clock, HistoPoints event) {
    static const int specNoBins = SE;

    static double start, stop;
    static int count;
    static double firstTime = 0.;
    static double bufStart;

    double runTimeSecs   = (clock - firstTime) *
                           Globals::get()->clockInSeconds();
    int    rowNumSecs    = int(runTimeSecs / specNoBins);
    double remainNumSecs = runTimeSecs - rowNumSecs * specNoBins;

    double runTimeMsecs   = runTimeSecs * 1000;
    int    rowNumMsecs    = int(runTimeMsecs / specNoBins);
    double remainNumMsecs = runTimeMsecs - rowNumMsecs * specNoBins;

    static double bufEnd = 0, bufLength = 0;
    // static double deadTime = 0 // not used
    DetectorDriver* driver = DetectorDriver::get();
    Messenger messenger;
    stringstream ss;

    if (firstTime > clock) {
        ss << "Backwards clock jump detected: prior start " << firstTime
           << ", now " << clock;
        messenger.warning(ss.str());
        ss.str("");
        // detect a backwards clock jump which occurs when some of the
        //   last buffers of a previous run sneak into the beginning of the
        //   next run, elapsed time of last buffers is usually small but
        //   just in case make some room for it
        double elapsed = stop - firstTime;
        // make an artificial 10 second gap by
        //   resetting the first time accordingly
        firstTime = clock - 10 / Globals::get()->clockInSeconds() - elapsed;
        ss << elapsed * Globals::get()->clockInSeconds()
           << " prior seconds elapsed "
           << ", resetting first time to " << firstTime;
        messenger.detail(ss.str());
        ss.str("");
    }

    switch (event) {
        case BUFFER_START:
            bufStart = clock;
            if(firstTime == 0.) {
                firstTime = clock;
            } else if (bufLength != 0.){
                //plot time between buffers as a function
                //of time - dead time spectrum
                // deadTime += (clock - bufEnd)*pixie::clockInSeconds;
                // plot(DD_DEAD_TIME_CUMUL,remainNumSecs,rownum,int(deadTime/runTimeSecs));
                driver->plot(dammIds::raw::DD_BUFFER_START_TIME, remainNumSecs,
                             rowNumSecs, (clock-bufEnd)/bufLength*1000.);
            }
            break;
        case BUFFER_END:
            driver->plot(D_BUFFER_END_TIME, (stop - bufStart) *
                                      Globals::get()->clockInSeconds() * 1000);
            bufEnd = clock;
            bufLength = clock - bufStart;
        case EVENT_START:
            driver->plot(D_EVENT_LENGTH, stop - start);
            driver->plot(D_EVENT_GAP, diff);
            driver->plot(D_EVENT_MULTIPLICITY, count);

            start = stop = clock; // reset the counters
            count = 1;
            break;
        case EVENT_CONTINUE:
            count++;
            if(diff > 0.) {
                driver->plot(D_SUBEVENT_GAP, diff + 100);
            }
            stop = clock;
            break;
        default:
            ss << "Unexpected type " << event << " given to HistoStats";
            messenger.warning(ss.str());
            ss.str("");
    }

    //fill these spectra on all events, id plots and runtime.
    // Exclude event type 0/1 since it will also appear as an
    // event type 11
    if ( event != BUFFER_START && event != BUFFER_END ){
        driver->plot(DD_RUNTIME_SEC, remainNumSecs, rowNumSecs);
        driver->plot(DD_RUNTIME_MSEC, remainNumMsecs, rowNumMsecs);
        //fill scalar spectrum (per second)
        driver->plot(D_HIT_SPECTRUM, id);
        driver->plot(D_SCALAR + id, runTimeSecs);
    }
}

/// Process all events in the event list. Replaces hissub_sec/ScanList
void Scanner::ProcessRawEvent(){
    DetectorDriver* driver = DetectorDriver::get();
    DetectorLibrary* modChan = DetectorLibrary::get();
    PixieEvent *current_event = NULL;
    
    // local variable for the detectors used in a given event
    set<string> usedDetectors;
    Messenger m;
    stringstream ss;

    // local variables for the times of the current event, previous
    // event and time difference between the two
    double diffTime = 0;
    deque<PixieEvent*>::iterator iEvent = rawEvent.begin();

        // Initialize the scan program before the first event
    if(counter == 0){
	// Retrieve the current time for use later to determine the total running time of the analysis.
	m.start("Initializing scan");
	
	/*string revision = Globals::get()->revision();
	// Initialize function pointer to point to
	// correct version of ReadBuffData
	if(revision == "F")
	ReadBuffData = ReadBuffDataF;
	if (revision == "D" || revision == "DF")
	ReadBuffData = ReadBuffDataD;
	else if (revision == "A")
	ReadBuffData = ReadBuffDataA;*/
	
	/*clockBegin = times(&tmsBegin);
	  
	  ss << "First buffer at " << clockBegin << " sys time";
	  m.detail(ss.str());
	  ss.str("");*/
	
	// After completion the descriptions of all channels are in the modChan
	// vector, the DetectorDriver and rawevent have been initialized with the
	// detectors that will be used in this analysis.
	modChan->PrintUsedDetectors(rawev);
	driver->Init(rawev);
	
	/* Make a last check to see that everything is in order for the driver
	 * before processing data. SanityCheck function throws exception if
	 * something went wrong.
	 */
	try{
	    driver->SanityCheck(); 
	} catch(GeneralException &e){
	    m.fail();
	    std::cout << "Exception caught while checking DetectorDriver" << " sanity in PixieStd" << std::endl;
	    std::cout << "\t" << e.what() << std::endl;
	    exit(EXIT_FAILURE);
	} catch(GeneralWarning &w){
	    std::cout << "Warning caught during checking DetectorDriver" << " at PixieStd" << std::endl;
	    std::cout << "\t" << w.what() << std::endl;
	}
	
	//lastVsn=-1; // set last vsn to -1 so we expect vsn 0 first
	  
	// ss << "Init at " << times(&tmsBegin) << " sys time.";
    // 	m.detail(ss.str());
    // 	m.done();
    } //if(counter == 0)
    
    bool IsFirstEvt = true;
    double lastTime, currTime;
    unsigned int id;
    static double firstTime;
    // Fill the raw event with ChanEvents
    while(!rawEvent.empty()){
	if(IsFirstEvt) {
	    lastTime = (*iEvent)->eventTime;
	    currTime = lastTime;
	    id = (*iEvent)->getID();

	    /* KM
	     * Save time of the beginning of the file,
	     * this is needed for the rejection regions */
	    firstTime = lastTime;
	    HistoStats(id, diffTime, lastTime, BUFFER_START);
	    IsFirstEvt = false;
	}
	
	current_event = rawEvent.front();
	rawEvent.pop_front(); // Remove this event from the raw event deque.
	// Check that this channel event exists.
	if(!current_event)
	    continue;
	
	// Do something with the current event.
	ChanEvent *event = new ChanEvent(current_event);
	
        unsigned int id = event->GetID();
        if (id == pixie::U_DELIMITER) {
            std::stringstream ss; 
	    ss << "pattern 0 ignore";
            m.warning(ss.str());
            ss.str("");
            continue;
        }
        if ((*modChan)[id].GetType() == "ignore")
            continue;

	usedDetectors.insert((*modChan)[id].GetType());
	rawev.AddChan(event);
	HistoStats(id, diffTime, lastTime, EVENT_CONTINUE);
    }
    
    driver->ProcessEvent(rawev);
    
    //after processing zero the rawevent variable
    rawev.Zero(usedDetectors);
    usedDetectors.clear();
    
    // Now clear all places in correlator (if resetable type)
    for (std::map<std::string, Place*>::iterator it = TreeCorrelator::get()->places_.begin(); it != TreeCorrelator::get()->places_.end(); ++it)
	if ((*it).second->resetable())
	    (*it).second->reset();
    
    //rawev.ClearAndDelete();
    counter++;
}

Scanner::Scanner(){
    output_fname = "output";
}

Scanner::~Scanner(){
    if(init)
	delete output_his;
    Close(); // Close the Unpacker object.
}

/// Initialize the map file, the config file, the processor handler, and add all of the required processors.
bool Scanner::Initialize(std::string prefix_){
    if(init)
	return(false);
    
    try{
	// Read in the name of the his file.
	output_his = new OutputHisFile(output_fname);
	
	// Set the output his file debug mode, if the user asked for it.
	if(scan_main->DebugMode())
	    output_his->SetDebugMode(true);
	
	/** The DetectorDriver constructor will load processors
	 *  from the xml configuration file upon first call.
	 *  The DeclarePlots function will instantiate the DetectorLibrary
	 *  class which will read in the "map" of channels.
	 *  Subsequently the raw histograms, the diagnostic histograms
	 *  and the processors and analyzers plots are declared.
	 *
	 *  Note that in the PixieStd the Init function of DetectorDriver
	 *  is called upon first buffer. This include reading in the
	 *  calibration and walk correction factors.
	 */
	DetectorDriver::get()->DeclarePlots();
	output_his->Finalize();
    } 
    catch(std::exception &e){
	// Any exceptions will be intercepted here
	std::cout << prefix_ << "Exception caught at Initialize:" << std::endl;
	std::cout << prefix_ << e.what() << std::endl;
	exit(EXIT_FAILURE);
    }
    return(init = true);
}

/// Return the syntax string for this program.
void Scanner::SyntaxStr(const char *name_, std::string prefix_){ 
    std::cout << prefix_ << "SYNTAX: " << std::string(name_) << " [input-fname] <options> <output-prefix>\n"; 
}       

/** 
 *	\param[in] prefix_ 
 */
void Scanner::CmdHelp(std::string prefix_){
    std::cout << prefix_ << "flush - Flush histogram entries to file.\n";
    std::cout << prefix_ << "zero  - Zero the output histogram (SLOW! Be patient...).\n";
}

/**
 * \param[in] args_
 * \param[out] filename
 */
bool Scanner::SetArgs(std::deque<std::string> &args_, std::string &filename){
    std::string current_arg;
    int count = 0;
    while(!args_.empty()){
	current_arg = args_.front();
	args_.pop_front();
	if(count == 0){ filename = current_arg; } // Set the input filename.
	else if(count == 1){ output_fname = current_arg; } // Set the output filename prefix.
	count++;
    }
    
    return true;
}

/** Search for an input command and perform the desired action.
 * 
 * \return True if the command is valid and false otherwise.
 */
bool Scanner::CommandControl(std::string cmd_, const std::vector<std::string> &args_){
    if(cmd_ == "flush"){
	std::cout << message_head << "Flushing histogram entries to file.\n";
	output_his->Flush();
    }
    else if(cmd_ == "zero"){
	if(args_.size() >= 1){
	    int his_id = atoi(args_.at(0).c_str());
	    if(output_his->Zero(his_id)){ std::cout << message_head << "Zeroed histo id " << his_id << "...\n"; }
	    else{ std::cout << message_head << "Failed to zero histo with id " << his_id << "!\n"; }
	}
	else{ // Zero all histos.
	    std::cout << message_head << "Zeroing output .his file. This may take some time...\n";
	    output_his->Zero();
	}
    }
    else{ return false; }
    
    return true;
}

int main(int argc, char *argv[]){
    // Define a new unpacker object.
    Unpacker *scanner = (Unpacker*)(new Scanner());
    
    // Setup the ScanMain object and link it to the unpacker object.
    ScanMain scan_main(scanner);
    
    // Link the unpacker object back to the ScanMain object so we may
    // access its command line arguments and options.
    scanner->SetScanMain(&scan_main);
    
    // Set the output message prefix.
    scan_main.SetMessageHeader("pixie_ldf_c: ");
    
    // Run the main loop.
    return scan_main.Execute(argc, argv);
}
