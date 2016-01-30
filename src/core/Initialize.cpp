/** \file Initialize.cpp
 * \brief C++ wrapper to upak's hd1d/hd2d functions
 * \author David Miller
 * \date Aug. 2009
 */
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "DetectorDriver.hpp"
#include "GetArguments.hpp"
#include "Globals.hpp"
#include "hisFile.hpp"

using namespace std;

/*! Defines the main interface with the SCANOR library, the program essentially
 * starts here.
 * \param [in] iexist : unused paramter from SCANOR call
 */
extern "C" void drrsub_(unsigned int& iexist) {
    try {
        //Read in the name of the his file
	char hisFileName[32];
	GetArgument(1, hisFileName, 32);
	string temp = hisFileName;
	temp = temp.substr(0, temp.find_first_of(" "));
	stringstream name;
	name << temp;

	output_his = new OutputHisFile(name.str());

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
	output_his->Close();
    } catch (std::exception &e) {
        // Any exceptions will be intercepted here
        std::cout << "Exception caught at Initialize:" << std::endl;
        std::cout << "\t" << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}
