{
    TH2D *toftof = new TH2D("toftof","",260,-10,250,260,-10,250);
    TH3D *toftofqdc = new TH3D("toftofqdc","",130,-10,250,130,-10,250,2000,0,8000);
    gROOT->ProcessLine(".L ./scripts/root/vandle.C");
    vandle t;
    t.Loop();
    toftof->Draw();
}
