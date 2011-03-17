#ifndef SIDEFLUXINTEGRALRZ_H
#define SIDEFLUXINTEGRALRZ_H

#include "SideFluxIntegral.h"

//Forward Declarations
class SideFluxIntegralRZ;

template<>
InputParameters validParams<SideFluxIntegralRZ>();

/**
 * This postprocessor computes a side integral of the mass flux.
 */
class SideFluxIntegralRZ : public SideFluxIntegral
{
public:
  SideFluxIntegralRZ(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

};

#endif
