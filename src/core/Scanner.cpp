#include <iostream>

// PixieCore libraries
#include "ScanMain.hpp"

// Local files
#include "Scanner.hpp"

/// Process all events in the event list.
void Scanner::ProcessRawEvent(){
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
		std::cout << "Scanner: Found " << chanCounts->GetHist()->GetEntries() << " total events.\n";
		if(!raw_event_mode){
			std::cout << "Scanner: Found " << handler->GetTotalEvents() << " start events.\n";
			std::cout << "Scanner: Total data time is " << handler->GetDeltaEventTime() << " s.\n";
		}
	}

	Close(); // Close the Unpacker object.
}

/// Initialize the map file, the config file, the processor handler, and add all of the required processors.
bool Scanner::Initialize(std::string prefix_){
	if(init)
	    return(false);

	// CALL DRR SUB HERE (in Initialize.cpp)
	drrsub_();
	
	if(online_mode){
	    //ADD SOME STUFFHERE
	}
	return(init = true);
}

/// Return the syntax string for this program.
void Scanner::SyntaxStr(const char *name_, std::string prefix_){ 
	std::cout << prefix_ << "SYNTAX: " << std::string(name_) 
		  << " <options> <input>\n"; 
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
		else if(current_arg == "-shm"){
			std::cout << "Scanner: Using online mode.\n";
			online_mode = true;
		}
		else{ filename = current_arg; }
	}
	
	return true;
}

int main(int argc, char *argv[]){
	ScanMain scan_main((Unpacker*)(new Scanner()));
	
	scan_main.SetMessageHeader("Scanner: ");

	return scan_main.Execute(argc, argv);
}
