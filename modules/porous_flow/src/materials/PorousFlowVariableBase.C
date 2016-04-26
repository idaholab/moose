/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowVariableBase.h"

template<>
InputParameters validParams<PorousFlowVariableBase>()
{
  InputParameters params = validParams<Material>();
  params.addCoupledVar("temperature", 20.0, "Fluid temperature");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names");
  params.addClassDescription("Base class for thermophysical variable materials. Provides pressure, saturation and temperature material properties for all phases as required");
  return params;
}

PorousFlowVariableBase::PorousFlowVariableBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
    _num_phases(_dictator_UO.num_phases()),
    _num_components(_dictator_UO.num_components()),
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
PorousFlowVariableBase::initQpStatefulProperties()
{
  /// Resize the material properties which constain pressure, saturation and temperature
  _porepressure_nodal[_qp].resize(_num_phases);
  _porepressure_qp[_qp].resize(_num_phases);
  _porepressure_nodal_old[_qp].resize(_num_phases);
  _gradp_qp[_qp].resize(_num_phases);
  _dporepressure_nodal_dvar[_qp].resize(_num_phases);
  _dporepressure_qp_dvar[_qp].resize(_num_phases);
  _dgradp_qp_dgradv[_qp].resize(_num_phases);
  _dgradp_qp_dv[_qp].resize(_num_phases);

  _saturation_nodal[_qp].resize(_num_phases);
  _saturation_qp[_qp].resize(_num_phases);
  _saturation_nodal_old[_qp].resize(_num_phases);
  _grads_qp[_qp].resize(_num_phases);
  _dsaturation_nodal_dvar[_qp].resize(_num_phases);
  _dsaturation_qp_dvar[_qp].resize(_num_phases);
  _dgrads_qp_dgradv[_qp].resize(_num_phases);
  _dgrads_qp_dv[_qp].resize(_num_phases);

  _temperature_nodal[_qp].resize(_num_phases);
  _temperature_qp[_qp].resize(_num_phases);
  _dtemperature_nodal_dvar[_qp].resize(_num_phases);
  _dtemperature_qp_dvar[_qp].resize(_num_phases);
}

void
PorousFlowVariableBase::computeQpProperties()
{
  /// The number of PorousFlow variables
  unsigned int num_vars = _dictator_UO.num_variables();

  /// Prepare the derivative matrices with zeroes
  for (unsigned phase = 0; phase < _num_phases; ++phase)
  {
    _dporepressure_nodal_dvar[_qp][phase].assign(num_vars, 0.0);
    _dporepressure_qp_dvar[_qp][phase].assign(num_vars, 0.0);
    _dgradp_qp_dgradv[_qp][phase].assign(num_vars, 0.0);
    _dgradp_qp_dv[_qp][phase].assign(num_vars, RealGradient());
    _dsaturation_nodal_dvar[_qp][phase].assign(num_vars, 0.0);
    _dsaturation_qp_dvar[_qp][phase].assign(num_vars, 0.0);
    _dgrads_qp_dgradv[_qp][phase].assign(num_vars, 0.0);
    _dgrads_qp_dv[_qp][phase].assign(num_vars, RealGradient());
    _dtemperature_nodal_dvar[_qp][phase].assign(num_vars, 0.0);
    _dtemperature_qp_dvar[_qp][phase].assign(num_vars, 0.0);
  }
}
