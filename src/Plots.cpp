/** \file Plots.cpp
 *
 * Implement a block declaration scheme for DAMM plots
 */

#include <cstring>
#include <iostream>

#include "Plots.hpp"
#include "PlotsRegister.hpp"

using namespace std;

/* create a DAMM 1D histogram
 * args are damm id, half-words per channel, param length, hist length,
 * low x-range, high x-range, and title
 */
extern "C" void hd1d_(const int &, const int &, const int &, const int &,
		      const int &, const int &, const char *, int);

/* create a DAMM 2D histogram
 * args are damm id, half-words per channel, x-param length, x-hist length
 * low x-range, high x-range, y-param length, y-hist length, low y-range
 * high y-range, and title
 */
extern "C" void hd2d_(const int &, const int &, const int &, const int &,
		      const int &, const int &, const int &, const int &,
		      const int &, const int &, const char *, int);

Plots::Plots(int aOffset, int aRange, PlotsRegister *reg)
{  
    offset = aOffset;
    range  = aRange;
    reg->Add(offset, range);
}

/**
 * Check if the id falls within the expected range
 */
bool Plots::CheckRange(int id) const
{
    return (id < range && id >= 0);
}

/**
 * Checks if id is taken
 */
bool Plots::Exists(int id) const
{
    return (idList.count(id) != 0);
}

bool Plots::Exists(const string &mne) const
{
    // Empty mnemonic is always allowed
    if (mne.size() == 0)
	return false;
    
    return (mneList.count(mne) != 0);
}

/** Constructors based on DeclareHistogram functions. */
bool Plots::DeclareHistogram1D(int dammId, int xSize, const char* title,
			       int halfWordsPerChan, int xHistLength,
			       int xLow, int xHigh, const string &mne)
{
    if (!CheckRange(dammId)) {
        cerr << "Id : " << dammId << " is outside of allowed range (" << range << ")." << endl;
        exit(EXIT_FAILURE);
    }
    if (Exists(dammId) || Exists(mne)) {
        cerr << "Histogram titled '" << title << "' requests id " 
             << dammId + offset << " which is already in use by"
             << " histogram '" << titleList[dammId] << "'." <<  endl;
        exit(EXIT_FAILURE);
    }
        
    pair<set<int>::iterator, bool> result = idList.insert(dammId);
    if (result.second == false)
        return false;
    // Mnemonic is optional and added only if longer then 0
    if (mne.size() > 0)
        mneList.insert( pair<string, int>(mne, dammId) );
    
    hd1d_(dammId + offset, halfWordsPerChan, xSize, xHistLength, xLow, xHigh, title, strlen(title));
    titleList.insert( pair<int, string>(dammId, string(title)));
    return true;
}

bool Plots::DeclareHistogram1D(int dammId, int xSize, const char* title,
			       int halfWordsPerChan /* = 2*/, const string &mne /*=empty*/ )
{
    return DeclareHistogram1D(dammId, xSize, title, halfWordsPerChan, xSize, 0, xSize - 1, mne);
}

bool Plots::DeclareHistogram1D(int dammId, int xSize, const char* title,
                               int halfWordsPerChan, int contraction, const string &mne)
{
    return DeclareHistogram1D(dammId, xSize, title, halfWordsPerChan,
			      xSize / contraction, 0, xSize / contraction - 1, mne);
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
                               const char *title, int halfWordsPerChan,
                               int xHistLength, int xLow, int xHigh,
                               int yHistLength, int yLow, int yHigh, 
			       const string &mne)
{
    if (!CheckRange(dammId)) {
        cerr << "Id : " << dammId << " is outside of allowed range (" << range << ")." << endl;
        exit(EXIT_FAILURE);
    }
    if (Exists(dammId) || Exists(mne)) {
        cerr << "Histogram titled '" << title << "' requests id " 
             << dammId + offset << " which is already in use by"
             << " histogram '" << titleList[dammId] << "'." << endl;
        exit(EXIT_FAILURE);
    }

    pair<set<int>::iterator, bool> result = idList.insert(dammId);
    if (result.second == false)
        return false;
    // Mnemonic is optional and added only if longer then 0
    if (mne.size() > 0)
        mneList.insert( pair<string, int>(mne, dammId) );
    
    hd2d_(dammId + offset, halfWordsPerChan, xSize, xHistLength, xLow, xHigh,
	  ySize, yHistLength, yLow, yHigh, title, strlen(title));
    titleList.insert( pair<int, string>(dammId, string(title)));
    return true;
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
                               const char* title,
                               int halfWordsPerChan /* = 1*/,
                               const string &mne /* = empty*/)
{
    return DeclareHistogram2D(dammId, xSize, ySize, title, halfWordsPerChan,
			      xSize, 0, xSize - 1,
			      ySize, 0, ySize - 1, mne);
}

bool Plots::DeclareHistogram2D(int dammId, int xSize, int ySize,
			       const char* title, int halfWordsPerChan,
			       int xContraction, int yContraction, const string &mne)
{
    return DeclareHistogram2D(dammId, xSize, ySize, title, halfWordsPerChan,
			      xSize / xContraction, 0, xSize / xContraction - 1,
			      ySize / yContraction, 0, ySize / yContraction - 1,
			      mne );
}


bool Plots::Plot(int dammId, double val1, double val2, double val3, const char* name)
{
    /*
      dammid - id of the damm spectrum in absence of root
      val1   - energy of a 1d spectrum
      x value in a 2d
      val2   - weight in a 1d
      - y value in a 2d
      val3   - weight in a 2d
      name   - name of a root spectrum (NOT CURRENTLY USED)
    */
    if (!Exists(dammId))
        return false;

	nonemptyList.insert(dammId);

    if (val2 == -1 && val3 == -1)
        count1cc_(dammId + offset, int(val1), 1);
    else if  (val3 == -1 || val3 == 0)
        count1cc_(dammId + offset, int(val1), int(val2));
    else 
        set2cc_(dammId + offset, int(val1), int(val2), int(val3));
    
    return true;
}

bool Plots::Plot(const std::string &mne, double val1, double val2, double val3, const char* name)
{    
    if (!Exists(mne))
        return false;
    return Plot(mneList.find(mne)->second, val1, val2, val3, name);
}

void Plots::PrintNonEmpty(std::ofstream& hislog) {
    set<int>::iterator it;
    for (it = nonemptyList.begin(); it != nonemptyList.end(); ++it) {
        int id = *it;
        hislog << "\t" << id + offset << " " << titleList[id] << endl;
    }
}
