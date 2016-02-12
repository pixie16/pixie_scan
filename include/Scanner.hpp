#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <string>

class ScanMain;

class Scanner : public Unpacker{
public:
    Scanner();
    Scanner(std::string fname_);
    ~Scanner();
    
    /// Initialize the map file, the config file, the processor handler, and add all of the required processors.
    bool Initialize(std::string prefix_="");
    
    /// Return the syntax string for this program.
    void SyntaxStr(const char *name_, std::string prefix_="");

	/// Print an in-terminal help dialogue for recognized commands.
	void CmdHelp(std::string prefix_="");
	
	/// Scan input arguments and set class variables.
	bool SetArgs(std::deque<std::string> &args_, std::string &filename_);
	
	/** Search for an input command and perform the desired action.
	  * 
	  * \return True if the command is valid and false otherwise.
	  */
	bool CommandControl(std::string cmd_, const std::vector<std::string> &args_);
    
private:
	std::string output_fname; /// The output histogram filename prefix.

	unsigned int counter; /// The number of times ProcessRawEvent is called.

    /// Process all events in the event list.
    void ProcessRawEvent();	
};//class Scanner 

/// Return a pointer to a new Scanner object.
Unpacker *GetCore(){ return (Unpacker*)(new Scanner()); }

#endif
