/** \file IS600Processor.cpp
 * \brief A class to process data from ISOLDE 599 and 600 experiments using
 * VANDLE.
 *
 *\author S. V. Paulauskas
 *\date July 14, 2015
 */
#include <fstream>
#include <iostream>
#include <string>

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
TFile *rootfile_, *walkfile_;
TTree *roottree_, *walktree_;

struct VandleRoot{
    double tof;
    double qdc;
    double ben;
    double snrl;
    double snrr;
    double pos;
    double tdiff;
    unsigned int vid;
};

struct CloverRoot{
    double gen0;
    double gen1;
};

struct TapeInfo{
    unsigned int move;
    unsigned int beam;
};

static VandleRoot vandleroot;
static CloverRoot cloverroot;
static HighResTimingData::HrtRoot leftside;
static HighResTimingData::HrtRoot rightside;
static TapeInfo tapeinfo;
static unsigned int vsize_ = 0;
static unsigned int evtnum_ = 0;
#endif //#ifdef useroot

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
    rootfile_ = new TFile(rootname.str().c_str(),"RECREATE");
    roottree_ = new TTree("data","");
    roottree_->Branch("vandle", &vandleroot, "tof/D:qdc:ben:snrl:snrr:pos:tdiff:vid/I");
    roottree_->Branch("clover", &cloverroot, "en0/D:en1");
    roottree_->Branch("tape", &tapeinfo,"move/b:beam");
    roottree_->Branch("eCleanup and vtnum",&evtnum_,"evtnum/I");
    roottree_->Branch("vsize",&vsize_,"vsize/I");

    walkfile_ = new TFile("walk.root","RECREATE");
    walktree_ = new TTree("walk","");
    walktree_->Branch("left",&leftside,"qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
    walktree_->Branch("right",&rightside,"qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
#endif
}

IS600Processor::~IS600Processor() {
#ifdef useroot
    rootfile_->Write();
    rootfile_->Close();
    walkfile_->Write();
    walkfile_->Close();
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
    }
    static const vector<ChanEvent*> &labr3Evts =
	event.GetSummary("labr3:mrbig")->GetList();

#ifdef useroot
    vsize_ = vbars.size();
    if(geAddback.size() != 0) {
        if(geAddback.at(0).size() != 0)
            cloverroot.gen0 = geAddback.at(0).at(0).energy;
        else
            cloverroot.gen0 = 0;
        if(geAddback.at(1).size() != 0)
            cloverroot.gen1 = geAddback.at(1).at(0).energy;
        else
            cloverroot.gen1 = 0;
    } else
        cloverroot.gen0 = cloverroot.gen1 = 0;

    if(TreeCorrelator::get()->place("TapeMove")->status())
        tapeinfo.move = 1;
    else
        tapeinfo.move = 0;
    if(TreeCorrelator::get()->place("Beam")->status())
        tapeinfo.beam = 1;
    else
        tapeinfo.beam = 0;
#endif

//    for(BarMap::iterator itStart = betas.begin();
//	    itStart != betas.end(); itStart++) {
//        (*itStart).second.GetRightSide().FillRootStructure(rightside);
//        (*itStart).second.GetLeftSide().FillRootStructure(leftside);
//        walktree_->Fill();
//    }
//
    //Obtain some useful logic statuses
    double lastProtonTime =
        TreeCorrelator::get()->place("logic_t1_0")->last().time;

    //Begin processing for VANDLE bars
    for (BarMap::iterator it = vbars.begin(); it !=  vbars.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        bar.GetRightSide().FillRootStructure(rightside);
        bar.GetLeftSide().FillRootStructure(leftside);
        walktree_->Fill();

        rightside.qdc = leftside.qdc = rightside.time = leftside.time =
            rightside.id = leftside.id = 0;

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

            #ifdef useroot
            vandleroot.qdc   = bar.GetQdc();
            vandleroot.pos   = bar.GetQdcPosition();
            vandleroot.tdiff = bar.GetTimeDifference();
            vandleroot.tof   = corTof;
            vandleroot.vid   = barLoc;
            vandleroot.snrr  = bar.GetRightSide().GetSignalToNoiseRatio();
            vandleroot.snrl  = bar.GetLeftSide().GetSignalToNoiseRatio();
            vandleroot.ben   = start.GetQdc();
            roottree_->Fill();
            #endif

            plot(DD_DEBUGGING1, tof*plotMult_+plotOffset_, bar.GetQdc());

            if (geEvts.size() != 0) {
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
