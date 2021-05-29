
#ifndef __CAEN_CALIB_H__
#define __CAEN_CALIB_H__

#include <string>
#include <stdint.h>
class Packet;

//
// Calibrations for one CAEN v1742 Board
//
class CAEN_Calib {

public:

  CAEN_Calib();
  CAEN_Calib(const char *calibfname);
  virtual ~CAEN_Calib() {};

  void load_calibs(const char *calibfname);
  void apply_calibs(Packet *p);
  void apply_sgl_adc_calib(Packet *p, const int feech);
  void apply_sgl_time_calib(Packet *p, const int feech);
  void peak_correction();

  float corrected(const int sample, const int channel) const;
  float caen_time(const int sample, const int channel) const;

  float corrected2(const int sample, const int channel) const;

  void set_cellCorrection(const int c) { cellCorrection = c; }
  void set_nsampleCorrection(const int n) { nsampleCorrection = n; }
  void set_timeCorrection(const int t) { timeCorrection = t; }

  //  virtual void identify(std::ostream& os = std::cout) const;

protected:

  int cellCorrection;     // correction to pedestal in each cell, and correlated peaks
  int nsampleCorrection;  // correction to correlated noise in last 30 samples
  int timeCorrection;     // correction to delay line to each cell

  float Tsamp;            // sampling period (0.2 ns for 5 GHz sampling)
  // caen calibration constants
  float cell[4][9][1024];     // cell pedestal correction [DRS4][ch][sample]
  float nsample[4][9][1024];  // sample pedestal correction [DRS4][ch][sample]
  float delay[4][1024];        // delay line time correction [DRS4][sample]
 
  // corrected ADC and Time
  float adc_corr[34][1024];   // [feech][sample]
  float time_corr[34][1024];  // [feech][sample]

  // corrected ADC in each 200 ps time bin
  float adc_corr2[34][1024];  // [feech][sample]
};

#endif

