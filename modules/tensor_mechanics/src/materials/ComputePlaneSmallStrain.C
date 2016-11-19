/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputePlaneSmallStrain.h"

template<>
InputParameters validParams<ComputePlaneSmallStrain>()
{
  InputParameters params = validParams<Compute2DSmallStrain>();
  params.addClassDescription("Compute a small strain under generalized plane strain assumptions where the out of plane strain is generally nonzero.");
  params.addCoupledVar("scalar_strain", "Scalar variable for generalized plane strain");
  params.addCoupledVar("variable_strain", "Nonlinear variable for plane stress");

  return params;
}

ComputePlaneSmallStrain::ComputePlaneSmallStrain(const InputParameters & parameters) :
    Compute2DSmallStrain(parameters),
    _scalar_strain_coupled(isCoupledScalar("scalar_strain")),
    _scalar_strain(_scalar_strain_coupled ? coupledScalarValue("scalar_strain") : _zero),
    _variable_strain_coupled(isCoupled("variable_strain")),
    _variable_strain(_variable_strain_coupled ? coupledValue("variable_strain") : _zero)
{
  if (_variable_strain_coupled && _scalar_strain_coupled)
    mooseError("Must define only one of variable_strain or scalar_strain");
}

Real
ComputePlaneSmallStrain::computeStrainZZ()
{
  if (_scalar_strain_coupled)
    return _scalar_strain[0];
  else
    return _variable_strain[_qp];
}
