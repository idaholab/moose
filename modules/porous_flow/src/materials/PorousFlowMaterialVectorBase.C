/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterialVectorBase.h"

template<>
InputParameters validParams<PorousFlowMaterialVectorBase>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("Base class for PorousFlow materials that combine phase-dependent properties into vectors expected by the kernels");
  return params;
}

PorousFlowMaterialVectorBase::PorousFlowMaterialVectorBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _num_components(_dictator.numComponents()),
    _num_var(_dictator.numVariables())
{
}
