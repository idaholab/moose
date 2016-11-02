#ifndef PUMPCOASTDOWN_H
#define PUMPCOASTDOWN_H

#include <vector>
#include "InterpolationFunctions.h"
#include "CrowTools.h"

class pumpCoastdownExponential;

template<>
InputParameters validParams<pumpCoastdownExponential>();

class pumpCoastdownExponential : public CrowTools
{
public:
  pumpCoastdownExponential(const InputParameters & parameters);
  ~pumpCoastdownExponential();
  double compute (double time);

};

#endif /* PUMPCOASTDOWN_H_ */
