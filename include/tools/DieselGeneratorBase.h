#ifndef DIESELGENERATORBASE_H
#define DIESELGENERATORBASE_H

#include "CrowTools.h"

class DieselGeneratorBase;

template<>
InputParameters validParams<DieselGeneratorBase>();

class DieselGeneratorBase : public CrowTools
{
public:
  DieselGeneratorBase(const InputParameters & parameters);
  virtual ~DieselGeneratorBase();
  double compute(double time);
};


#endif /* DIESELGENERATORBASE_H_ */
