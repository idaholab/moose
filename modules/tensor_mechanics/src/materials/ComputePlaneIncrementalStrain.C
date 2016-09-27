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
  params.addCoupledVar("scalar_strain_zz", "Variable containing the out-of-plane strain");
  return params;
}

ComputePlaneIncrementalStrain::ComputePlaneIncrementalStrain(const InputParameters & parameters) :
    Compute2DIncrementalStrain(parameters),
    _scalar_strain_zz(isCoupledScalar("scalar_strain_zz") ? coupledScalarValue("scalar_strain_zz") : _zero),
    _scalar_strain_zz_old(isCoupledScalar("scalar_strain_zz") ? coupledScalarValueOld("scalar_strain_zz") : _zero)
{
}

Real
ComputePlaneIncrementalStrain::computeDeformGradZZ()
{
    return _scalar_strain_zz[0];
}

Real
ComputePlaneIncrementalStrain::computeDeformGradZZold()
{
    return _scalar_strain_zz_old[0];
}
