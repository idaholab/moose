/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSPIECEWISELINEARSINKFLUX_H
#define RICHARDSPIECEWISELINEARSINKFLUX_H

#include "SideIntegralVariablePostprocessor.h"
#include "LinearInterpolation.h"
#include "RichardsPorepressureNames.h"

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
  bool _use_mobility;
  bool _use_relperm;
  LinearInterpolation _sink_func;

  const RichardsPorepressureNames & _pp_name_UO;
  unsigned int _pvar;

  MaterialProperty<std::vector<Real> > &_viscosity;
  MaterialProperty<RealTensorValue> & _permeability;
  MaterialProperty<std::vector<Real> > &_rel_perm;
  MaterialProperty<std::vector<Real> > &_density;

};

#endif
