#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <string>

#include "Structures.h"

class MapFile;
class ConfigFile;
class ProcessorHandler;
class OnlineProcessor;
class Plotter;

class Scanner : public Unpacker{
  private:
	MapFile *mapfile;
	ConfigFile *configfile;
	ProcessorHandler *handler;
	OnlineProcessor *online;
	
	RawEventStructure structure; /// Root data structure for storing raw channel event information.
	RawEventWaveform waveform; /// Root data structure for storing raw waveforms.

	Plotter *chanCounts; /// 2d root histogram to store number of total channel counts found.
	Plotter *chanEnergy; /// 2d root histogram to store the energy spectra from all channels.
	
	int events_since_last_update; /// The number of processed events since the last online histogram update.
	int events_between_updates; /// The number of events to process before updating online histograms.
	
	bool force_overwrite;	
	bool raw_event_mode;
	bool online_mode;
	bool use_root_fitting;
	
	std::string output_filename;
	
	/// Process all events in the event list.
	void ProcessRawEvent();
	
  public:
	Scanner();
	
	Scanner(std::string fname_);
	
	~Scanner();

	/// Initialize the map file, the config file, the processor handler, and add all of the required processors.
	bool Initialize(std::string prefix_="");

	/// Return the syntax string for this program.
	void SyntaxStr(const char *name_, std::string prefix_="");
	
	/// Print a command line help dialogue for recognized command line arguments.
	void ArgHelp(std::string prefix_="");
	
	/// Print an in-terminal help dialogue for recognized commands.
	void CmdHelp(std::string prefix_="");
	
	/// Scan input arguments and set class variables.
	bool SetArgs(std::deque<std::string> &args_, std::string &filename_);

	/// Print a status message.	
	void PrintStatus(std::string prefix_="");

	/** Search for an input command and perform the desired action.
	  * 
	  * \return True if the command is valid and false otherwise.
	  */
	bool CommandControl(std::string cmd_, const std::vector<std::string> &args_);
};

/// Return a pointer to a new Scanner object.
Unpacker *GetCore(){ return (Unpacker*)(new Scanner()); }

#endif
