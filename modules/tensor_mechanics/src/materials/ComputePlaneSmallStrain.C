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
  params.addCoupledVar("scalar_strain_zz", "Scalar variable containing the out-of-plane strain for generalized plane strain");
  params.addCoupledVar("strain_zz", "Nonlinear variable containing the out-of-plane strain for plane stress");

  return params;
}

ComputePlaneSmallStrain::ComputePlaneSmallStrain(const InputParameters & parameters) :
    Compute2DSmallStrain(parameters),
    _scalar_strain_zz_coupled(isCoupledScalar("scalar_strain_zz")),
    _scalar_strain_zz(_scalar_strain_zz_coupled ? coupledScalarValue("scalar_strain_zz") : _zero),
    _strain_zz_coupled(isCoupled("strain_zz")),
    _strain_zz(_strain_zz_coupled ? coupledValue("strain_zz") : _zero)
{
}

Real
ComputePlaneSmallStrain::computeStrainZZ()
{
  if (_scalar_strain_zz_coupled)
    return _scalar_strain_zz[0];
  else
    return _strain_zz[_qp];
}
