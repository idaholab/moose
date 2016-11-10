#ifndef BATTERIES_H
#define BATTERIES_H

#include "CrowTools.h"

class Batteries;

template<>
InputParameters validParams<Batteries>();

class Batteries : public CrowTools
{
public:
  Batteries(const InputParameters & parameters);
  virtual ~Batteries();
  double compute(double time);
};

#endif /* BATTERIES_H */
