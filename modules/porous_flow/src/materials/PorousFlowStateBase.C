/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowStateBase.h"

template<>
InputParameters validParams<PorousFlowStateBase>()
{
  InputParameters params = validParams<Material>();
  params.addCoupledVar("temperature", 20.0, "Fluid temperature");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names");
  params.addClassDescription("Base class for fluid state materials. Provides pressure, saturation and temperature material properties for all phases");
  return params;
}

PorousFlowStateBase::PorousFlowStateBase(const InputParameters & parameters) :
    Material(parameters),

    _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
    _temperature_nodal_var(coupledNodalValue("temperature")),
    _temperature_qp_var(coupledValue("temperature")),
    _temperature_varnum(coupled("temperature")),

    _porepressure_nodal(declareProperty<std::vector<Real> >("PorousFlow_porepressure_nodal")),
    _porepressure_nodal_old(declarePropertyOld<std::vector<Real> >("PorousFlow_porepressure_nodal")),
    _porepressure_qp(declareProperty<std::vector<Real> >("PorousFlow_porepressure_qp")),
    _gradp_qp(declareProperty<std::vector<RealGradient> >("PorousFlow_grad_porepressure_qp")),
    _dporepressure_nodal_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_nodal_dvar")),
    _dporepressure_qp_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_qp_dvar")),
    _dgradp_qp_dgradv(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_grad_porepressure_qp_dgradvar")),
    _dgradp_qp_dv(declareProperty<std::vector<std::vector<RealGradient> > >("dPorousFlow_grad_porepressure_qp_dvar")),

    _saturation_nodal(declareProperty<std::vector<Real> >("PorousFlow_saturation_nodal")),
    _saturation_nodal_old(declarePropertyOld<std::vector<Real> >("PorousFlow_saturation_nodal")),
    _saturation_qp(declareProperty<std::vector<Real> >("PorousFlow_saturation_qp")),
    _grads_qp(declareProperty<std::vector<RealGradient> >("PorousFlow_grad_saturation_qp")),
    _dsaturation_nodal_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_nodal_dvar")),
    _dsaturation_qp_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_qp_dvar")),
    _dgrads_qp_dgradv(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_grad_saturation_qp_dgradvar")),
    _dgrads_qp_dv(declareProperty<std::vector<std::vector<RealGradient> > >("dPorousFlow_grad_saturation_qp_dv")),

    _temperature_nodal(declareProperty<std::vector<Real> >("PorousFlow_temperature_nodal")),
    _temperature_qp(declareProperty<std::vector<Real> >("PorousFlow_temperature_qp")),
    _dtemperature_nodal_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_temperature_nodal_dvar")),
    _dtemperature_qp_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_temperature_qp_dvar"))
{
}

void
PorousFlowStateBase::initQpStatefulProperties()
{
}

void
PorousFlowStateBase::computeQpProperties()
{
}
