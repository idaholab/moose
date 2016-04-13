/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorFlowMaterialPorosityConst.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorFlowMaterialPorosityConst>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<Real>("porosity", "The porosity, which is assumed constant for this material");
  params.addRequiredParam<UserObjectName>("PorFlowVarNames_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates the porosity assuming it is constant");
  return params;
}

PorFlowMaterialPorosityConst::PorFlowMaterialPorosityConst(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _input_porosity(getParam<Real>("porosity")),
    _porflow_name_UO(getUserObject<PorFlowVarNames>("PorFlowVarNames_UO")),

    _porosity(declareProperty<Real>("PorFlow_porosity")),
    _porosity_old(declarePropertyOld<Real>("PorFlow_porosity")),
    _dporosity_dvar(declareProperty<std::vector<Real> >("dPorFlow_porosity_dvar"))
{
}

void
PorFlowMaterialPorosityConst::initQpStatefulProperties()
{
  _porosity[_qp] = _input_porosity; // this becomes _porosity_old[_qp] in the first call to computeQpProperties
}

void
PorFlowMaterialPorosityConst::computeQpProperties()
{
  _porosity[_qp] = _input_porosity;

  const unsigned int num_var = _porflow_name_UO.num_v();
  _dporosity_dvar[_qp].resize(num_var, 0.0);
}

