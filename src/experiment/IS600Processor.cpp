/** \file IS600Processor.cpp
 * \brief A class to process data from ISOLDE 599 and 600 experiments using
 * VANDLE.
 *
 *\author S. V. Paulauskas
 *\date July 14, 2015
 */
#include <fstream>
#include <iostream>

#include <cmath>

#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
#include "DoubleBetaProcessor.hpp"
#include "DetectorDriver.hpp"
#include "GeProcessor.hpp"
#include "GetArguments.hpp"
#include "Globals.hpp"
#include "IS600Processor.hpp"
#include "RawEvent.hpp"
#include "TimingMapBuilder.hpp"
#include "VandleProcessor.hpp"

#ifdef useroot
double IS600Processor::tof_ = 0.;
double IS600Processor::qdc_ = 0.;
double IS600Processor::ben_ = 0.;
double IS600Processor::snrl_ = 0.;
double IS600Processor::snrr_ = 0.;
double IS600Processor::pos_ = 0.;
double IS600Processor::tdiff_ = 0.;
unsigned int IS600Processor::vid_ = 9999;
unsigned int IS600Processor::evtnum_ = 0;
unsigned int IS600Processor::vsize_ = 0;
unsigned int IS600Processor::gsize_ = 0;
#endif

namespace dammIds {
    namespace experiment {
        const int DD_DEBUGGING0  = 0;
        const int DD_DEBUGGING1  = 1;
        const int DD_DEBUGGING2  = 2;
        const int DD_DEBUGGING3  = 3;
        const int DD_DEBUGGING4  = 4;
        const int DD_DEBUGGING5  = 5;
        const int DD_DEBUGGING6  = 6;
        const int DD_DEBUGGING7  = 7;
        const int DD_DEBUGGING8  = 8;
        const int DD_DEBUGGING9  = 9;
        const int DD_DEBUGGING10  = 10;
        const int DD_DEBUGGING11  = 11;
        const int DD_DEBUGGING12  = 12;
        const int DD_PROTONBETA2TDIFF_VS_BETA2EN = 13;
        const int D_ENERGY = 14;
        const int D_ENERGYBETA = 15;
        const int DD_PROTONGAMMATDIFF_VS_GAMMAEN = 16;
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void IS600Processor::DeclarePlots(void) {
    DeclareHistogram2D(DD_DEBUGGING0, SC, SD, "QDC CTof- No Tape Move");
    DeclareHistogram2D(DD_DEBUGGING1, SC, SD, "QDC ToF Ungated");
    DeclareHistogram2D(DD_DEBUGGING2, SC, SC, "Cor ToF vs. Gamma E");
    DeclareHistogram2D(DD_DEBUGGING4, SC, SC, "QDC vs Cor Tof Mult1");
    DeclareHistogram2D(DD_DEBUGGING5, SC, SC, "Mult2 Sym Plot Tof ");
    DeclareHistogram1D(DD_DEBUGGING6, SE, "LaBr3 RAW");
    DeclareHistogram2D(DD_PROTONBETA2TDIFF_VS_BETA2EN, SD, SA,
                       "BetaProton Tdiff vs. Beta Energy");

    const int energyBins1  = SD;
    DeclareHistogram1D(D_ENERGY, energyBins1,
		       "Gamma singles ungated");
    DeclareHistogram1D(D_ENERGYBETA, energyBins1,
                       "Gamma singles beta gated");
//    DeclareHistogram2D(DD_PROTONGAMMATDIFF_VS_GAMMAEN,
//		       SD, SB, "GammaProton TDIFF vs. Gamma Energy");
}

IS600Processor::IS600Processor() : EventProcessor(OFFSET, RANGE, "IS600PRocessor") {
    associatedTypes.insert("vandle");
    associatedTypes.insert("labr3");
    associatedTypes.insert("beta");
    associatedTypes.insert("ge");

    char hisFileName[32];
    GetArgument(1, hisFileName, 32);
    string temp = hisFileName;
    temp = temp.substr(0, temp.find_first_of(" "));
#ifdef useroot
    stringstream rootname;
    rootname << temp << ".root";
    rootfile_ = new TFile(rootname.str().c_str(),"UPDATE");
    roottree_ = new TTree("vandle","");
    roottree_->Branch("tof",&tof_,"tof/D");
    roottree_->Branch("qdc",&qdc_,"qdc/D");
    roottree_->Branch("ben",&ben_,"ben/D");
    roottree_->Branch("vid",&vid_,"vid/I");
    roottree_->Branch("evtnum",&evtnum_,"evtnum/I");
    roottree_->Branch("vsize",&vsize_,"vsize/I");
    roottree_->Branch("gsize",&gsize_,"gsize/I");
    roottree_->Branch("snrl", &snrl_,"snrl/D");
    roottree_->Branch("snrr", &snrr_,"snrr/D");
    roottree_->Branch("pos", &pos_,"pos/D");
    roottree_->Branch("tdiff", &tdiff_,"tdiff/D");
#endif
}

IS600Processor::~IS600Processor() {
#ifdef useroot
    rootfile_->Write();
    rootfile_->Close();
    delete(rootfile_);
#endif
}

///We do nothing here since we're completely dependent on the resutls of others
bool IS600Processor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return(false);
    return(true);
}

bool IS600Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    double plotMult_ = 2;
    double plotOffset_ = 1000;

    BarMap vbars, betas;
    map<unsigned int, pair<double,double> > lrtBetas;
    vector<ChanEvent*> geEvts;
    vector<vector<AddBackEvent> > geAddback;

    if(event.GetSummary("vandle")->GetList().size() != 0)
        vbars = ((VandleProcessor*)DetectorDriver::get()->
            GetProcessor("VandleProcessor"))->GetBars();
    if(event.GetSummary("beta:double")->GetList().size() != 0) {
        betas = ((DoubleBetaProcessor*)DetectorDriver::get()->
            GetProcessor("DoubleBetaProcessor"))->GetBars();
        lrtBetas = ((DoubleBetaProcessor*)DetectorDriver::get()->
            GetProcessor("DoubleBetaProcessor"))->GetLowResBars();
    }
    if(event.GetSummary("ge")->GetList().size() != 0) {
        geEvts = ((GeProcessor*)DetectorDriver::get()->
            GetProcessor("GeProcessor"))->GetGeEvents();
        geAddback = ((GeProcessor*)DetectorDriver::get()->
            GetProcessor("GeProcessor"))->GetAddbackEvents();
        cout << geAddback.size() << endl;
    }
    static const vector<ChanEvent*> &labr3Evts =
	event.GetSummary("labr3:mrbig")->GetList();

#ifdef useroot
    vsize_ = vbars.size();
    gsize_ = geEvts.size();
#endif

    //Obtain some useful logic statuses
    double lastProtonTime =
        TreeCorrelator::get()->place("logic_t1_0")->last().time;
    bool isTapeMoving = TreeCorrelator::get()->place("TapeMove")->status();

    int bananaNum = 2;
    bool hasMultOne = vbars.size() == 1;
    bool hasMultTwo = vbars.size() == 2;

    //Begin processing for VANDLE bars
    for (BarMap::iterator it = vbars.begin(); it !=  vbars.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        if(!bar.GetHasEvent() || bar.GetType() == "small")
            continue;

        unsigned int barLoc = barId.first;
        TimingCalibration cal = bar.GetCalibration();

        for(BarMap::iterator itStart = betas.begin();
	    itStart != betas.end(); itStart++) {
            unsigned int startLoc = (*itStart).first.first;
            BarDetector start = (*itStart).second;
            if(!start.GetHasEvent())
                continue;

            double tofOffset = cal.GetTofOffset(startLoc);
            double tof = bar.GetCorTimeAve() -
                start.GetCorTimeAve() + tofOffset;

            double corTof =
                ((VandleProcessor*)DetectorDriver::get()->
                GetProcessor("VandleProcessor"))->
                CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

            bool notPrompt = corTof > 45.;
            bool inPeel = histo.BananaTest(bananaNum,
					   corTof*plotMult_+plotOffset_,
					   bar.GetQdc());
            bool isLowStart = start.GetQdc() < 300;

#ifdef useroot
        qdc_ = bar.GetQdc();
        pos_ = bar.GetQdcPosition();
        tdiff_ = bar.GetTimeDifference();
        tof_ = tof;
        vid_ = barLoc;
        snrr_ = bar.GetRightSide().GetSignalToNoiseRatio();
        snrl_ = bar.GetLeftSide().GetSignalToNoiseRatio();
        ben_ = start.GetQdc();
        roottree_->Fill();
        qdc_ = tof_ = vid_ = ben_ = -9999;
#endif

            plot(DD_DEBUGGING1, tof*plotMult_+plotOffset_, bar.GetQdc());
            if(!isTapeMoving && !isLowStart)
                plot(DD_DEBUGGING0, corTof*plotMult_+plotOffset_,bar.GetQdc());
            if(hasMultOne)
                plot(DD_DEBUGGING4, corTof*plotMult_+plotOffset_, bar.GetQdc());

            if (geEvts.size() != 0 && notPrompt && hasMultOne) {
                for (vector<ChanEvent *>::const_iterator itGe = geEvts.begin();
                itGe != geEvts.end(); itGe++) {
                    plot(DD_DEBUGGING2, (*itGe)->GetCalEnergy(),
                        corTof*plotMult_+plotOffset_);
                }
                //plot(DD_TQDCAVEVSTOF_VETO+histTypeOffset, tof, bar.GetQdc());
                //plot(DD_TOFBARS_VETO+histTypeOffset, tof, barPlusStartLoc);
            }
        } // for(TimingMap::iterator itStart
    } //(BarMap::iterator itBar
    //End processing for VANDLE bars

    //-------------- LaBr3 Processing ---------------
    for(vector<ChanEvent*>::const_iterator it = labr3Evts.begin();
	it != labr3Evts.end(); it++)
	plot(DD_DEBUGGING6, (*it)->GetEnergy());


    //------------------ Double Beta Processing --------------
    for(map<unsigned int, pair<double,double> >::iterator it = lrtBetas.begin();
	it != lrtBetas.end(); it++)
        plot(DD_PROTONBETA2TDIFF_VS_BETA2EN, it->second.second,
            (it->second.first - lastProtonTime) /
            (10e-3/Globals::get()->clockInSeconds()) );


    //----------------- GE Processing -------------------
    bool hasBeta = TreeCorrelator::get()->place("Beta")->status();
    double clockInSeconds = Globals::get()->clockInSeconds();
    // plot with 10 ms bins
    const double plotResolution = 10e-3 / clockInSeconds;

    for (vector<ChanEvent*>::iterator it1 = geEvts.begin();
	 it1 != geEvts.end(); ++it1) {
        ChanEvent *chan = *it1;

        double gEnergy = chan->GetCalEnergy();
        double gTime   = chan->GetCorrectedTime();
        if (gEnergy < 10.) //hard coded fix later.
            continue;

        plot(D_ENERGY, gEnergy);
	if(hasBeta)
	    plot(D_ENERGYBETA, gEnergy);
	plot(DD_PROTONGAMMATDIFF_VS_GAMMAEN, gEnergy ,
	     (gTime - lastProtonTime) / plotResolution) ;
    }

#ifdef useroot
    evtnum_++;
#endif
    EndProcess();
    return(true);
}
