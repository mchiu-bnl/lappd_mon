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

  static int counter = 0;

  int evtno = e->getEvtSequence();
  if ( counter<10 || evtno%1000 == 0 )
  {
    cout << "Processing Event " << evtno << endl;
    counter++;
  }

  lappdmon->process_event(e);

  return 0;
}
