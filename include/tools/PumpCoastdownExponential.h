#ifndef PUMPCOASTDOWN_H
#define PUMPCOASTDOWN_H

#include <vector>
#include "InterpolationFunctions.h"
#include "CrowTools.h"

class PumpCoastdownExponential;

template<>
InputParameters validParams<PumpCoastdownExponential>();

class PumpCoastdownExponential : public CrowTools
{
public:
  PumpCoastdownExponential(const InputParameters & parameters);
  ~PumpCoastdownExponential();
  double compute (double time);

};

#endif /* PUMPCOASTDOWN_H_ */
