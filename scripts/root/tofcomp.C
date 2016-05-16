{
    TFile *august = new TFile("052k-aug.root");
    TTree *augDat = (TTree*)august->Get("data");
    TFile *october = new TFile("052k-oct.root");
    TTree *octDat = (TTree*)october->Get("data");

    TH2D augQdcTof("augQdcTof","",130,-10,250,8000,0,32000);
    augDat->Draw("qdc:tof>>augQdcTof","tof>40 && qdc > 2000","colz");
    TH1D *projAug = augQdcTof->ProjectionX("projAug",0,8000);
    projAug->SetLineColor(kBlack);
    Double_t augNorm = projAug->GetEntries();
    projAug->Scale(1./augNorm);

    TH2D octQdcTof("octQdcTof","",130,-10,250,8000,0,32000);
    octDat->Draw("qdc:tof>>octQdcTof","tof>40 && qdc > 2000","colz");
    TH1D *projOct = octQdcTof->ProjectionX("projOct",0,8000);
    projOct->SetLineColor(kRed);
    Double_t octNorm = projOct->GetEntries();
    projOct->Scale(1./octNorm);


    projAug->SetXTitle("ToF (ns)");
    projAug->SetYTitle("Counts / (0.5 ns)");
    projAug->SetAxisRange(40,249,"X");
    projAug->SetTitleOffset(1.45,"Y");

    projAug->Draw();
    projOct->Draw("same");
}
