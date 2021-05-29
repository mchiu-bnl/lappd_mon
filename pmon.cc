#include <iostream>
#include <pmonitor/pmonitor.h>
#include "pmon.h"
#include "LAPPDMon.h"

#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TGraph.h>

using namespace std;

LAPPDMon *lappdmon;
int init_done = 0;

// Executed Once at Beginning of Running
int pinit()
{
  if (init_done) return 1;
  init_done = 1;

  cout << "pinit()" << endl;

  lappdmon = new LAPPDMon();

  return 0;
}


// Event Loop
int process_event (Event * e)
{

  int evtno = e->getEvtSequence();
  cout << "Processing Event " << evtno << endl;

  lappdmon->process_event(e);

  return 0;
}
