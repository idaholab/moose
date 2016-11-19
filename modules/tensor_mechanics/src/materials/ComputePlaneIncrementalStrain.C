/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputePlaneIncrementalStrain.h"

template<>
InputParameters validParams<ComputePlaneIncrementalStrain>()
{
  InputParameters params = validParams<Compute2DIncrementalStrain>();
  params.addClassDescription("Compute strain increment for small strain under 2D planar assumptions.");
  params.addCoupledVar("scalar_strain", "Scalar variable for generalized plane strain");
  params.addCoupledVar("variable_strain", "Nonlinear variable for plane stress");

  return params;
}

ComputePlaneIncrementalStrain::ComputePlaneIncrementalStrain(const InputParameters & parameters) :
    Compute2DIncrementalStrain(parameters),
    _scalar_strain_coupled(isCoupledScalar("scalar_strain")),
    _scalar_strain(_scalar_strain_coupled ? coupledScalarValue("scalar_strain") : _zero),
    _scalar_strain_old(_scalar_strain_coupled ? coupledScalarValueOld("scalar_strain") : _zero),
    _variable_strain_coupled(isCoupled("variable_strain")),
    _variable_strain(_variable_strain_coupled ? coupledValue("variable_strain") : _zero),
    _variable_strain_old(_variable_strain_coupled ? coupledValueOld("variable_strain") : _zero)
{
  if (_variable_strain_coupled && _scalar_strain_coupled)
    mooseError("Must define only one of variable_strain or scalar_strain");
}

Real
ComputePlaneIncrementalStrain::computeGradDispZZ()
{
  if (_scalar_strain_coupled)
    return _scalar_strain[0];
  else
    return _variable_strain[_qp];
}

Real
ComputePlaneIncrementalStrain::computeGradDispZZOld()
{
  if (_scalar_strain_coupled)
    return _scalar_strain_old[0];
  else
    return _variable_strain_old[_qp];
}
