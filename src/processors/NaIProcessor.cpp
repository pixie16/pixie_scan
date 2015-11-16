/** \file NaIProcessor.cpp
 * NaI class takes the signals from the mcp detector
 *   and calculates a 2D position based on the readout
 * SNL 2-2-08, Modified DTM 9-09
 */
#include "DammPlotIds.hpp"
#include "NaIProcessor.hpp"
#include "RawEvent.hpp"

using std::string;
using std::vector;
using namespace dammIds::nai;

namespace dammIds {
  namespace nai {	
    const int DD_POSENE = 0;
    const int D_ENE     = 1;
  }
}
void NaIProcessor::NaIData::Clear(void)
{
}

NaIProcessor::NaIProcessor(void) : EventProcessor(OFFSET, RANGE, "nai")
{
  associatedTypes.insert("nai");
}

void NaIProcessor::DeclarePlots(void)
{
  
  const int posBins      = S4; //    16
  const int energyBins   = SE; // 16384
  DeclareHistogram2D(DD_POSENE, posBins, energyBins, "NaI Position and Energy");
  
}


bool NaIProcessor::Process(RawEvent &event)
{
  if (!EventProcessor::Process(event))
    return false;

  static const vector<ChanEvent*> &naiEvents = sumMap["nai"]->GetList();

  data.Clear();
  for (vector<ChanEvent*>::const_iterator it = naiEvents.begin();
       it != naiEvents.end(); it++) {
      ChanEvent *chan = *it;

      string subtype   = chan->GetChanID().GetSubtype();
      int number 	= chan->GetChanID().GetLocation();
      double calEnergy = chan->GetCalEnergy();
      double naiTime   = chan->GetTime();
      using std::cout;
      using std:: endl; 
      if(number<13){
	plot(DD_POSENE,number,calEnergy);
      }
  }
      
      
      EndProcess();
      return true;
}
