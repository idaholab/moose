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
  params.addCoupledVar("strain_zz", "Variable containing the out-of-plane strain");
  return params;
}

ComputePlaneIncrementalStrain::ComputePlaneIncrementalStrain(const InputParameters & parameters) :
    Compute2DIncrementalStrain(parameters),
    _strain_zz(isCoupledScalar("strain_zz") ? coupledScalarValue("strain_zz") : _zero),
    _strain_zz_old(isCoupledScalar("strain_zz") ? coupledScalarValueOld("strain_zz") : _zero)
{
}

Real
ComputePlaneIncrementalStrain::computeDeformGradZZ()
{
    return _strain_zz[0];
}

Real
ComputePlaneIncrementalStrain::computeDeformGradZZold()
{
    return _strain_zz_old[0];
}
