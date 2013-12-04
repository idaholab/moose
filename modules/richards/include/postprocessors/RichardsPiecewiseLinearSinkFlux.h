#ifndef RICHARDSPIECEWISELINEARSINKFLUX_H
#define RICHARDSPIECEWISELINEARSINKFLUX_H

#include "SideIntegralVariablePostprocessor.h"

#include "LinInt.h"

//Forward Declarations
class RichardsPiecewiseLinearSinkFlux;

template<>
InputParameters validParams<RichardsPiecewiseLinearSinkFlux>();

/**
 * This postprocessor computes the fluid mass by integrating the density over the volume
 *
 */
class RichardsPiecewiseLinearSinkFlux: public SideIntegralVariablePostprocessor
{
public:
  RichardsPiecewiseLinearSinkFlux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  FEProblem & _feproblem;
  LinInt _sink_func;

};

#endif
