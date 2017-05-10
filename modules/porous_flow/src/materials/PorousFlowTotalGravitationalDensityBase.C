/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowTotalGravitationalDensityBase.h"

template <>
InputParameters
validParams<PorousFlowTotalGravitationalDensityBase>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addClassDescription("Base class Material for porous medium density");
  return params;
}

PorousFlowTotalGravitationalDensityBase::PorousFlowTotalGravitationalDensityBase(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _gravdensity(declareProperty<Real>("density")),
    _dgravdensity_dvar(declareProperty<std::vector<Real>>("ddensity_dvar"))
{
}
