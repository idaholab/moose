/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorFlowMaterialRelativePermeabilityConst.h"


template<>
InputParameters validParams<PorFlowMaterialRelativePermeabilityConst>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<UserObjectName>("PorFlowVarNames_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates the relative permeability assuming it is constant");
  return params;
}

PorFlowMaterialRelativePermeabilityConst::PorFlowMaterialRelativePermeabilityConst(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _porflow_name_UO(getUserObject<PorFlowVarNames>("PorFlowVarNames_UO")),

  _relative_permeability(declareProperty<std::vector<Real> > ("PorFlow_relative_permeability")),
  _drelative_permeability_dvar(declareProperty<std::vector<std::vector<Real> > > ("dPorFlow_relative_permeability_dvar"))
{
}


void
PorFlowMaterialRelativePermeabilityConst::computeQpProperties()
{
  unsigned int number_phases=2;
  int i;
  
  // FIX THIS: need to get number of phases.
  _relative_permeability[_qp].resize(number_phases,1.0);

  const unsigned int num_var = _porflow_name_UO.num_v();
  _drelative_permeability_dvar[_qp].resize(number_phases);
  for (unsigned int i = 0 ; i < number_phases; i++)
    _drelative_permeability_dvar[_qp][i].resize(num_var,0.0);

}

