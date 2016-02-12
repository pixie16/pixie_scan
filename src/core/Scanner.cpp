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
	force_overwrite = false;
	raw_event_mode = false;
	online_mode = false;
	output_filename = "out.root";
	events_since_last_update = 0;
	events_between_updates = 5000;
}

Scanner::~Scanner(){
	if(init){
		// Do some cleanup stuff.
	}

	Close(); // Close the Unpacker object.
}

/// Initialize the map file, the config file, the processor handler, and add all of the required processors.
bool Scanner::Initialize(std::string prefix_){
	if(init)
	    return(false);
	    
	// Code taken from drrsub_ in Initialize.cpp
	try{
		// Read in the name of the his file
		char hisFileName[32];
		//GetArgument(1, hisFileName, 32); // FIX THIS! NO GETARGUMENT IN C++!!!
		std::string temp = "dummy";//hisFileName;
		temp = temp.substr(0, temp.find_first_of(" "));
		std::stringstream name;
		name << temp;

		output_his = new OutputHisFile(name.str());
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
		std::cout << "Exception caught at Initialize:" << std::endl;
		std::cout << "\t" << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	
	if(online_mode){
	    //ADD SOME STUFFHERE
	}
	return(init = true);
}

/// Return the syntax string for this program.
void Scanner::SyntaxStr(const char *name_, std::string prefix_){ 
	std::cout << prefix_ << "SYNTAX: " << std::string(name_) << " <options> <input>\n"; 
}       

/**
 * \param[in] args_
 * \param[out] filename
 */
bool Scanner::SetArgs(std::deque<std::string> &args_, std::string &filename){
	std::string current_arg;
	while(!args_.empty()){
		current_arg = args_.front();
		args_.pop_front();
		if(current_arg == "-shm"){
			std::cout << "pixie_ldf_c: Using online mode.\n";
			online_mode = true;
		}
		else{ filename = current_arg; }
	}
	
	return true;
}

int main(int argc, char *argv[]){
	ScanMain scan_main((Unpacker*)(new Scanner()));
	
	scan_main.SetMessageHeader("pixie_ldf_c: ");

	return scan_main.Execute(argc, argv);
}
