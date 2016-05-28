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

double cloverroot[4];

struct TapeInfo{
    unsigned int move;
    unsigned int beam;
};

static VandleRoot vandleroot;
static HighResTimingData::HrtRoot leftside;
static HighResTimingData::HrtRoot rightside;
static TapeInfo tapeinfo;
static unsigned int vsize_ = 0;
static unsigned int evtnum_ = 0;
static unsigned int numFills = 0;
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
//    DeclareHistogram1D(DD_DEBUGGING6, SE, "LaBr3 Cal");
//    DeclareHistogram2D(DD_PROTONBETA2TDIFF_VS_BETA2EN, SD, SA,
//                       "BetaProton Tdiff vs. Beta Energy");
//    DeclareHistogram1D(D_ENERGY, SD, "Gamma singles ungated");
//    DeclareHistogram1D(D_ENERGYBETA, SD, "Gamma singles beta gated");
//    DeclareHistogram2D(DD_PROTONGAMMATDIFF_VS_GAMMAEN,
//		       SD, SB, "GammaProton TDIFF vs. Gamma Energy");
}

IS600Processor::IS600Processor() : EventProcessor(OFFSET, RANGE, "IS600Processor") {
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
    roottree_->Branch("clover", &cloverroot, "en[4]/D");
    roottree_->Branch("tape", &tapeinfo,"move/b:beam");
    roottree_->Branch("evtnum",&evtnum_,"evtnum/I");
    roottree_->Branch("vsize",&vsize_,"vsize/I");

    rootname.str("");
    rootname << temp << "-walk.root";
    walkfile_ = new TFile(rootname.str().c_str(),"RECREATE");
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
    delete(walkfile_);
#endif
}

bool IS600Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    bool useAddback = true;
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
    if(useAddback) {
        if(geAddback.size() != 0) {
            if(geAddback.at(0).size() != 0)
                cloverroot[0] = geAddback.at(0).at(0).energy;
            if(geAddback.at(1).size() != 0)
                cloverroot[1] = geAddback.at(1).at(0).energy;
            if(geAddback.at(2).size() != 0)
                cloverroot[2] = geAddback.at(2).at(0).energy;
            if(geAddback.at(3).size() != 0)
                cloverroot[3] = geAddback.at(3).at(0).energy;
        }
    } else {
        if (geEvts.size() != 0) {
            for (vector<ChanEvent *>::const_iterator itGe = geEvts.begin();
                itGe != geEvts.end(); itGe++) {
                switch((*itGe)->GetChanID().GetLocation()) {
                    case(0):
                        cloverroot[0] = (*itGe)->GetCalEnergy();
                        break;
                    case(1):
                        cloverroot[1] = (*itGe)->GetCalEnergy();
                        break;
                    case(2):
                        cloverroot[2] = (*itGe)->GetCalEnergy();
                        break;
                    case(3):
                        cloverroot[3] = (*itGe)->GetCalEnergy();
                        break;
                    default:
                        break;
                }
            }
        }
    }

    if(TreeCorrelator::get()->place("TapeMove")->status())
        tapeinfo.move = 1;
    else
        tapeinfo.move = 0;
    if(TreeCorrelator::get()->place("Beam")->status())
        tapeinfo.beam = 1;
    else
        tapeinfo.beam = 0;
#endif

    //Obtain some useful logic statuses
    double lastProtonTime =
        TreeCorrelator::get()->place("logic_t1_0")->last().time;

    //Begin processing for VANDLE bars
    for (BarMap::iterator it = vbars.begin(); it !=  vbars.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        if(bar.GetType() != "small") {
            bar.GetRightSide().FillRootStructure(rightside);
            bar.GetLeftSide().FillRootStructure(leftside);
            walktree_->Fill();
            bar.GetRightSide().ZeroRootStructure(rightside);
            bar.GetLeftSide().ZeroRootStructure(leftside);
        }

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

            double corTof = ((VandleProcessor*)DetectorDriver::get()->
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
            //roottree_->Fill();
            numFills++;
            vandleroot.qdc = vandleroot.pos = vandleroot.tdiff =
                vandleroot.tof = vandleroot.vid = vandleroot.snrr =
                vandleroot.snrl = vandleroot.ben = 0;
            #endif

            plot(DD_DEBUGGING1, tof*plotMult_+plotOffset_, bar.GetQdc());
            if (geAddback.size() != 0) {
                for (unsigned int idx = 0; idx < geAddback.size(); idx++) {
                    if(geAddback.at(idx).size() != 0)
                        plot(DD_DEBUGGING2, geAddback.at(idx).at(0).energy,
                            corTof*plotMult_+plotOffset_);
                }
            }
        } // for(TimingMap::iterator itStart
    } //(BarMap::iterator itBar
    //End processing for VANDLE bars

#ifdef useroot
    evtnum_++;
    cloverroot[0] = cloverroot[1] =
        cloverroot[2] = cloverroot[3] = 0;
#endif
    EndProcess();
    return(true);
}
