//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed May 11 14:11:14 2016 by ROOT version 5.34/32
// from TTree vandle/
// found on file: 052k-aug.root
//////////////////////////////////////////////////////////

#ifndef vandle_h
#define vandle_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class vandle {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Double_t        tof;
   Double_t        qdc;
   Double_t        ben;
   Int_t           vid;
   Int_t           evtnum;
   Int_t           vsize;
   Int_t           gsize;

   // List of branches
   TBranch        *b_tof;   //!
   TBranch        *b_qdc;   //!
   TBranch        *b_ben;   //!
   TBranch        *b_vid;   //!
   TBranch        *b_evtnum;   //!
   TBranch        *b_vsize;   //!
   TBranch        *b_gsize;   //!

   vandle(TTree *tree=0);
   virtual ~vandle();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef vandle_cxx
vandle::vandle(TTree *tree) : fChain(0) {
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("052k-aug.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("052k-aug.root");
      }
      f->GetObject("vandle",tree);

   }
   Init(tree);
}

vandle::~vandle()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t vandle::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t vandle::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void vandle::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("tof", &tof, &b_tof);
   fChain->SetBranchAddress("qdc", &qdc, &b_qdc);
   fChain->SetBranchAddress("ben", &ben, &b_ben);
   fChain->SetBranchAddress("vid", &vid, &b_vid);
   fChain->SetBranchAddress("evtnum", &evtnum, &b_evtnum);
   fChain->SetBranchAddress("vsize", &vsize, &b_vsize);
   fChain->SetBranchAddress("gsize", &gsize, &b_gsize);
   Notify();
}

Bool_t vandle::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void vandle::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t vandle::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef vandle_cxx
