#ifndef __LAPPDMON_H__
#define __LAPPDMON_H__

#include <vector>

class Event;
class TCanvas;
class TH1;
class TH2;
class TGraph;

class CAEN_Calib;


//using namespace std;

class LAPPDMon
{
public:
  LAPPDMon();
  virtual ~LAPPDMon() {}

  int process_event(Event *e);

protected:

  static const int NCHPERBOARD = 34;	// Num ch's in each V1742
  static const int MAXCH=8*NCHPERBOARD;
  static const int MAXBOARDS = 10;	// Max Number of V1742's
  static const int MAXPMTS = 3;	        // Max Number of PMTs

  int NBOARDS;		// Num V1742's
  int NPMTS;		// Num PMT's
  int NCH;			// Num total ch's
  int packetlist[MAXBOARDS] = { 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 }; // list of packets

  int _draw_waveforms; // whether to draw the pulses

  int _init_done = 0;
  int _run_number;
  int _first_run;

  CAEN_Calib *caen_calib[MAXBOARDS];

  TH1 *h_pulse[MAXBOARDS][NCHPERBOARD];
  TGraph *g_pulse[MAXBOARDS][NCHPERBOARD];
  TH2 *h2_all[MAXCH]; 

  std::vector<std::pair<int,int>> *chmap;

  // Display of each waveform
  TCanvas *c_chdisplay[MAXBOARDS];
  TCanvas *c_hitmap[MAXPMTS];
  TCanvas *c_hittime;
  TCanvas *c_hitampl;

  TH2 *h2_hitmap[MAXPMTS];   // hit distribution on detector
  TH1 *h_hittime[MAXPMTS];   // hit time distribution
  TH1 *h_hitampl[MAXPMTS];   // hit ampl distribution

};



#endif /* __LAPPDMON_H__ */
