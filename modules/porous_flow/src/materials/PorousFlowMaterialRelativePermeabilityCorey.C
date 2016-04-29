/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterialRelativePermeabilityCorey.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialRelativePermeabilityCorey>()
{
  InputParameters params = validParams<PorousFlowMaterialRelativePermeabilityBase>();

  params.addRequiredParam<Real>("n_j", "The Corey exponent of phase j.");
  params.addRequiredParam<unsigned int>("phase", "The phase number j");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates relative permeability of either phase Sj, using the simple Corey model (Sj-Sjr)^n/(1-S1r-S2r)");
  return params;
}

PorousFlowMaterialRelativePermeabilityCorey::PorousFlowMaterialRelativePermeabilityCorey(const InputParameters & parameters) :
    PorousFlowMaterialRelativePermeabilityBase(parameters),

    _n(getParam<Real>("n_j"))
{
}

void
PorousFlowMaterialRelativePermeabilityCorey::computeQpProperties()
{
  _relative_permeability[_qp] = std::pow(_saturation_nodal[_qp][_phase_num], _n);
  _drelative_permeability_ds[_qp] = _n * std::pow(_saturation_nodal[_qp][_phase_num], _n - 1.0);
}
