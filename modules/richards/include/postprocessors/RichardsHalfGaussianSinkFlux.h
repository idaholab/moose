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

/**
 * Postprocessor that records the mass flux from porespace to
 * a half-gaussian sink.  (Positive if fluid is being removed from porespace.)
 * flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and flux out = max otherwise
 */
class RichardsHalfGaussianSinkFlux: public SideIntegralVariablePostprocessor
{
public:
  RichardsHalfGaussianSinkFlux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  FEProblem & _feproblem;

  /// flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and flux out = max otherwise
  Real _maximum;

  /// flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and flux out = max otherwise
  Real _sd;

  /// flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and flux out = max otherwise
  Real _centre;
};

#endif
