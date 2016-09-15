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
  params.addCoupledVar("strain_zz", "Variable containing the out-of-plane strain");
  return params;
}

ComputePlaneSmallStrain::ComputePlaneSmallStrain(const InputParameters & parameters) :
    Compute2DSmallStrain(parameters),
    _strain_zz(isCoupledScalar("strain_zz") ? coupledScalarValue("strain_zz") : _zero)
{
}

Real
ComputePlaneSmallStrain::computeStrainZZ()
{
  return _strain_zz[0];
}
