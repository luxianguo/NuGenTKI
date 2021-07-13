#include "style.h"

TString gOutdir;
TString gConfig;

#define __KSLOW__ 0

bool IsPiPlusBeam()
{
  return gOutdir.Contains("Piplus");
}

void CorrectFSString(TString & slen)
{
  if(!IsPiPlusBeam()){
    if(slen.Contains("#pi^{+}")){
      slen.ReplaceAll("#pi^{+}","e");
    }
    else if(slen.Contains("#pi^{0}")){
      slen.Prepend("e ");
    }
  }
}

TH1D *savedraw(const int iplot, TTree *t, TCanvas *c1, const TString var, const TString cut="1", const TString opt="hist", const int nbin=-999, const double xmin = -999, const double xmax = -999)
{
  gStyle->SetOptStat("enoum");
  if(opt.Contains("same")||iplot>50){
    gPad->SetLogy(0);
  }
  else{
    gPad->SetLogy(1);
  }

  const TString hn=Form("%sid%d", c1->GetName(), iplot);
  const TString hbin = nbin>0?Form("(%d, %f, %f)", nbin, xmin, xmax):"";
  const TString v2 = var+">>"+hn+hbin;
  const int nd = t->Draw(v2, cut, opt);
  printf("Draw(%s, %s, %s) ndraw %d\n", v2.Data(), cut.Data(), opt.Data(), nd);

  c1->Print(Form(gOutdir+"%s.png", hn.Data()));

  TH1D * hout = (TH1D*) gDirectory->Get(hn);
  if(!hout){
    gDirectory->ls();
    printf("can't find hist %s\n", hn.Data());
    exit(1);
  }
  return hout;
}

void sub2DM(TTree *t, TCanvas *c1, TList *lout, const TString basecut, const TString xvar, const double xmin, const double xmax, const TString yvar)
{
  const TString h2n = Form("%s_%s_2DM_basecut", c1->GetName(), xvar.Data());
  TH2D * hm=new TH2D(h2n,"", 40, xmin, xmax, 23, -1.5, 21.5); lout->Add(hm);
  style::ResetStyle(hm);

  if(yvar=="(recoilM-event_recoilM)"){
    hm->SetYTitle("#it{M}_{X}-#it{M}_{A'} (GeV/#it{c}^{2})");
  }
  else{
    printf("wrong yvar! %s\n", yvar.Data()); exit(1);
  }

  if(xvar=="event_recoilM"){
    hm->SetXTitle("#it{M}_{A'} (GeV/#it{c}^{2})");
  }
  else if(xvar=="recoilM"){
    hm->SetXTitle("#it{M}_{X} (GeV/#it{c}^{2})");
  }
  else{
    printf("wrong xvar! %s\n", xvar.Data()); exit(1);
  }

  t->Draw(Form("%s:%s>>%s", yvar.Data(), xvar.Data(), h2n.Data()), basecut, "colz");

  const TString tag=c1->GetName();
  TString slen;
  if(tag.Contains("PiZero_OneP")){
    slen="#pi^{0} p";
  }
  else if(tag.Contains("Pion_OneP")){
    slen="#pi^{+} p";
  }
  else if(tag.Contains("PiZero_TwoP")){
    slen="#pi^{0} p p";
  }
  else if(tag.Contains("Pion_TwoP")){
    slen="#pi^{0} p p";
  }
  else{
    printf("wrong tag! %s\n", c1->GetName()); exit(1);
  }

  CorrectFSString(slen);

  TLatex * lh = new TLatex(0.2, 0.83, gConfig);
  style::ResetStyle(lh);
  
  TLatex * lt = new TLatex(0.65, 0.83, slen);
  style::ResetStyle(lt);

  lh->Draw();
  lt->Draw();
  
  c1->Print(gOutdir+h2n+".png");

  const double underf = hm->Integral(0,10000,0,1);//only for y 21, -1, 20 !!
  if(underf>0){
    printf("\n\n********************************************* Underflow!! %s %f *************************************\n\n", c1->GetName(), underf); exit(1);
  }
}

void Draw2DM(TTree *t, TCanvas *c1, TList *lout, const TString basecut)
{
  style::PadSetup(c1);
  c1->SetBottomMargin(0.18);
  c1->SetLeftMargin(0.13);
  c1->SetRightMargin(0.12);
  c1->SetTopMargin(0.07);
  
  gStyle->SetOptStat(0);

  sub2DM(t, c1, lout, basecut, "event_recoilM", 17.5, 37.5, "(recoilM-event_recoilM)");
  sub2DM(t, c1, lout, basecut, "recoilM",       34.8,   37.4,   "(recoilM-event_recoilM)");
}

void PhysicsBlock(TTree *t, TCanvas *c1, TList *lout, const TString basecut, const TString sigdef, const TString selcut, int nbin = -999, double xmin = -999, double xmax = -999)
{
  const TString opt="hist"; ;
    
  TH1D * hout = 0x0;

  hout = savedraw(2000, t, c1, "recoilM", basecut,               opt, nbin, xmin, xmax); lout->Add(hout);
  //hout = savedraw(2001, t, c1, "recoilM", basecut+" && "+sigdef, opt, nbin, xmin, xmax); lout->Add(hout);
  hout = savedraw(2001, t, c1, "recoilM", sigdef,                opt, nbin, xmin, xmax); lout->Add(hout);
  hout = savedraw(2002, t, c1, "recoilM", basecut+" && "+selcut, opt, nbin, xmin, xmax); lout->Add(hout);

  nbin = 50; xmin = 0; xmax = 2;
  hout = savedraw(100, t, c1, "recoilP", basecut+" && "+selcut, opt, nbin, xmin, xmax); lout->Add(hout);
  savedraw(101, t, c1, "event_recoilP", basecut+" && "+selcut,"e same", nbin, xmin, xmax);

  Draw2DM(t, c1, lout, basecut);
}


void subdrawPiZero_OneP(TTree * t, TList *lout)
{
  TCanvas * c1 = new TCanvas("PiZero_OneP","",600,400);

  const TString beamCut = IsPiPlusBeam() ? "nPion==0 && nPiZero==1 && nElectron==0" : "nPion==0 && nPiZero==1 && nElectron==1";//e pi0 p final state
  const TString basecut = beamCut+" && nBkg==0 && nProton==1";
  const TString sigdef  = IsPiPlusBeam() ? "(totparcount-1000000*nGamma)==100001" : "(totparcount-1000000*nGamma)==110001";
  const TString selcut  = IsPiPlusBeam() ? "recoilM<36.302": "recoilM<36.307";

#if __KSLOW__
  int iplot=9000;
  savedraw(iplot++, t, c1, "nProton==1");
  savedraw(iplot++, t, c1, "nPiZero==1", "nProton==1");
  savedraw(iplot++, t, c1, "nPiZero==1 && nProton==1");
  savedraw(iplot++, t, c1, "nPion==0", "nPiZero==1 && nProton==1");
  savedraw(iplot++, t, c1, "nPion==0 && nPiZero==1 && nProton==1");
  savedraw(iplot++, t, c1, "nBkg==0", "nPion==0 && nPiZero==1 && nProton==1");
  savedraw(iplot++, t, c1, basecut);//selection rate 3.6%
  savedraw(iplot++, t, c1, sigdef, basecut);//purity 28%

  //https://www.wolframalpha.com/input/?i=argon-39+mass+in+gev //36.295028 GeV
  savedraw(iplot++, t, c1, "event_recoilM", basecut+" && "+sigdef);
  savedraw(iplot++, t, c1, "event_recoilM", basecut+" && abs(event_recoilM-36.295)<0.002");
  savedraw(iplot++, t, c1, sigdef, basecut+" && abs(event_recoilM-36.295)<0.002");//purity 99.99%

  //36.295 - 36.304
  //NOTE: need to draw smeared kinematic reconstructed recoilM, resolution should be much smaller than missing nucleon mass from neutron!
  savedraw(iplot++, t, c1, sigdef, basecut+" && "+selcut);//purity 99.52
#endif

  PhysicsBlock(t, c1, lout, basecut, sigdef, selcut, 50, 36.293, 36.318);
}


void subdrawPion_OneP(TTree * t, TList *lout)
{
  TCanvas * c1 = new TCanvas("Pion_OneP","",600,400);

  const TString beamCut = IsPiPlusBeam() ? "nPion==1 && nPiZero==0 && nElectron==0" : "nPion==0 && nPiZero==0 && nElectron==1";//e p final state
  const TString basecut = beamCut+" && nBkg==0 && nProton==1";
  const TString sigdef  = IsPiPlusBeam() ? "(totparcount-1000000*nGamma)==11" : "(totparcount-1000000*nGamma)==10001";
  const TString selcut  = "recoilM<36.307";
  
#if __KSLOW__
  int iplot=9000;
  savedraw(iplot++, t, c1, "nProton==1");
  savedraw(iplot++, t, c1, "nPion==1", "nProton==1");
  savedraw(iplot++, t, c1, "nPion==1 && nProton==1");
  savedraw(iplot++, t, c1, "nPiZero==0", "nPion==1 && nProton==1");
  savedraw(iplot++, t, c1, "nPiZero==0 && nPion==1 && nProton==1");
  savedraw(iplot++, t, c1, "nBkg==0", "nPiZero==0 && nPion==1 && nProton==1");
  savedraw(iplot++, t, c1, basecut);//selection rate 8.3%
  savedraw(iplot++, t, c1, sigdef, basecut);//purity 31%

  //https://www.wolframalpha.com/input/?i=chlorine-39+mass+in+gev //36.2984698
  savedraw(iplot++, t, c1, "event_recoilM", sigdef +" && "+basecut);
  savedraw(iplot++, t, c1, "event_recoilM",basecut+" && abs(event_recoilM-36.298)<0.04");
  savedraw(iplot++, t, c1, sigdef,basecut+" && abs(event_recoilM-36.298)<0.04");//purity 99.98%
  
  //36.295 - 36.309
  savedraw(iplot++, t, c1, sigdef,basecut+" && "+selcut);//purity 99.98
#endif

  PhysicsBlock(t, c1, lout, basecut, sigdef, selcut, 50, 36.293, 36.318);
}


void subdrawPiZero_TwoP(TTree * t, TList *lout)
{
  //NOTE:something special with pi0 2p!!
  
  TCanvas * c1 = new TCanvas("PiZero_TwoP","",600,400);

  const TString beamCut = IsPiPlusBeam() ? "nPion==0 && nPiZero==1 && nElectron==0" : "nPion==0 && nPiZero==1 && nElectron==1";//e pi0 p p final state
  const TString basecut = beamCut+" && nBkg==0 && nProton==2";
  const TString sigdef  = IsPiPlusBeam() ? "(totparcount-1000000*nGamma)==100002" : "(totparcount-1000000*nGamma)==110002";
  const TString selcut  = IsPiPlusBeam() ? "recoilM<35.374" : "recoilM<35.379";
  
#if __KSLOW__
  int iplot=9000;
  savedraw(iplot++, t, c1, "nProton==2");
  savedraw(iplot++, t, c1, "nPiZero==1", "nProton==2");
  savedraw(iplot++, t, c1, "nPiZero==1 && nProton==2");
  savedraw(iplot++, t, c1, "nPion==0", "nPiZero==1 && nProton==2");
  savedraw(iplot++, t, c1, "nPion==0 && nPiZero==1 && nProton==2");
  savedraw(iplot++, t, c1, "nBkg==0", "nPion==0 && nPiZero==1 && nProton==2");
  savedraw(iplot++, t, c1, basecut);//selection rate 1.9%
  savedraw(iplot++, t, c1, sigdef, basecut);//purity 0.94%

  //https://www.wolframalpha.com/input/?i=chlorine-38+mass+in+gev //35.3669778
  savedraw(iplot++, t, c1, "event_recoilM",basecut+" && "+selcut);
  savedraw(iplot++, t, c1, "event_recoilM",basecut+" && abs(event_recoilM-35.367)<0.003");
  savedraw(iplot++, t, c1, sigdef,basecut+" && abs(event_recoilM-35.367)<0.003");//purity 1

  //savedraw(iplot++, t, c1, sigdef,basecut+" && abs(recoilM-35.367)<0.009");//55.7%
  savedraw(iplot++, t, c1, sigdef,basecut+" && "+selcut);//99.43%
#endif

  PhysicsBlock(t, c1, lout, basecut, sigdef, selcut, 50, 35.365, 35.415);
}


void subdrawPion_TwoP(TTree * t, TList *lout)
{
  TCanvas * c1 = new TCanvas("Pion_TwoP","",600,400);

  const TString beamCut = IsPiPlusBeam() ? "nPion==1 && nPiZero==0 && nElectron==0" : "nPion==0 && nPiZero==0 && nElectron==1";//e p p final state
  const TString basecut = beamCut+" && nBkg==0 && nProton==2";
  const TString sigdef  = IsPiPlusBeam() ? "(totparcount-1000000*nGamma)==12" : "(totparcount-1000000*nGamma)==10002";
  const TString selcut  = "recoilM<35.379";

#if __KSLOW__
  int iplot=9000;
  savedraw(iplot++, t, c1, "nProton==2");
  savedraw(iplot++, t, c1, "nPion==1", "nProton==2");
  savedraw(iplot++, t, c1, "nPion==1 && nProton==2");
  savedraw(iplot++, t, c1, "nPiZero==0", "nPion==1 && nProton==2");
  savedraw(iplot++, t, c1, "nPiZero==0 && nPion==1 && nProton==2");
  savedraw(iplot++, t, c1, "nBkg==0", "nPiZero==0 && nPion==1 && nProton==2");
  savedraw(iplot++, t, c1, basecut);//selection rate 2.6%
  savedraw(iplot++, t, c1, sigdef, basecut);//purity 2.75%

  //https://www.wolframalpha.com/input/?i=sulphur-38+mass+in+gev //35.3699147
  savedraw(iplot++, t, c1, "event_recoilM",basecut+" && "+selcut);
  savedraw(iplot++, t, c1, "event_recoilM",basecut+" && abs(event_recoilM-35.370)<0.006");
  savedraw(iplot++, t, c1, sigdef,basecut+" && abs(event_recoilM-35.370)<0.006");//purity 1

  savedraw(iplot++, t, c1, sigdef,basecut+" && "+selcut);//purity 99.59%
#endif

  PhysicsBlock(t, c1, lout, basecut, sigdef, selcut, 50, 35.365, 35.415);
}

void summary_RecoilP(TList *lout)
{
  TH1D * PiZero_OneP = (TH1D*) lout->FindObject("PiZero_OnePid100"); if(!PiZero_OneP){printf("PiZero_OneP not found!\n"); lout->ls(); exit(1);}
  TH1D * Pion_OneP = (TH1D*) lout->FindObject("Pion_OnePid100"); if(!Pion_OneP){printf("Pion_OneP not found!\n"); lout->ls(); exit(1);}
  TH1D * PiZero_TwoP = (TH1D*) lout->FindObject("PiZero_TwoPid100"); if(!PiZero_TwoP){printf("PiZero_TwoP not found!\n"); lout->ls(); exit(1);}
  TH1D * Pion_TwoP = (TH1D*) lout->FindObject("Pion_TwoPid100"); if(!Pion_TwoP){printf("Pion_TwoP not found!\n"); lout->ls(); exit(1);}

  TH1D * hhs[]={PiZero_OneP, Pion_OneP, PiZero_TwoP, Pion_TwoP};
  //const int stkcols[]={1014, 1003, 1002, 1009, 1010, 1007, 1012, 1005, 1011, 1008, 1014, 1017};
  const int lincols[]={1002, 1009, 1008, 1011, 1014, 1017, 1015};
  const int nh = sizeof(hhs)/sizeof(TH1D*);
  for(int ii=0; ii<nh; ii++){
    style::ResetStyle(hhs[ii]);
    hhs[ii]->Scale(1./hhs[ii]->Integral(0,100000), "width");
    hhs[ii]->SetLineWidth(2);
    hhs[ii]->SetLineColor(style::GetColor(lincols[ii]));  
    hhs[ii]->GetXaxis()->SetRangeUser(0,0.8);
    hhs[ii]->SetYTitle("p.d.f.");
    hhs[ii]->SetXTitle("#it{p}_{X} (GeV/#it{c})");
  }

  TCanvas * c0=new TCanvas("cP","",600,400);
  style::PadSetup(c0);
  c0->SetBottomMargin(0.18);
  c0->SetRightMargin(0.03);
  c0->SetTopMargin(0.03);
  
  gStyle->SetOptStat(0);

  TLegend * lg = new TLegend(0.6, 0.6, 0.9, 0.9);
  style::ResetStyle(lg,0.3);
  lg->SetHeader(gConfig);
  
  TString opt="hist C ";

  TString beam;
  TString slen;
  slen = "#pi^{0}"; CorrectFSString(slen); PiZero_OneP->Draw(opt); lg->AddEntry(PiZero_OneP,slen+" p","l");
  slen = "#pi^{+}"; CorrectFSString(slen); Pion_OneP->Draw(opt+" same"); lg->AddEntry(Pion_OneP,slen+" p","l");
  //c0->Print(gOutdir+"summary_RecoilP_OnePi.png"); 
  slen = "#pi^{0}"; CorrectFSString(slen); PiZero_TwoP->Draw(opt+" same"); lg->AddEntry(PiZero_TwoP,slen+" p p","l");
  slen = "#pi^{+}"; CorrectFSString(slen); Pion_TwoP->Draw(opt+" same"); lg->AddEntry(Pion_TwoP,slen+" p p","l");
  lg->Draw();
  
  c0->Print(gOutdir+"summary_RecoilP_TwoPi.png");
}


void summary_RecoilM(TList *lout)
{
  TH1D * PiZero_OneP_Raw = (TH1D*) lout->FindObject("PiZero_OnePid2000"); if(!PiZero_OneP_Raw){printf("PiZero_OneP_Raw not found!\n"); lout->ls(); exit(1);}
  TH1D * Pion_OneP_Raw = (TH1D*) lout->FindObject("Pion_OnePid2000"); if(!Pion_OneP_Raw){printf("Pion_OneP_Raw not found!\n"); lout->ls(); exit(1);}
  TH1D * PiZero_TwoP_Raw = (TH1D*) lout->FindObject("PiZero_TwoPid2000"); if(!PiZero_TwoP_Raw){printf("PiZero_TwoP_Raw not found!\n"); lout->ls(); exit(1);}
  TH1D * Pion_TwoP_Raw = (TH1D*) lout->FindObject("Pion_TwoPid2000"); if(!Pion_TwoP_Raw){printf("Pion_TwoP_Raw not found!\n"); lout->ls(); exit(1);}
  
  TH1D * PiZero_OneP_Sig = (TH1D*) lout->FindObject("PiZero_OnePid2001"); if(!PiZero_OneP_Sig){printf("PiZero_OneP_Sig not found!\n"); lout->ls(); exit(1);}
  TH1D * Pion_OneP_Sig = (TH1D*) lout->FindObject("Pion_OnePid2001"); if(!Pion_OneP_Sig){printf("Pion_OneP_Sig not found!\n"); lout->ls(); exit(1);}
  TH1D * PiZero_TwoP_Sig = (TH1D*) lout->FindObject("PiZero_TwoPid2001"); if(!PiZero_TwoP_Sig){printf("PiZero_TwoP_Sig not found!\n"); lout->ls(); exit(1);}
  TH1D * Pion_TwoP_Sig = (TH1D*) lout->FindObject("Pion_TwoPid2001"); if(!Pion_TwoP_Sig){printf("Pion_TwoP_Sig not found!\n"); lout->ls(); exit(1);}
  
  TH1D * PiZero_OneP_Sel = (TH1D*) lout->FindObject("PiZero_OnePid2002"); if(!PiZero_OneP_Sel){printf("PiZero_OneP_Sel not found!\n"); lout->ls(); exit(1);}
  TH1D * Pion_OneP_Sel = (TH1D*) lout->FindObject("Pion_OnePid2002"); if(!Pion_OneP_Sel){printf("Pion_OneP_Sel not found!\n"); lout->ls(); exit(1);}
  TH1D * PiZero_TwoP_Sel = (TH1D*) lout->FindObject("PiZero_TwoPid2002"); if(!PiZero_TwoP_Sel){printf("PiZero_TwoP_Sel not found!\n"); lout->ls(); exit(1);}
  TH1D * Pion_TwoP_Sel = (TH1D*) lout->FindObject("Pion_TwoPid2002"); if(!Pion_TwoP_Sel){printf("Pion_TwoP_Sel not found!\n"); lout->ls(); exit(1);}
  
  TH1D * hhs[]={PiZero_OneP_Raw, Pion_OneP_Raw, PiZero_TwoP_Raw, Pion_TwoP_Raw, PiZero_OneP_Sig, Pion_OneP_Sig, PiZero_TwoP_Sig, Pion_TwoP_Sig, PiZero_OneP_Sel, Pion_OneP_Sel, PiZero_TwoP_Sel, Pion_TwoP_Sel};
  
  const int nh = sizeof(hhs)/sizeof(TH1D*);

  //const int linecol[]={1014, 1003, 1002, 1009, 1010, 1007, 1012, 1005, 1011, 1008, 1014, 1017};
  //const int linecol[]={1002, 1009, 1008, 1011};//, 1014, 1017, 1015};
  const int ngroup=4;//sizeof(linecol)/sizeof(int);
  
  for(int ii=0; ii<nh; ii++){
    printf("Setting ii %d %s\n", ii, hhs[ii]->GetName());
    style::ResetStyle(hhs[ii]);
    hhs[ii]->SetLineWidth(1);
    //hhs[ii]->SetLineColor(style::GetColor(linecol[ii%ngroup]));
    hhs[ii]->SetLineColor(style::GetColor(1002));

    if(ii>=2*ngroup){
      hhs[ii]->SetLineStyle(kDashed);
      hhs[ii]->SetLineWidth(2);
      hhs[ii]->SetLineColor(style::GetColor(1009));
    }
    else if(ii>=ngroup){
      hhs[ii]->SetLineStyle(kDotted);
      hhs[ii]->SetLineWidth(6);
      hhs[ii]->SetLineColor(style::GetColor(1008));
    }

    hhs[ii]->SetMaximum(hhs[ii]->GetBinContent(hhs[ii]->GetMaximumBin())*1.1);
    hhs[ii]->SetYTitle("#it{N}_{event}");
    hhs[ii]->SetXTitle("#it{M}_{X} (GeV/#it{c}^{2})");
  }

  TCanvas * c0=new TCanvas("cM","",600,400);
  style::PadSetup(c0);
  c0->SetBottomMargin(0.18);
  c0->SetRightMargin(0.03);
  c0->SetTopMargin(0.09);
  
  gStyle->SetOptStat(0);

  const double lx = 0.6;
  TLegend * lg = new TLegend(lx, 0.55, lx+0.3, 0.85);
  style::ResetStyle(lg,0.3);
  
  TString opt="hist ";

  TString slen;

  if(!IsPiPlusBeam()){
    lg->SetX1(0.17);
    lg->SetX2(0.47);
  }

  slen = "#pi^{0}"; CorrectFSString(slen);
  PiZero_OneP_Raw->Draw(opt);         lg->AddEntry(PiZero_OneP_Raw,slen+" p raw","l");
  PiZero_OneP_Sig->Draw(opt+" same"); lg->AddEntry(PiZero_OneP_Sig,slen+" p signal","l");
  PiZero_OneP_Sel->Draw(opt+" same"); lg->AddEntry(PiZero_OneP_Sel,slen+" p selected","l");
  lg->SetHeader(gConfig); lg->Draw();
  c0->Print(gOutdir+"summary_PiZero_OneP_RecoilM.png");
  lg->Clear();

  slen = "#pi^{+}"; CorrectFSString(slen);
  Pion_OneP_Raw->Draw(opt);         lg->AddEntry(Pion_OneP_Raw,slen+" p raw","l");
  Pion_OneP_Sig->Draw(opt+" same"); lg->AddEntry(Pion_OneP_Sig,slen+" p signal","l");
  Pion_OneP_Sel->Draw(opt+" same"); lg->AddEntry(Pion_OneP_Sel,slen+" p selected","l");
  lg->SetHeader(gConfig); lg->Draw();
  c0->Print(gOutdir+"summary_Pion_OneP_RecoilM.png");
  lg->Clear();

  lg->SetX1NDC(lx); lg->SetX2NDC(lx+0.3);
  slen = "#pi^{0}"; CorrectFSString(slen);
  PiZero_TwoP_Raw->Draw(opt);         lg->AddEntry(PiZero_TwoP_Raw,slen+" p p raw","l");
  PiZero_TwoP_Sig->Draw(opt+" same"); lg->AddEntry(PiZero_TwoP_Sig,slen+" p p signal","l");
  PiZero_TwoP_Sel->Draw(opt+" same"); lg->AddEntry(PiZero_TwoP_Sel,slen+" p p selected","l");
  lg->SetHeader(gConfig); lg->Draw();
  c0->Print(gOutdir+"summary_PiZero_TwoP_RecoilM.png");
  lg->Clear();

  slen = "#pi^{+}"; CorrectFSString(slen);
  Pion_TwoP_Raw->Draw(opt);         lg->AddEntry(Pion_TwoP_Raw,slen+" p p raw","l");
  Pion_TwoP_Sig->Draw(opt+" same"); lg->AddEntry(Pion_TwoP_Sig,slen+" p p signal","l");
  Pion_TwoP_Sel->Draw(opt+" same"); lg->AddEntry(Pion_TwoP_Sel,slen+" p p selected","l");
  lg->SetHeader(gConfig); lg->Draw();
  c0->Print(gOutdir+"summary_Pion_TwoP_RecoilM.png");
  lg->Clear();
}

void overdraw(const TString sin)
{
  TFile * fin = new TFile(sin);
  TTree * tree = (TTree*)fin->Get("tree");

  TList * lout = new TList;
  subdrawPiZero_OneP(tree, lout);
  subdrawPion_OneP(tree, lout);
  subdrawPiZero_TwoP(tree, lout);
  subdrawPion_TwoP(tree, lout);

  summary_RecoilP(lout);
  summary_RecoilM(lout);
  
  TFile * fout=new TFile(gOutdir+"summary.root","recreate");
  lout->Write();
  fout->Save();
  fout->Close();
}

int main(int argc, char * argv[])
{
  if(argc!=3){
    printf("need input file and out dir as argument\n");
    return 0;
  }

  gOutdir = argv[2];
  gOutdir+="/";

  const TString sin = argv[1];
  if(sin.Contains("GEANT4")){
    gConfig += "GEANT4 ";
    gOutdir+="GEANT4_";
  }
  else if(sin.Contains("FLUKA")){
    gConfig += "FLUKA ";
    gOutdir+="FLUKA_";
  }
  else{
    printf("Unknown generator! %s\n", sin.Data()); exit(1);
  }
  
  if(sin.Contains("Ar")){
    gConfig += "Ar ";
    gOutdir+="Ar_";
  }
  else{
    printf("Unknown target! %s\n", sin.Data()); exit(1);
  }
  
  if(sin.Contains("Piplus")){
    gConfig += "#pi^{+} ";
    gOutdir+="Piplus_";
  }
  else if(sin.Contains("Electron")){
    gConfig += "e ";
    gOutdir+="Electron_";
  }
  else{
    printf("Unknown beam particle! %s\n", sin.Data()); exit(1);
  }

  if(sin.Contains("1GeVc")){
    gConfig += "(1 GeV/#it{c}) ";
  }
  else if(sin.Contains("1GeV")){
    gConfig += "(1 GeV) ";
  }
  else{
    printf("Unknown beam energy! %s\n", sin.Data()); exit(1);
  }
  
  
  printf("\n\n=============================== Drawing %s input %s output %s ===============================\n", gConfig.Data(), sin.Data(), gOutdir.Data());

  style::SetGlobalStyle();
  style::fgkTextSize = 0.07; //0.05;
  style::fgkTitleSize = 0.07;
  style::fgkYTitleOffset = 0.85;
  style::fgkXTitleOffset = 1.1;

  overdraw(sin);
  return 0;
}