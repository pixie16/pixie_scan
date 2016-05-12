#define vandle_cxx
#include "vandle.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void vandle::Loop() {
//   In a ROOT session, you can do:
//      Root > .L vandle.C
//      Root > vandle t
//      Root > t.GetEntry(12); // Fill t data members with entry number 12
//      Root > t.Show();       // Show values of entry 12
//      Root > t.Show(16);     // Read and show values of entry 16
//      Root > t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
    if (fChain == 0)
        return;

    Long64_t nentries = fChain->GetEntriesFast();

    Long64_t nbytes = 0, nb = 0;
    unsigned int prevEvtNum = 0, prevVid = -9999;
    double prevTof = -9999, prevQdc = -9999;
    for (Long64_t jentry = 0; jentry < nentries; jentry++) {
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0)
            break;
        nb = fChain->GetEntry(jentry);
        nbytes += nb;

        if (vsize != 2)
            continue;
        if(prevEvtNum == evtnum && abs(vid-prevVid) > 2) {
            toftof->Fill(tof,prevTof);
            toftofqdc->Fill(tof,prevTof,qdc);
        }
        prevEvtNum = evtnum;
        prevTof = tof;
        prevVid = vid;
        prevQdc = qdc;
    }
}
