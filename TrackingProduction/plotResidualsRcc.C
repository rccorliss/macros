bool debug=false;
//works for acts, but not for helix:  TString cut="m_quality<100 && sqrt(clusgy*clusgy+clusgx*clusgx)>30 && sqrt(clusgy*clusgy+clusgx*clusgx)<78";
//TString cut="sqrt(clusgy*clusgy+clusgx*clusgx)>30 && sqrt(clusgy*clusgy+clusgx*clusgx)<78";
TString cut="cluslayer>6 && cluslayer<54 && abs(clusgz)<100 && abs(clusgz)>0.1";


void wrappedDraw(TTree* tree, const char* draw, const char* cut, const char* option){
  if (debug) printf("trying residualtree->Draw(\"%s\",\"%s\",\"%s\")\n",draw,cut,option);
  tree->Draw(draw,cut,option);
}

void plotResidualsRcc(const char* filename){
  TFile *f=TFile::Open(filename);
  TTree* residualtree=(TTree*)f->Get("residualtree");
  if (!residualtree){
    printf("Error: could not find residualtree in file %s\n",filename);
    return;
  }
  TCanvas *c = new TCanvas("c","c",1600,900);
  c->Divide(4,3);
  c->cd(1);
  float bound=1;
  int pad=1;
  for (int charge=1;charge>-2; charge-=2){
    for (int side=1;side>-2;side-=2){
      c->cd(pad);
      wrappedDraw(residualtree,"(atan2(clusgy,clusgx)-atan2(stategy,stategx))*sqrt(clusgy*clusgy+clusgx*clusgx):cluslayer",Form("abs((atan2(clusgy,clusgx)-atan2(stategy,stategx))*sqrt(clusgy*clusgy+clusgx*clusgx))<%f && m_charge==%d && clusgz*%d>0  && %s",bound,charge,side, cut.Data()),"colz");
      TH3* ht=((TH3*)c->cd(pad)->FindObject("htemp"));
      ht->SetTitle(Form("d(r*phi) vs layer number (z%c0, charge=%c1);layer;d(rphi)",side>0?'>':'<',charge>0?'+':'-'));
      ht->SetStats(1);
      pad++;
    }
  }
  bound=0.25;
  for (int charge=1;charge>-2; charge-=2){
    for (int side=1;side>-2;side-=2){
      c->cd(pad);
      wrappedDraw(residualtree,"(atan2(clusgy,clusgx)-atan2(stategy,stategx))*sqrt(clusgy*clusgy+clusgx*clusgx):cluslayer",Form("abs((atan2(clusgy,clusgx)-atan2(stategy,stategx))*sqrt(clusgy*clusgy+clusgx*clusgx))<%f && m_charge==%d && clusgz*%d>0  && %s",bound,charge,side,cut.Data()),"colz");
      TH3* ht=((TH3*)c->cd(pad)->FindObject("htemp"));
      ht->SetTitle(Form("d(r*phi) vs layer number (z%c0, charge=%c1);layer;d(rphi)",side>0?'>':'<',charge>0?'+':'-'));
      ht->SetStats(1);
      pad++;
    }
  }
  bound=0.5;
  for (int charge=1;charge>-2; charge-=2){
    c->cd(pad);
    wrappedDraw(residualtree,"(atan2(clusgy,clusgx)-atan2(stategy,stategx))*sqrt(clusgy*clusgy+clusgx*clusgx):clusgz",Form("abs((atan2(clusgy,clusgx)-atan2(stategy,stategx))*sqrt(clusgy*clusgy+clusgx*clusgx))<%f && m_charge==%d && %s",bound,charge,cut.Data()),"colz");
    TH3* ht=((TH3*)c->cd(pad)->FindObject("htemp"));
    ht->SetTitle(Form("d(r*phi) vs clusz (charge=%c1);cluster z;d(rphi)",charge>0?'+':'-'));
    ht->SetStats(1);
    pad++;
    c->cd(pad);
    wrappedDraw(residualtree,"sqrt(clusgy*clusgy+clusgx*clusgx)-sqrt(stategy*stategy+stategx*stategx):clusgz",Form("abs(sqrt(clusgy*clusgy+clusgx*clusgx)-sqrt(stategy*stategy+stategx*stategx))<%f && m_charge==%d && %s",bound*0.1,charge,cut.Data()),"colz");
    ht=((TH3*)c->cd(pad)->FindObject("htemp"));
    ht->SetTitle(Form("dr vs clusz (charge=%c1);cluster z;dr",charge>0?'+':'-'));
    ht->SetStats(1);
    pad++;
  }


 
  c = new TCanvas("c2","c2",1600,900);
  c->Divide(4,3);
  c->cd(1);
  bound=5;
  pad=1;
  for (int charge=1;charge>-2; charge-=2){
    for (int side=1;side>-2;side-=2){
      c->cd(pad);
      wrappedDraw(residualtree,"cluslayer:clusgz",Form("abs(m_dcaxy)%c%f && m_charge==%d && %s",side>0?'>':'<',bound,charge, cut.Data()),"colz");
      TH3* ht=((TH3*)c->cd(pad)->FindObject("htemp"));
      ht->SetTitle(Form("hits with track dcaxy%c%1.2f, charge=%c1);z;layer",side>0?'>':'<',bound,charge>0?'+':'-'));
      ht->SetStats(1);
      pad++;
    }
  } 
   bound=1;
  for (int charge=1;charge>-2; charge-=2){
    for (int side=1;side>-2;side-=2){
      c->cd(pad);
      wrappedDraw(residualtree,"cluslayer:clusgz",Form("abs(m_dcaxy)%c%f && m_charge==%d && %s",side>0?'>':'<',bound,charge,cut.Data()),"colz");
      TH3* ht=((TH3*)c->cd(pad)->FindObject("htemp"));
      ht->SetTitle(Form("hits with track dcaxy%c%1.2f, charge=%c1);z;layer",side>0?'>':'<',bound,charge>0?'+':'-'));
      ht->SetStats(1);
      pad++;
    }
  }

  bound=10;
  for (int charge=1;charge>-2; charge-=2){
    for (int side=1;side>-2;side-=2){
      c->cd(pad);
      wrappedDraw(residualtree,"m_dcaxy",Form("abs(m_dcaxy)<%f && clusgz%c0 && m_charge==%d && %s",bound,side>0?'>':'<',charge,cut.Data()),"colz");
      TH3* ht=((TH3*)c->cd(pad)->FindObject("htemp"));
      ht->SetTitle(Form("track dcaxy (charge=%c1, z%c0);dca (cm)",charge>0?'+':'-',side>0?'>':'<'));
      ht->SetStats(1);
      pad++;
    }
  }

  
}
