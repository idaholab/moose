/*
 * pumpCoastdown.h
 *
 *  Created on: Aug 8, 2012
 *      Author: mandd
 */

#ifndef PUMPCOASTDOWN_H_
#define PUMPCOASTDOWN_H_

#include <vector>
#include "Interpolation_Functions.h"
#include "RavenTools.h"

class pumpCoastdownExponential;

template<>
InputParameters validParams<pumpCoastdownExponential>();

class pumpCoastdownExponential : public RavenTools{

public:
  pumpCoastdownExponential(const std::string & name, InputParameters parameters);
  ~pumpCoastdownExponential();
  double compute (double time);

protected:

//Interpolation_Functions _interpolation;
};

//class pumpCoastdownCurve;
//
//template<>
//InputParameters validParams<pumpCoastdownCurve>();
//
//class pumpCoastdownCurve : public RavenTools{
//
//public:
//  pumpCoastdownCurve(const std::string & name, InputParameters parameters);
//  ~pumpCoastdownCurve();
//  double compute (double time);
//
//protected:
//  Interpolation_Functions _interpolation;
//};


#endif /* PUMPCOASTDOWN_H_ */
