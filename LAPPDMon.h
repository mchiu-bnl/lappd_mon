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

  int init_done = 0;
  static const int NCHPERBOARD = 34;	// Num ch's in each V1742
  static const int MAXCH=8*NCHPERBOARD;
  static const int MAXBOARDS = 10;	// Max Number of V1742's

  int NBOARDS = 2;		// Num V1742's
  int NCH = 0;			// Num total ch's
  int packetlist[MAXBOARDS] = { 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 }; // list of packets

  int draw_waveforms; // whether to draw the pulses

  CAEN_Calib *caen_calib[MAXBOARDS];

  TH1 *h_pulse[MAXBOARDS][MAXCH];
  TGraph *g_pulse[MAXBOARDS][MAXCH];
  TH2 *h2_data; 
  TH2 *h2_SED; 

  std::vector<std::pair<int,int>> *chmap;

  // Display of each waveform
  TCanvas *c_chdisplay[10];
  TCanvas *c_hitmap;
  TCanvas *c_hittime;
  TCanvas *c_hitampl;

  TH2 *h2_hitmap;   // hit distribution on detector
  TH1 *h_hittime;   // hit time distribution
  TH1 *h_hitampl;   // hit ampl distribution

};



#endif /* __LAPPDMON_H__ */
