#ifndef SIDEINTEGRALRZ_H
#define SIDEINTEGRALRZ_H

#include "SideIntegral.h"

//Forward Declarations
class SideIntegralRZ;

template<>
InputParameters validParams<SideIntegralRZ>();

class SideIntegralRZ : public SideIntegral
{
public:
  SideIntegralRZ(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

};

#endif
