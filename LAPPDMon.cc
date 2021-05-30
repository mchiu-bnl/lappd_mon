#include <iostream>

#include "LAPPDMon.h"
#include "caen_calib.h"

#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TMath.h>
#include <TFile.h>
#include <TStyle.h>
#include <TSystem.h>

#include <pmonitor/pmonitor.h>
#include <Event/Event.h>
#include <Event/EventTypes.h>

using namespace std;

LAPPDMon::LAPPDMon()
{
  gStyle->SetTitleFontSize(0.08);

  _init_done = 0;
  _first_run = 1;
  NBOARDS = 1;

  _draw_waveforms = 1;

  _run_number = -999999;

  /*
  // Laser Data just prior to test beam
  const char *caen_calibfname[MAXBOARDS] = {
    "/phenix/u/chiu/sphenix/lappd/caen_calibration/calib_0097_5G.dat",
    "/phenix/u/chiu/sphenix/lappd/caen_calibration/calib_0106_5G.dat",
    "/phenix/u/chiu/sphenix/lappd/caen_calibration/calib_0081_5G.dat",
    "/phenix/u/chiu/sphenix/lappd/caen_calibration/calib_0087_5G.dat",
    "xxx",
    "xxx",
    "xxx",
    "xxx",
    "xxx",
    "xxx"
  };
  */

  const char *caen_calibfname[MAXBOARDS] = {
    "caen_calibration/calib_0087_5G.dat",
    "caen_calibration/calib_0097_5G.dat",
    "caen_calibration/calib_0106_5G.dat",
    "caen_calibration/calib_0081_5G.dat",
    "xxx",
    "xxx",
    "xxx",
    "xxx",
    "xxx",
    "xxx"
  };

  // Get Mapping
  //TFile *mapping_tfile = new TFile("/phenix/u/chiu/sphenix/lappd/Mapping/L00i.map.v00.root","READ");
  //TFile *mapping_tfile = new TFile("Mapping/L02b.map.v00.root","READ");
  TFile *mapping_tfile = new TFile("Mapping/X00e.map.v00.root","READ");
  mapping_tfile->GetObject("bl_chmap",chmap);  // get the feech->position map
  cout << "map " << (*chmap)[10].first << endl;

  // Set the total number of channels
  NCH = NBOARDS*NCHPERBOARD;

  h2_data = new TH2F( "h2_data","Integrated data", 1024, -0.5, 1023.5, NCH, 0, NCH); 
  h2_SED = new TH2F( "h2_SED","V1742 Single Event Display", 34, -0.5, 33.5, 1024 , -0.5, 1023.5); 

  // Hit Distribution
  h2_hitmap = new TH2F( "h2_hitmap","hit distribution",32,0,32,32,0,32);
  h_hitampl = new TH1F( "h_hitampl","hit amplitudes",100,0,1000);
  h_hittime = new TH1F( "h_hittime","hit times",1024,0,1024);

  c_hitmap = new TCanvas("hitmap","hit distribution",800,800);
  c_hittime = new TCanvas("hittimes","hit times",640,480);
  c_hitampl = new TCanvas("hitampl","hit amplitudes",640,480);

  pupdate( c_hitmap, 30 );
  pupdate( c_hittime, 30 );
  pupdate( c_hitampl, 30 );

  TString name; 
  TString title;
  for (int iboard=0; iboard<NBOARDS; iboard++)
  {
    if ( _draw_waveforms )
    {
      name = "c_chdisplay"; name += iboard;
      title = "Ch Display, Board "; title += iboard;
      c_chdisplay[iboard] = new TCanvas(name,title,1200,850);
      //c_chdisplay[iboard]->Divide(8,5,-1,-1);
      c_chdisplay[iboard]->Divide(8,5);
      pupdate( c_chdisplay[iboard], 60 );
    }

    for (int ich=0; ich<NCHPERBOARD; ich++)
    {
      int datach = iboard*(NCHPERBOARD-2) + ich;

      if ( ich<(NCHPERBOARD-2) )
      {
        name = "h_pulse"; name += datach;
        title = "ch"; title += datach;
      }
      else
      {
        name = "h_tpulse"; name += iboard; name += ich-NCHPERBOARD+2;
        title = "tch_"; title += iboard; title += "_"; title += ich-NCHPERBOARD+2;
      }

      h_pulse[iboard][ich] = new TH1F(name, title, 1024 , -0.5, 1023.5); 

      if ( ich<(NCHPERBOARD-2) )
      {
        name = "g_pulse"; name += datach;
        title = "ch"; title += datach;
      }
      else
      {
        name = "g_tpulse"; name += iboard; name += ich-NCHPERBOARD+2;
        title = "tch_"; title += iboard; title += "_"; title += ich-NCHPERBOARD+2;
      }
      g_pulse[iboard][ich] = new TGraph(1024);
      g_pulse[iboard][ich]->SetName(name);
      g_pulse[iboard][ich]->SetTitle(title);
      g_pulse[iboard][ich]->SetMinimum(0);
      g_pulse[iboard][ich]->SetMaximum(4096);
      g_pulse[iboard][ich]->SetMarkerColor(4);
      g_pulse[iboard][ich]->SetMarkerSize(0.1);
      g_pulse[iboard][ich]->SetLineColor(4);
      g_pulse[iboard][ich]->GetXaxis()->SetLabelSize(0.08);
      g_pulse[iboard][ich]->GetYaxis()->SetLabelSize(0.065);
    }

    // Get CAEN Calibrations
    caen_calib[iboard] = new CAEN_Calib( caen_calibfname[iboard] );

  }

}


// Event Loop
int LAPPDMon::process_event (Event * e)
{

  // Look for begin run (or as backup runnumber change)
  if ( e->getEvtType() == 9 || e->getRunNumber() != _run_number )
  {
    if ( _first_run == 1 )
    {
      _first_run = 0;
    }

    int new_run_number = e->getRunNumber();
    cout << "Found Start of Run " << new_run_number << endl;

    // Reset histograms
    h2_hitmap->Reset(); // might have to delete and remake if geometry changes
    h_hittime->Reset();
    h_hitampl->Reset();
  }

  // End Run
  if ( e->getEvtType() == 12 )
  {
    cout << "Found End of Run " << _run_number << endl;
    // Save Histograms
    TString cmd;
    TString dir = "~/onlmon_out/"; dir += _run_number;
    cmd = "mkdir -p "; cmd += dir;
    gSystem->Exec( cmd );

    TString jpgname = dir; jpgname += "hit_time.jpg";
    h_hittime->SaveAs( jpgname );
  }

  _run_number = e->getRunNumber();

  int evtno = e->getEvtSequence();
  if ( evtno%1000 == 0 ) cout << "Event " << evtno << endl;

  static int counter = 0;

  // Reset Event by Event Plots
  h2_SED->Reset(); 
  for (int iboard=0; iboard<NBOARDS; iboard++)
  {
    for (int ich=0; ich<NCHPERBOARD; ich++)
    {
      h_pulse[iboard][ich]->Reset(); 
    }
  }

  // Loop over the packets
  for (int ipkt=0; ipkt<NBOARDS; ipkt++)
  {
    // Get Packet
    Packet *p = e->getPacket( packetlist[ipkt] );
    int board = packetlist[ipkt] - 2001;

    if (p)  // skip unfound packets
    {
      //cout << "Processing packet " << packetlist[ipkt] << endl;

      // Apply CAEN calib
      caen_calib[ipkt]->apply_calibs(p);

      int samples =  p->iValue(0,"SAMPLES");

      if ( counter<2 )
      {
        cout << "Nsamples = " << samples << endl;
        counter++;
      }

      for (int ich=0; ich<NCHPERBOARD; ich++)
      {

        for (int is=0; is<samples; is++)
        {
          g_pulse[board][ich]->SetPoint(is, is, caen_calib[ipkt]->corrected(ich, is));

        } // end loop over samples on one board
      } // end loop over ch's on one board

      delete p;

    }
  }

  // Find hits
  for (int iboard=0; iboard<NBOARDS; iboard++)
  {
    for (int ich=0; ich<NCHPERBOARD-2; ich++)
    {
      int data_ch = iboard*(NCHPERBOARD-2) + ich; // data_ch includes only data channels

      // Get ped
      Double_t *x = g_pulse[iboard][ich]->GetX();
      Double_t *y = g_pulse[iboard][ich]->GetY();
      Int_t n = g_pulse[iboard][ich]->GetN();

      Double_t ped = y[20];

      for (int isamp=20; isamp<n; isamp++)
      {
        if ( y[isamp]< ped-20 )
        {
          Double_t integ = 0.;
          for (int is=isamp-5; is<isamp+5; is++)
          {
            integ += (y[is] - ped);
          }

          integ = -integ;

          int xpos = (*chmap)[data_ch].first;
          int ypos = (*chmap)[data_ch].second;

          //cout << data_ch << "\t" << xpos << "\t" << ypos << "\t" << integ << "\t" << x[isamp] << endl;

          if ( integ>40 )
          {
            h2_hitmap->Fill( xpos, ypos, integ );
            h_hitampl->Fill( integ );
            h_hittime->Fill( x[isamp] );
          }

          break;
        }
      }
    }
  }

  c_hitmap->cd();
  h2_hitmap->Draw("colz");

  c_hittime->cd();
  h_hittime->Draw();

  c_hitampl->cd();
  h_hitampl->Draw();

  // Draw the waveforms
  TString name;
  if ( _draw_waveforms )
  {
    for (int iboard=0; iboard<NBOARDS; iboard++)
    {
      for (int ich=0; ich<NCHPERBOARD; ich++)
      {
        c_chdisplay[iboard]->cd(ich+1);

        //Double_t *x = g_pulse[iboard][ich]->GetX();
        Double_t *y = g_pulse[iboard][ich]->GetY();
        Int_t n = g_pulse[iboard][ich]->GetN();

        Double_t min = TMath::MinElement( n, y );
        Double_t max = TMath::MaxElement( n, y );

        g_pulse[iboard][ich]->SetMinimum( min-5 );
        g_pulse[iboard][ich]->SetMaximum( max+5 );
        g_pulse[iboard][ich]->Draw("ap");
      }

      //name = "pics/bd"; name += iboard; name += "evt"; name += evtno; name += ".png";
      //c_chdisplay[iboard]->SaveAs(name);
    }
  }

  return 0;
}

