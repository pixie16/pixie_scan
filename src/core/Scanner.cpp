#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>

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

// Contains event information, the information is filled in ScanList() and is
// referenced in DetectorDriver.cpp, particularly in ProcessEvent().
RawEvent rawev;

// Define a pointer to an OutputHisFile for later use.
OutputHisFile *output_his = NULL;

/// Process all events in the event list.
void Scanner::ProcessRawEvent(){
	//static clock_t clockBegin; // initialization time

    DetectorLibrary* modChan = DetectorLibrary::get();
    DetectorDriver* driver = DetectorDriver::get();

    // local variable for the detectors used in a given event
    std::set<std::string> usedDetectors;

	Messenger messenger;

	PixieEvent *current_event = NULL;

	// Fill the raw event with ChanEvents
	while(!rawEvent.empty()){
		current_event = rawEvent.front();
		rawEvent.pop_front(); // Remove this event from the raw event deque.
		
		// Check that this channel event exists.
		if(!current_event){ continue; }
		
		// Do something with the current event.
		ChanEvent *event = new ChanEvent(current_event);

        unsigned int id = event->GetID();
        if (id == pixie::U_DELIMITER) {
            std::stringstream ss; ss << "pattern 0 ignore";
            messenger.warning(ss.str());
            ss.str("");
            continue;
        }
        if ((*modChan)[id].GetType() == "ignore") {
            continue;
        }

		usedDetectors.insert((*modChan)[id].GetType());
		rawev.AddChan(event);
	}
	
	// Initialize the scan program before the first event
	if(counter == 0){
		// Retrieve the current time for use later to determine the total running time of the analysis.
		messenger.start("Initializing scan");

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
		messenger.detail(ss.str());
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
		try{ driver->SanityCheck(); } 
		catch(GeneralException &e){
			messenger.fail();
			std::cout << "Exception caught while checking DetectorDriver" << " sanity in PixieStd" << std::endl;
			std::cout << "\t" << e.what() << std::endl;
			exit(EXIT_FAILURE);
		} 
		catch(GeneralWarning &w){
			std::cout << "Warning caught during checking DetectorDriver" << " at PixieStd" << std::endl;
			std::cout << "\t" << w.what() << std::endl;
		}

		/*lastVsn=-1; // set last vsn to -1 so we expect vsn 0 first

		ss << "Init at " << times(&tmsBegin) << " sys time.";
		messenger.detail(ss.str());
		messenger.done();*/
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
	if(init){
		// This automatically closes the .his file.
		delete output_his;
	}

	Close(); // Close the Unpacker object.
}

/// Initialize the map file, the config file, the processor handler, and add all of the required processors.
bool Scanner::Initialize(std::string prefix_){
	if(init)
	    return(false);
	    
	// Code taken from drrsub_ in Initialize.cpp
	try{
		// Read in the name of the his file.
		output_his = new OutputHisFile(output_fname);
		
		// Set the output his file debug mode, if the user asked for it.
		//if(scan_main->DebugMode()){ 
		output_his->SetDebugMode(true); //}

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
