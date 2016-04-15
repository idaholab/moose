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
  InputParameters params = validParams<Material>();

  params.addRequiredParam<Real>("n_j", "The Corey exponent of phase j.");
  params.addRequiredParam<unsigned int>("phase", "The phase number j");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates relative permeability of either phase Sj, using the simple Corey model (Sj-Sjr)^n/(1-S1r-S2r)");
  return params;
}

PorousFlowMaterialRelativePermeabilityCorey::PorousFlowMaterialRelativePermeabilityCorey(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _n(getParam<Real>("n_j")),
    _phase_num(getParam<unsigned int>("phase")),
    _porflow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),

    _saturation(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation")),
    _dsaturation_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_dvar")),
    _relative_permeability(declareProperty<Real>("PorousFlow_fluid_relative_permeability" + Moose::stringify(_phase_num))),
    _drelative_permeability_dvar(declareProperty<std::vector<Real> >("dPorousFlow_fluid_relative_permeability" + Moose::stringify(_phase_num) + "_dvar"))
{
}

void
PorousFlowMaterialRelativePermeabilityCorey::computeQpProperties()
{
  _drelative_permeability_dvar[_qp].resize(_porflow_name_UO.num_v());  
  _relative_permeability[_qp] = std::pow(_saturation[_qp][_phase_num],_n);

  for (unsigned v = 0; v < _porflow_name_UO.num_v(); ++v)
    _drelative_permeability_dvar[_qp][v] = _n*std::pow(_saturation[_qp][_phase_num],_n-1.0)* _dsaturation_dvar[_qp][_phase_num][v];
}

