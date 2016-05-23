/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowRelativePermeabilityCorey.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowRelativePermeabilityCorey>()
{
  InputParameters params = validParams<PorousFlowRelativePermeabilityBase>();
  params.addRequiredParam<Real>("n_j", "The Corey exponent of phase j.");
  params.addClassDescription("This Material calculates relative permeability of either phase Sj, using the simple Corey model (Sj-Sjr)^n/(1-S1r-S2r)");
  return params;
}

PorousFlowRelativePermeabilityCorey::PorousFlowRelativePermeabilityCorey(const InputParameters & parameters) :
    PorousFlowRelativePermeabilityBase(parameters),
    _n(getParam<Real>("n_j"))
{
}

void
PorousFlowRelativePermeabilityCorey::computeQpProperties()
{
  _relative_permeability[_qp] = std::pow(_saturation_nodal[_qp][_phase_num], _n);
  _drelative_permeability_ds[_qp] = _n * std::pow(_saturation_nodal[_qp][_phase_num], _n - 1.0);
}
