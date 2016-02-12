#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <string>

class Scanner : public Unpacker{
public:
    Scanner();
    Scanner(std::string fname_);
    ~Scanner();
    
    /// Initialize the map file, the config file, the processor handler, and add all of the required processors.
    bool Initialize(std::string prefix_="");
    
    /// Return the syntax string for this program.
    void SyntaxStr(const char *name_, std::string prefix_="");
    
    /// Scan input arguments and set class variables.
    bool SetArgs(std::deque<std::string> &args_, std::string &filename_);
    
private:
    int events_since_last_update; /// The number of processed events since the last online histogram update.
    int events_between_updates; /// The number of events to process before updating online histograms.
    
    bool force_overwrite;	
    bool raw_event_mode;
    bool online_mode;
    
    std::string output_filename;
    
    /// Process all events in the event list.
    void ProcessRawEvent();	
};//class Scanner 

/// Return a pointer to a new Scanner object.
Unpacker *GetCore(){ return (Unpacker*)(new Scanner()); }

#endif
