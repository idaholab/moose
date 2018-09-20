//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElasticModel.h"

#include "SymmElasticityTensor.h"

registerMooseObject("SolidMechanicsApp", ElasticModel);

template <>
InputParameters
validParams<ElasticModel>()
{
  InputParameters params = validParams<ConstitutiveModel>();

  // These parameters are a hack to get around the fact that SolidModel::createConstitutiveModel
  // function is used to create ElasticModel objects by injecting other parameters into it
  // (which it doesn't understand).  This is an ugly hack to get around this behavior which MOOSE
  // will no longer allow.
  params.addCoupledVar("X_Pu", 0, "Coupled plutonium atom fraction");
  params.addCoupledVar("X_Zr", 0, "Coupled zirconium atom fraction");
  params.addParam<bool>("calc_youngs", true, "Flag to calculate Youngs Modulus or use given value");
  params.addParam<bool>(
      "calc_poissons", true, "Flag to calculate Poissons ratio or use given value");
  params.addParam<bool>("model_creep", true, "Flag for creep model");
  params.addParam<std::vector<std::string>>("volumetric_models", "Volumetric models to apply");
  params.addParam<Real>("A_U", 0.2380289, "Atomic weight of uranium [kg/mol]");
  params.addParam<Real>("A_Pu", 0.244, "Atomic weight of plutonium [kg/mol]");
  params.addParam<Real>("A_Zr", 0.091224, "Atomic weight of zirconium [kg/mol]");
  params.addParam<bool>("absolute_tolerance", "dummy");
  params.addParam<bool>("acceptable_multiplier", "dummy");
  params.addParam<bool>("compute_material_timestep_limit", "dummy");
  params.addParam<bool>("fission_rate", "dummy");
  params.addParam<bool>("fission_rate_material", "dummy");
  params.addParam<bool>("gamma_transition", "dummy");
  params.addParam<bool>("hydrostatic_stress", "dummy");
  params.addParam<bool>("max_inelastic_increment", "dummy");
  params.addParam<bool>("max_its", "dummy");
  params.addParam<bool>("open_pore_compressibility_factor", "dummy");
  params.addParam<MooseEnum>("internal_solve_output_on", MooseEnum("dummy"), "dummy");
  params.addParam<bool>("internal_solve_full_iteration_history", "dummy");
  params.addParam<bool>("plenum_pressure", "dummy");
  params.addParam<bool>("porosity", "dummy");
  params.addParam<bool>("relative_tolerance", "dummy");
  params.addParam<bool>("use_material_fission_rate", "dummy");

  return params;
}

ElasticModel::ElasticModel(const InputParameters & parameters) : ConstitutiveModel(parameters) {}

////////////////////////////////////////////////////////////////////////

ElasticModel::~ElasticModel() {}

////////////////////////////////////////////////////////////////////////

void
ElasticModel::computeStress(const Elem & /*current_elem*/,
                            const SymmElasticityTensor & elasticity_tensor,
                            const SymmTensor & stress_old,
                            SymmTensor & strain_increment,
                            SymmTensor & stress_new)
{
  stress_new = elasticity_tensor * strain_increment;
  stress_new += stress_old;
}
