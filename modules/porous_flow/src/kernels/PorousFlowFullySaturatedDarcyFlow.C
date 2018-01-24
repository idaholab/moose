//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFullySaturatedDarcyFlow.h"

template <>
InputParameters
validParams<PorousFlowFullySaturatedDarcyFlow>()
{
  InputParameters params = validParams<PorousFlowFullySaturatedDarcyBase>();
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index corresponding to the fluid component for this kernel");
  params.addClassDescription("Darcy flux suitable for models involving a fully-saturated single "
                             "phase, multi-component fluid.  No upwinding is used");
  return params;
}

PorousFlowFullySaturatedDarcyFlow::PorousFlowFullySaturatedDarcyFlow(
    const InputParameters & parameters)
  : PorousFlowFullySaturatedDarcyBase(parameters),
    _mfrac(getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp")),
    _dmfrac_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
        "dPorousFlow_mass_frac_qp_dvar")),
    _fluid_component(getParam<unsigned int>("fluid_component"))
{
  if (_fluid_component >= _porousflow_dictator.numComponents())
    mooseError("PorousFlowFullySaturatedDarcyFlow: The dictator proclaims the number of fluid "
               "components is ",
               _porousflow_dictator.numComponents(),
               " whereas you set the fluid_component = ",
               _fluid_component,
               ".  Happyness equals perfection.");
}

Real
PorousFlowFullySaturatedDarcyFlow::mobility() const
{
  const unsigned ph = 0;
  return _mfrac[_qp][ph][_fluid_component] * PorousFlowFullySaturatedDarcyBase::mobility();
}

Real
PorousFlowFullySaturatedDarcyFlow::dmobility(unsigned pvar) const
{
  const unsigned ph = 0;
  const Real darcy_mob = PorousFlowFullySaturatedDarcyBase::mobility();
  const Real ddarcy_mob = PorousFlowFullySaturatedDarcyBase::dmobility(pvar);
  return _dmfrac_dvar[_qp][ph][_fluid_component][pvar] * darcy_mob +
         _mfrac[_qp][ph][_fluid_component] * ddarcy_mob;
}
