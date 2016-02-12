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
	use_root_fitting = false;
	mapfile = NULL;
	configfile = NULL;
	handler = NULL;
	online = NULL;
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
		
			delete mapfile;
			delete configfile;
			delete handler;
		}
		
		// If the root file is open, write the tree and histogram.
		if(root_file->IsOpen()){
			std::cout << "Scanner: Writing " << root_tree->GetEntries() << " entries to root file.\n";
			root_file->cd();
			root_tree->Write();
			chanCounts->GetHist()->Write();
			chanEnergy->GetHist()->Write();
			root_file->Close();
		}
		delete root_file;
		
		if(online){ delete online; }
	}

	Close(); // Close the Unpacker object.
}

/// Initialize the map file, the config file, the processor handler, and add all of the required processors.
bool Scanner::Initialize(std::string prefix_){
	if(init){ return false; }

	// CALL DRR SUB HERE (in Initialize.cpp)
	drrsub_();
	
	if(online_mode){
	    //ADD SOME STUFFHERE
	}
	return (init = true);
}

/// Return the syntax string for this program.
void Scanner::SyntaxStr(const char *name_, std::string prefix_){ 
	std::cout << prefix_ << "SYNTAX: " << std::string(name_) << " <options> <input>\n"; 
}
	
/**
 * \param[in] prefix_
 */
void Scanner::ArgHelp(std::string prefix_){
	std::cout << prefix_ << "--force-overwrite - Force an overwrite of the output root file if it exists (default=false)\n";
	std::cout << prefix_ << "--raw-event-mode  - Write raw channel information to the output root file (default=false)\n";
	std::cout << prefix_ << "--online-mode     - Plot online root histograms for monitoring data (default=false)\n";
	std::cout << prefix_ << "--fitting         - Use root fitting for high resolution timing (default=false)\n";
}

/** 
 *	\param[in] prefix_ 
 */
void Scanner::CmdHelp(std::string prefix_){
	if(online_mode){
		std::cout << prefix_ << "refresh <update>           - Set refresh frequency of online diagnostic plots (default=5000).\n";
		std::cout << prefix_ << "list                       - List all plottable online histograms.\n";
		std::cout << prefix_ << "set [index] [hist]         - Set the histogram to draw to part of the canvas.\n";
		std::cout << prefix_ << "xlog [index]               - Toggle the x-axis log/linear scale of a specified histogram.\n";
		std::cout << prefix_ << "ylog [index]               - Toggle the y-axis log/linear scale of a specified histogram.\n";
		std::cout << prefix_ << "zlog [index]               - Toggle the z-axis log/linear scale of a specified histogram.\n"; 
		std::cout << prefix_ << "xrange [index] [min] [max] - Set the x-axis range of a histogram displayed on the canvas.\n";
		std::cout << prefix_ << "yrange [index] [min] [max] - Set the y-axis range of a histogram displayed on the canvas.\n";
		std::cout << prefix_ << "unzoom [index] <axis>      - Unzoom the x-axis, the y-axis, or both.\n";
		std::cout << prefix_ << "range [index] [xmin] [xmax] [ymin] [ymax] - Set the range of the x and y axes.\n";
	}
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
		
		if(current_arg == "--force-overwrite"){
			std::cout << "Scanner: Forcing overwrite of output file.\n";
			force_overwrite = true;
		}
		else if(current_arg == "--raw-event-mode"){
			std::cout << "Scanner: Using raw event output mode.\n";
			raw_event_mode = true;
		}
		else if(current_arg == "--online-mode"){
			std::cout << "Scanner: Using online mode.\n";
			online_mode = true;
		}
		else if(current_arg == "--fitting"){
			std::cout << "Scanner: Toggling root fitting ON.\n";
			use_root_fitting = true;
		}
		else{ filename = current_arg; }
	}
	
	return true;
}

/** Search for an input command and perform the desired action.
  * 
  * \return True if the command is valid and false otherwise.
  */
bool Scanner::CommandControl(std::string cmd_, const std::vector<std::string> &args_){
	if(online_mode){
		if(cmd_ == "refresh"){
			if(args_.size() >= 1){
				int frequency = atoi(args_.at(0).c_str());
				if(frequency > 0){ 
					std::cout << message_head << "Set canvas update frequency to " << frequency << " events.\n"; 
					events_between_updates = frequency;
				}
				else{ std::cout << message_head << "Failed to set canvas update frequency to " << frequency << " events!\n"; }
			}
			else{ online->Refresh(); }
		}
		else if(cmd_ == "list"){
			online->PrintHists();
		}
		else if(cmd_ == "set"){
			if(args_.size() >= 2){
				int index1 = atoi(args_.at(0).c_str());
				int index2 = atoi(args_.at(1).c_str());
				if(online->ChangeHist(index1, args_.at(1))){ std::cout << message_head << "Set TPad " << index1 << " to histogram '" << args_.at(1) << "'.\n"; }
				else if(online->ChangeHist(index1, index2)){ std::cout << message_head << "Set TPad " << index1 << " to histogram ID " << index2 << ".\n"; }
				else{ std::cout << message_head << "Failed to set TPad " << index1 << " to histogram '" << args_.at(1) << "'!\n"; }
			}
			else{
				std::cout << message_head << "Invalid number of parameters to 'set'\n";
				std::cout << message_head << " -SYNTAX- set [index] [hist]\n";
			}
		}
		else if(cmd_ == "xlog"){
			if(args_.size() >= 1){
				int index = atoi(args_.at(0).c_str());
				if(online->ToggleLogX(index)){ std::cout << message_head << "Successfully toggled x-axis log scale for TPad " << index << ".\n"; }
				else{ std::cout << message_head << "Failed to toggle x-axis log scale for TPad " << index << ".\n"; }
			}
			else{
				std::cout << message_head << "Invalid number of parameters to 'xlog'\n";
				std::cout << message_head << " -SYNTAX- xlog [index]\n";
			}
		}
		else if(cmd_ == "ylog"){
			if(args_.size() >= 1){
				int index = atoi(args_.at(0).c_str());
				if(online->ToggleLogY(index)){ std::cout << message_head << "Successfully toggled y-axis log scale for TPad " << index << ".\n"; }
				else{ std::cout << message_head << "Failed to toggle y-axis log scale for TPad " << index << ".\n"; }
			}
			else{
				std::cout << message_head << "Invalid number of parameters to 'ylog'\n";
				std::cout << message_head << " -SYNTAX- ylog [index]\n";
			}
		}
		else if(cmd_ == "zlog"){
			if(args_.size() >= 1){
				int index = atoi(args_.at(0).c_str());
				if(online->ToggleLogZ(index)){ std::cout << message_head << "Successfully toggled z-axis log scale for TPad " << index << ".\n"; }
				else{ std::cout << message_head << "Failed to toggle z-axis log scale for TPad " << index << ".\n"; }
			}
			else{
				std::cout << message_head << "Invalid number of parameters to 'zlog'\n";
				std::cout << message_head << " -SYNTAX- zlog [index]\n";
			}
		}
		else if(cmd_ == "xrange"){
			if(args_.size() >= 3){
				int index = atoi(args_.at(0).c_str());
				float min = atof(args_.at(1).c_str());
				float max = atof(args_.at(2).c_str());
				if(max > min){ 
					if(online->SetXrange(index, min, max)){ std::cout << message_head << "Successfully set range of TPad " << index << ".\n"; }
					else{ std::cout << message_head << "Failed to set range of TPad " << index << "!\n"; }
				}
				else{ std::cout << message_head << "Invalid range for x-axis [" << min << ", " << max << "]\n"; }
			}
			else{
				std::cout << message_head << "Invalid number of parameters to 'xrange'\n";
				std::cout << message_head << " -SYNTAX- xrange [index] [min] [max]\n";
			}
		}
		else if(cmd_ == "yrange"){
			if(args_.size() >= 3){
				int index = atoi(args_.at(0).c_str());
				float min = atof(args_.at(1).c_str());
				float max = atof(args_.at(2).c_str());
				if(max > min){ 
					if(online->SetYrange(index, min, max)){ std::cout << message_head << "Successfully set range of TPad " << index << ".\n"; }
					else{ std::cout << message_head << "Failed to set range of TPad " << index << "!\n"; }
				}
				else{ std::cout << message_head << "Invalid range for y-axis [" << min << ", " << max << "]\n"; }
			}
			else{
				std::cout << message_head << "Invalid number of parameters to 'yrange'\n";
				std::cout << message_head << " -SYNTAX- yrange [index] [min] [max]\n";
			}
		}
		else if(cmd_ == "unzoom"){
			if(args_.size() >= 1){
				int index = atoi(args_.at(0).c_str());
				if(args_.size() >= 2){
					if(args_.at(1) == "x" || args_.at(1) == "X" || args_.at(1) == "0"){
						online->ResetXrange(index);
						std::cout << message_head << "Reset range of X axis.\n";
					}
					else if(args_.at(1) == "y" || args_.at(1) == "Y" || args_.at(1) == "1"){
						online->ResetYrange(index);
						std::cout << message_head << "Reset range of Y axis.\n";
					}
					else{ std::cout << message_head << "Unknown axis (" << args_.at(1) << ")!\n"; }
				}
				else{
					online->ResetRange(index);
					std::cout << message_head << "Reset range of X and Y axes.\n";
				}
			}
			else{
				std::cout << message_head << "Invalid number of parameters to 'unzoom'\n";
				std::cout << message_head << " -SYNTAX- unzoom [index] <axis>\n";
			}
		}
		else if(cmd_ == "range"){
			if(args_.size() >= 5){
				int index = atoi(args_.at(0).c_str());
				float xmin = atof(args_.at(1).c_str());
				float xmax = atof(args_.at(2).c_str());
				float ymin = atof(args_.at(3).c_str());
				float ymax = atof(args_.at(4).c_str());
				if(xmax > xmin && ymax > ymin){ 
					if(online->SetRange(index, xmin, xmax, ymin, ymax)){ std::cout << message_head << "Successfully set range of TPad " << index << ".\n"; }
					else{ std::cout << message_head << "Failed to set range of TPad " << index << "!\n"; }
				}
				else{ 
					if(xmax <= xmin && ymax <= ymin){
						std::cout << message_head << "Invalid range for x-axis [" << xmin << ", " << xmax;
						std::cout << "] and y-axis [" << ymin << ", " << ymax << "]\n"; 
					}
					else if(xmax <= xmin){ std::cout << message_head << "Invalid range for x-axis [" << xmin << ", " << xmax << "]\n"; }
					else{ std::cout << message_head << "Invalid range for y-axis [" << ymin << ", " << ymax << "]\n"; }
				}
			}
			else{
				std::cout << message_head << "Invalid number of parameters to 'range'\n";
				std::cout << message_head << " -SYNTAX- range [index] [xmin] [xmax] [ymin] [ymax]\n";
			}
		}
		else{ return false; }
	}
	else{ return false; }

	return true;
}

/// Print a status message.	
void Scanner::PrintStatus(std::string prefix_){ 
}

int main(int argc, char *argv[]){
	ScanMain scan_main((Unpacker*)(new Scanner()));
	
	scan_main.SetMessageHeader("Scanner: ");

	return scan_main.Execute(argc, argv);
}
