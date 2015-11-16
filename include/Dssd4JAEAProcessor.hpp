/** \file Dssd4SHEProcessor.hpp
 *
 */

#ifndef __DSSD4JAEA_PROCESSOR_HPP_
#define __DSSD4JAEA_PROCESSOR_HPP_

#include <vector>
#include <utility>
#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "JAEACorrelator.hpp"
#include "Trace.hpp" 

namespace dammIds { 
  namespace dssd4jaea {
    // Diagnostic plot in PreProcessor()
    const int DD_DSSDFRONT_POSENERGY = 0;
    const int DD_DSSDBACK_POSENERGY  = 1;
    
    // Diagnostic plot in Processor()
    const int DD_EVENT_POSITION_WITHOUT_MWPC=10;
  
    const int DD_DSSDFRONT_POSENERGY_WITHOUT_MWPC=11;
    const int DD_DSSDBACK_POSENERGY_WITHOUT_MWPC=12;

    // Trace Plot in Processor()
    
    const int DD_IMPLANT_POSITION = 25; // 1825 : 725
    const int DD_DECAY_POSITION   = 26;
    const int DD_IMPLANT_FRONT_ENERGY__POSITION = 41; // 1841 : 741
    const int DD_IMPLANT_BACK_ENERGY__POSITION  = 42; // 1842 : 742
    const int DD_DECAY_FRONT_ENERGY__POSITION   = 43;
    const int DD_DECAY_BACK_ENERGY__POSITION  = 44;
    // new
    const int DD_DECAY_FRONT_ENERGY__POSITION_NOVETO   = 45;
    const int DD_DECAY_BACK_ENERGY__POSITION_NOVETO  = 46;
    const int DD_DECAY_FRONT_ENERGY__POSITION_VETO   = 47;
    const int DD_DECAY_BACK_ENERGY__POSITION_VETO  = 48;
    // end of new

    const int DD_ENERGY_DECAY_TIME_GRANX = 50;
    const int D_DECAY_FRONT_ENERGY = 59; // 1859 : 759
    const int D_DECAY_BACK_ENERGY = 60;  // 1860 : 760
    const int DD_IMPLANT_POSITION__MCP = 61; // 1861 : 761


    /** Histograms with VETO system for JAEA experiment **/
    const int D_DECAY_FRONT_ENERGY_NOVETO = 63; // with no VETO
    const int D_DECAY_BACK_ENERGY_NOVETO  = 64; // with no VETO
    const int D_DECAY_FRONT_ENERGY_VETO = 65; // with VETO
    const int D_DECAY_BACK_ENERGY_VETO  = 66; // with VETO
    
    const int D_DECAY_FRONT_ENERGY_NAI = 67; // with NaI
    const int D_DECAY_BACK_ENERGY_NAI  = 68; // with NaI
    const int D_DECAY_FRONT_ENERGY_PIN = 69; // with Pin
    const int D_DECAY_BACK_ENERGY_PIN  = 70; // with Pin
  
    
    /** Plotting Pixel Correlated events **/
    const int DD_CHAIN_NUM_FISSION = 70;
    const int DD_CHAIN_NUM_ALPHA = 71;
    const int DD_CHAIN_ALPHA_V_ALPHA = 72;
    
    
    const int DD_ENERGY2F_DT=73;
    const int DD_ENERGY2F_ENERGY1F=74;
    const int DD_ENERGY2B_ENERGY1B=75;
    const int DD_DOUBLETRACE_FRONT_WITHOUT_MWPC=76;
    const int DD_DOUBLETRACE_BACK_WITHOUT_MWPC=77;
    
    const int DD_ENERGY_DECAY_TIME_GRANX_SECOND = 80;

    // correlation between first and second decay energy
    const int DD_ENERGY_DECAY12 = 89;

    const int DD_TEST=99;
    
  }
}
class Dssd4JAEAProcessor : public EventProcessor {
public:
    Dssd4JAEAProcessor(double frontBackTimeWindow, 
                      double deltaEnergy,
		      double recoilEnergyCut, 
                      double highEnergyCut, 
                      double lowEnergyCut, 
                      double fisisonEnergyCut, 
                      int numFrontStrips, 
                      int numBackStrips);
    virtual void DeclarePlots();
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
public:
  int numDoubleTraces;
protected:
    bool pickEventType(JAEAEvent& event);

    struct StripEvent {
        StripEvent() {
	  Trace emptyTrace;
	  t = 0;
	  E = 0;
	  pos = -1;
	  sat = false;
	  pileup = false;
	  tr = emptyTrace;
	}

      StripEvent(double energy, double time, int position,
		 bool saturated, Trace trace) {
	E = energy;
	t = time;
	pos = position;
	sat = saturated;
	pileup = false;
	tr = trace;
      }
      
      double t;
      double E;
      int pos;
      bool sat;
      bool pileup;
      Trace tr;
    };

    JAEACorrelator correlator_;
    /** Events matched based on energy (MaxEvent) **/
    std::vector<std::pair<StripEvent, StripEvent> > xyEventsEMatch_; 

    /** Events matched based on timing correlation  **/
    std::vector<std::pair<StripEvent, StripEvent> > xyEventsTMatch_; 

    /**Limit in seconds for the time difference between front and
     * back to be correlated. Also to find Si Side detectors correlated
     * events (escapes)*/
    double timeWindow_;

    /**Limit in keV of difference between front and back events to
     * be considered a good event */
    double deltaEnergy_;

    /**Limit in keV of difference between front and back events to
     * be considered a good event */
    double recoilEnergyCut_;

    /** Energy cut to differentiate high-energy events (fission?)
     * from implantation and alpha decays (in keV) **/
    double highEnergyCut_;

    /** Low Energy cut for interesting alphas (in keV) **/
    double lowEnergyCut_;

    /** Fission Energy cut (in keV) **/
    double fissionEnergyCut_;

};

#endif 
