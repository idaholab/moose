/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFullySaturatedHeatAdvection.h"

template <>
InputParameters
validParams<PorousFlowFullySaturatedHeatAdvection>()
{
  InputParameters params = validParams<PorousFlowFullySaturatedDarcyBase>();
  params.addClassDescription("Heat flux that arises from the advection of a fully-saturated single "
                             "phase fluid.  No upwinding is used");
  return params;
}

PorousFlowFullySaturatedHeatAdvection::PorousFlowFullySaturatedHeatAdvection(
    const InputParameters & parameters)
  : PorousFlowFullySaturatedDarcyBase(parameters),
    _enthalpy(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_qp")),
    _denthalpy_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_enthalpy_qp_dvar"))
{
}

Real
PorousFlowFullySaturatedHeatAdvection::mobility() const
{
  const unsigned ph = 0;
  return _enthalpy[_qp][ph] * PorousFlowFullySaturatedDarcyBase::mobility();
}

Real
PorousFlowFullySaturatedHeatAdvection::dmobility(unsigned pvar) const
{
  const unsigned ph = 0;
  const Real darcy_mob = PorousFlowFullySaturatedDarcyBase::mobility();
  const Real ddarcy_mob = PorousFlowFullySaturatedDarcyBase::dmobility(pvar);
  return _denthalpy_dvar[_qp][ph][pvar] * darcy_mob + _enthalpy[_qp][ph] * ddarcy_mob;
}
