/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSHALFGAUSSIANSINKFLUX_H
#define RICHARDSHALFGAUSSIANSINKFLUX_H

#include "SideIntegralVariablePostprocessor.h"

//Forward Declarations
class RichardsHalfGaussianSinkFlux;

template<>
InputParameters validParams<RichardsHalfGaussianSinkFlux>();

class RichardsHalfGaussianSinkFlux: public SideIntegralVariablePostprocessor
{
public:
  RichardsHalfGaussianSinkFlux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  FEProblem & _feproblem;
  Real _maximum;
  Real _sd;
  Real _centre;
};

#endif
