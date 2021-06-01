#include <iostream>
#include <iomanip>
#include <fstream>

#include "caen_calib.h"
#include "Event/packet.h"

using namespace std;

CAEN_Calib::CAEN_Calib()
{
  cellCorrection = 1;
  nsampleCorrection = 1;
  timeCorrection = 1;
  Tsamp = 0.2;
}

CAEN_Calib::CAEN_Calib( const char *calibfname ) 
{ 
  cellCorrection = 1;
  nsampleCorrection = 1;
  timeCorrection = 1;
  Tsamp = 0.2;

  load_calibs( calibfname );
}

void CAEN_Calib::load_calibs( const char *calibfname )
{

  // Open Calibration File
  ifstream IN;
  IN.open(calibfname,ios_base::in);
  if ( ! IN.is_open())
  {
    cerr << "ERROR opening file " << calibfname << endl;
    return;
  }
  else
  {
    cout << "Reading calibrations from " << calibfname << endl;
  }

  int index;
  for (int ichip = 0; ichip < 4; ichip++)
  {
    for (int ich = 0; ich < 1024; ich++)
    {
      IN >> index
        >> cell[ichip][0][index]
        >> cell[ichip][1][index]
        >> cell[ichip][2][index]
        >> cell[ichip][3][index]
        >> cell[ichip][4][index]
        >> cell[ichip][5][index]
        >> cell[ichip][6][index]
        >> cell[ichip][7][index]
        >> cell[ichip][8][index]
        >> delay[ichip][index]
        >> nsample[ichip][0][index]
        >> nsample[ichip][1][index]
        >> nsample[ichip][2][index]
        >> nsample[ichip][3][index]
        >> nsample[ichip][4][index]
        >> nsample[ichip][5][index]
        >> nsample[ichip][6][index]
        >> nsample[ichip][7][index]
        >> nsample[ichip][8][index];

      if ( index != ich ) cout << "ERROR, index != ich " << index << "\t" << ich << endl;
    }
  }
  IN.close();
}


// Apply calibrations
void CAEN_Calib::apply_calibs(Packet *p)
{
  // Calibrate ADC of all data channels, including TR channels
  if ( cellCorrection )
  {
    for (int ich=0; ich<34; ich++)
    {
      apply_sgl_adc_calib( p, ich );
    }

    // Peak Correction
    peak_correction();
  }

  // Now calibrate Time
  if ( timeCorrection )
  {
    for (int ich=0; ich<34; ich++)
    {
        apply_sgl_time_calib( p, ich );
    }
  }

}


// Apply ADC Calibrations on Single Channel
void CAEN_Calib::apply_sgl_adc_calib(Packet *p, const int feech)
{
  if ( cellCorrection == 0 ) return;

  int chip = feech/8;
  int ch = feech%8;

  if ( feech==32 )      // TR0
  {
    chip = 0;
    ch = 8;
  }
  else if ( feech==33 ) // TR1
  {
    chip = 2;
    ch = 8;
  }
  else if ( feech<0 || feech>33 )
  {
    cerr << "ERROR invalid feech " << feech << endl;
    return;
  }

  int cell_index = p->iValue(chip,"INDEXCELL");

  // ADC corrections
  for (int isamp = 0; isamp < 1024; isamp++)
  {
    // get adc value
    if ( feech<32 )       adc_corr[feech][isamp] = p->iValue(isamp,feech);
    else if ( feech==32 ) adc_corr[feech][isamp] = p->iValue(isamp,"TR0");
    else if ( feech==33 ) adc_corr[feech][isamp] = p->iValue(isamp,"TR1");

    // cell pedestal correction
    adc_corr[feech][isamp] -= cell[chip][ch][(cell_index+isamp)%1024];
    adc_corr2[feech][isamp] = adc_corr[feech][isamp];

    // nsample pedestal correction
    if ( nsampleCorrection==1 )
    {
      adc_corr[feech][isamp] -= nsample[chip][ch][isamp];
      adc_corr2[feech][isamp] = adc_corr[feech][isamp];
    }
  }
}

// Apply Time Calibrations on Single Channel
void CAEN_Calib::apply_sgl_time_calib(Packet *p, const int feech)
{
  // Time Corrections
  if ( timeCorrection==0 ) return;

  int chip = feech/8;

  if ( feech==32 )      // TR0
  {
    chip = 0;
  }
  else if ( feech==33 ) // TR1
  {
    chip = 2;
  }
  else if ( feech<0 || feech>33 )
  {
    cerr << "ERROR invalid feech " << feech << endl;
    return;
  }

  int cell_index = p->iValue(chip,"INDEXCELL");

  float t0 = delay[chip][cell_index];

  time_corr[feech][0] = 0.;
  for (int isamp = 1; isamp < 1024; isamp++)
  {
    t0 = delay[chip][ (cell_index+isamp)%1024 ] - t0;

    if ( t0>0 )
    {
      time_corr[feech][isamp] = time_corr[feech][isamp-1] + t0;
    }
    else
    {
      time_corr[feech][isamp] = time_corr[feech][isamp-1] + t0 + Tsamp*1024;
    }

    t0 = delay[chip][ (cell_index+isamp)%1024 ];
  }

  // Now correct ADC to expected value for sampling at every 200 ps
  adc_corr[feech][0] = adc_corr[feech][1];
  adc_corr2[feech][0] = adc_corr[feech][0];

  float vcorr = 0.;

  int k = 0;  // time ordered index
  for (int isamp = 0; isamp < 1024; isamp++)
  {
    // find index k that corresponds to sample that is the next in time
    while ((k<1024-1) && (time_corr[feech][k]<(isamp*Tsamp)))
    {
      k++;
    }

    vcorr = ((adc_corr[feech][k] - adc_corr[feech][k-1])/(time_corr[feech][k] - time_corr[feech][k-1]))*((isamp*Tsamp)-time_corr[feech][k-1]);
    adc_corr2[feech][isamp] = adc_corr[feech][k-1] + vcorr;
    k--;
  }

}

void CAEN_Calib::peak_correction() 
{
  // copy the 2nd sample to the 1st
  for (int ich=0; ich<34; ich++)
  {
    adc_corr[ich][0] = adc_corr[ich][1];
  }

  // Now loop through each DRS4 chip
  for (int idrs=0; idrs<4; idrs++)
  {
    // go through each sample in a chip
    for (int isamp=1; isamp<1024; isamp++)
    {
      int offset = 0;

      // look for correlated spikes across all ch's in a chip
      for (int ich=0; ich<8; ich++)
      {
        int feech = idrs*8 + ich;
        if ( isamp==1 )
        {
          if ( (adc_corr[feech][2] - adc_corr[feech][1]) > 30 ) {
            offset++;
          }
          else if ( ((adc_corr[feech][3]-adc_corr[feech][1]) > 30) && ((adc_corr[feech][3]-adc_corr[feech][2])>30) )
          {
            offset++;
          }
        }
        else
        {
          if ( isamp==1023 && ((adc_corr[feech][1022]-adc_corr[feech][1023])>30) )
          {
            offset++;
          }
          else
          {
            if ( (adc_corr[feech][isamp-1]-adc_corr[feech][isamp]) > 30 )
            {
              if ( (adc_corr[feech][isamp+1]-adc_corr[feech][isamp]) > 30  )
              {
                offset++;
              }
              else
              {
                if (isamp==1022 || (adc_corr[feech][isamp+2]-adc_corr[feech][isamp])>30 )
                {
                  offset++;
                }
              }
            }
          }
        }
      }

      // Found correlated noise
      if ( offset==8 )
      {
        // Note: Need to correct trigger channels!!!
        for (int ich=0; ich<8; ich++)
        {
          int feech = idrs*8 + ich;
          if ( isamp==1 )
          {
            if ( (adc_corr[feech][2]-adc_corr[feech][1]) > 30 )
            {
              adc_corr[feech][0] = adc_corr[feech][2];
              adc_corr[feech][1] = adc_corr[feech][2];
            }
            else
            {
              adc_corr[feech][0] = adc_corr[feech][3];
              adc_corr[feech][1] = adc_corr[feech][3];
              adc_corr[feech][2] = adc_corr[feech][3];
            }
          }
          else
          {
            if ( isamp==1023 )
            {
              adc_corr[feech][1023] = adc_corr[feech][1022];
            }
            else
            {
              if ( (adc_corr[feech][isamp+1] - adc_corr[feech][isamp]) > 30 )
              {
                adc_corr[feech][isamp] = ((adc_corr[feech][isamp+1] + adc_corr[feech][isamp-1])/2);
              }
              else
              {
                if ( isamp==1022 )
                {
                  adc_corr[feech][1022] = adc_corr[feech][1021];
                  adc_corr[feech][1023] = adc_corr[feech][1021];
                }
                else
                {
                  adc_corr[feech][isamp] = ((adc_corr[feech][isamp+2] + adc_corr[feech][isamp-1])/2);
                  adc_corr[feech][isamp+1] = ((adc_corr[feech][isamp+2] + adc_corr[feech][isamp-1])/2);
                }
              }
            }
          }
        }
      }
    }
  }
}

float CAEN_Calib::corrected(const int channel, const int sample) const
{
  if ( sample < 0 || sample >1023 || channel < 0 || channel > 33) return 0;
  //cout << "sample " << sample << "\t" << channel << endl;
  return adc_corr[channel][sample];
}

float CAEN_Calib::corrected2(const int channel, const int sample) const
{
  if ( sample < 0 || sample >1023 || channel < 0 || channel > 33) return 0;
  //cout << "sample " << sample << "\t" << channel << endl;
  return adc_corr2[channel][sample];
}

float CAEN_Calib::caen_time(const int channel, const int sample) const
{
  if ( sample < 0 || sample >1023 || channel < 0 || channel > 31) return 0;
  return time_corr[channel][sample];
}

