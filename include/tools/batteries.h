#ifndef BATTERIES_H
#define BATTERIES_H

#include "CrowTools.h"

class batteries;

template<>
InputParameters validParams<batteries>();

class batteries : public CrowTools
{
public:
  batteries(const std::string & name, InputParameters parameters);
  virtual ~batteries();
  double compute(double time);
};

#endif /* BATTERIES_H */
