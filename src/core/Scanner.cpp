#include <iostream>

// PixieCore libraries
#include "Unpacker.hpp"
#include "ScanMain.hpp"
#include "ChannelEvent.hpp"

// Local files
#include "Scanner.hpp"
#include "DetectorDriver.hpp"

// Define a pointer to an OutputHisFile for later use.
OutputHisFile *output_his = NULL;

/// Process all events in the event list.
void Scanner::ProcessRawEvent(){
	ChannelEvent *current_event = NULL;
	
	// Fill the processor event deques with events
	while(!rawEvent.empty()){
		current_event = rawEvent.front();
		rawEvent.pop_front(); // Remove this event from the raw event deque.
		
		// Check that this channel event exists.
		if(!current_event){ continue; }
		
		// Do something with the current event.
		delete current_event;
	}
}

Scanner::Scanner(){
	output_fname = "output";
}

Scanner::~Scanner(){
	if(init){
		output_his->Close();
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
		if(scan_main->DebugMode()){ output_his->SetDebugMode(true); }

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
