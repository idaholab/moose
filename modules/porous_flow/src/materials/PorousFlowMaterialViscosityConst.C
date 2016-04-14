/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialViscosityConst.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialViscosityConst>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<Real>("viscosity", "The viscosity, which is assumed constant for this material");
  params.addRequiredParam<UserObjectName>("PorousFlowVarNames_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates the viscosity assuming it is constant");
  return params;
}

PorousFlowMaterialViscosityConst::PorousFlowMaterialViscosityConst(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _input_viscosity(getParam<Real>("viscosity")),
    _porflow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowVarNames_UO")),

    _viscosity(declareProperty<Real>("PorousFlow_viscosity")),
    _viscosity_old(declarePropertyOld<Real>("PorousFlow_viscosity")),
    _dviscosity_dvar(declareProperty<std::vector<Real> >("dPorousFlow_viscosity_dvar"))
{
}

void
PorousFlowMaterialViscosityConst::initQpStatefulProperties()
{
  _viscosity[_qp] = _input_viscosity; // this becomes _viscosity_old[_qp] in the first call to computeQpProperties
}

void
PorousFlowMaterialViscosityConst::computeQpProperties()
{
  _viscosity[_qp] = _input_viscosity;

  const unsigned int num_var = _porflow_name_UO.num_v();
  _dviscosity_dvar[_qp].resize(num_var, 0.0);
}
