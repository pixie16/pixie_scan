#include <iostream>

// PixieCore libraries
#include "Unpacker.hpp"
#include "ScanMain.hpp"

// Local files
#include "Scanner.hpp"

/// Process all events in the event list.
void Scanner::ProcessRawEvent(){
	ChannelEvent *current_event = NULL;
	ChannelEventPair *current_pair = NULL;
	
	// Fill the processor event deques with events
	while(!rawEvent.empty()){
		current_event = rawEvent.front();
		rawEvent.pop_front(); // Remove this event from the raw event deque.
		
		// Check that this channel event exists.
		if(!current_event){ continue; }

		// Fill the output histograms.
		chanCounts->Fill(current_event->chanNum, current_event->modNum);
		chanEnergy->Fill(current_event->energy, current_event->modNum*16+current_event->chanNum);

		if(!raw_event_mode){ // Standard operation. Individual processors will handle output
			// Link the channel event to its corresponding map entry.
			current_pair = new ChannelEventPair(current_event, mapfile->GetMapEntry(current_event));
		
			// Pass this event to the correct processor
			if(current_pair->entry->type == "ignore" || !handler->AddEvent(current_pair)){ // Invalid detector type. Delete it
				delete current_pair;
			}
		
			// This channel is a start signal. Due to the way ScanList
			// packs the raw event, there may be more than one start signal
			// per raw event.
			if(current_pair->entry->tag == "start"){ 
				handler->AddStart(current_pair);
			}
		}
		else{ // Raw event mode operation. Dump raw event information to root file.
			structure.Append(current_event->modNum, current_event->chanNum, current_event->time, current_event->energy);
			waveform.Append(current_event->trace);
			delete current_pair;
		}
	}
	
	if(!raw_event_mode){
		// Call each processor to do the processing. Each
		// processor will remove the channel events when finished.
		if(handler->Process()){
			// This event had at least one valid signal
			root_tree->Fill();
		}
		
		// Zero all of the processors.
		handler->ZeroAll();
	}
	else{
		root_tree->Fill();
		structure.Zero();
		waveform.Zero();
	}
	
	// Check for the need to update the online canvas.
	if(online_mode){
		if(events_since_last_update >= events_between_updates){
			online->Refresh();
			events_since_last_update = 0;
			std::cout << "refresh!\n";
		}
		else{ events_since_last_update++; }
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
