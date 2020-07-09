//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsActionBase.h"
#include "CommonTensorMechanicsAction.h"
#include "ActionWarehouse.h"
#include "ComputeFiniteStrain.h"

// map tensor name shortcuts to tensor material property names
const std::map<std::string, std::string>
    TensorMechanicsActionBase::_rank_two_cartesian_component_table = {
        {"strain", "total_strain"},
        {"stress", "stress"},
        {"elastic_strain", "elastic_strain"},
        {"plastic_strain", "plastic_strain"},
        {"creep_strain", "creep_strain"},
        {"creep_stress", "creep_stress"}};
const std::vector<char> TensorMechanicsActionBase::_component_table = {'x', 'y', 'z'};

// map aux variable name prefixes to RankTwoInvariant option and list of permitted tensor name
// shortcuts
const std::map<std::string, std::pair<std::string, std::vector<std::string>>>
    TensorMechanicsActionBase::_rank_two_invariant_table = {
        {"vonmises", {"VonMisesStress", {"stress"}}},
        {"effective", {"EffectiveStrain", {"plastic_strain", "creep_strain"}}},
        {"hydrostatic", {"Hydrostatic", {"stress"}}},
        {"l2norm",
         {"L2norm", {"stress", "strain", "elastic_strain", "plastic_strain", "creep_strain"}}},
        {"volumetric", {"VolumetricStrain", {"strain"}}},
        {"firstinv", {"FirstInvariant", {"stress", "strain"}}},
        {"secondinv", {"SecondInvariant", {"stress", "strain"}}},
        {"thirdinv", {"ThirdInvariant", {"stress", "strain"}}},
        {"triaxiality", {"TriaxialityStress", {"stress"}}},
        {"maxshear", {"MaxShear", {"stress"}}},
        {"intensity", {"StressIntensity", {"stress"}}},
        {"max_principal", {"MaxPrincipal", {"stress", "strain"}}},
        {"mid_principal", {"MidPrincipal", {"stress", "strain"}}},
        {"min_principal", {"MinPrincipal", {"stress", "strain"}}}};

const std::map<std::string, std::pair<std::string, std::vector<std::string>>>
    TensorMechanicsActionBase::_rank_two_directional_component_table = {
        {"directional", {"Direction", {"stress", "strain"}}}};

const std::map<std::string, std::pair<std::string, std::vector<std::string>>>
    TensorMechanicsActionBase::_rank_two_cylindrical_component_table = {
        {"axial",
         {"AxialStress", {"stress", "strain", "plastic_strain", "creep_strain", "elastic_strain"}}},
        {"hoop",
         {"HoopStress", {"stress", "strain", "plastic_strain", "creep_strain", "elastic_strain"}}},
        {"radial", {"RadialStress", {"stress", "strain"}}}};

InputParameters
TensorMechanicsActionBase::validParams()
{
  InputParameters params = Action::validParams();

  params.addRequiredParam<std::vector<VariableName>>(
      "displacements", "The nonlinear displacement variables for the problem");
  params.addParam<std::vector<VariableName>>("temperature", "The temperature");

  MooseEnum strainType("SMALL FINITE", "SMALL");
  params.addParam<MooseEnum>("strain", strainType, "Strain formulation");
  params.addParam<bool>("incremental", "Use incremental or total strain");

  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<bool>(
      "volumetric_locking_correction", false, "Flag to correct volumetric locking");
  params.addParam<bool>(
      "use_finite_deform_jacobian", false, "Jacobian for corrotational finite strain");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParam<bool>("add_variables", false, "Add the displacement variables");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of eigenstrains to be applied in this strain calculation");
  params.addParam<bool>("use_automatic_differentiation",
                        false,
                        "Flag to use automatic differentiation (AD) objects when possible");
  // Global Strain
  params.addParam<MaterialPropertyName>(
      "global_strain",
      "Name of the global strain material to be applied in this strain calculation. "
      "The global strain tensor is constant over the whole domain and allows visualization "
      "of the deformed shape with the periodic BC");

  // Advanced
  params.addParam<std::vector<AuxVariableName>>("save_in", "The displacement residuals");
  params.addParam<std::vector<AuxVariableName>>("diag_save_in",
                                                "The displacement diagonal preconditioner terms");
  params.addParam<MooseEnum>("decomposition_method",
                             ComputeFiniteStrain::decompositionType(),
                             "Methods to calculate the finite strain and rotation increments");
  params.addParamNamesToGroup("save_in diag_save_in", "Advanced");

  // Planar Formulation
  MooseEnum planarFormulationType("NONE WEAK_PLANE_STRESS PLANE_STRAIN GENERALIZED_PLANE_STRAIN",
                                  "NONE");
  params.addParam<MooseEnum>(
      "planar_formulation", planarFormulationType, "Out-of-plane stress/strain formulation");
  params.addParam<VariableName>("scalar_out_of_plane_strain",
                                "Scalar variable for the out-of-plane strain (in y "
                                "direction for 1D Axisymmetric or in z direction for 2D "
                                "Cartesian problems)");
  params.addParam<VariableName>("out_of_plane_strain",
                                "Variable for the out-of-plane strain for plane stress models");
  MooseEnum outOfPlaneDirection("x y z", "z");
  params.addParam<MooseEnum>(
      "out_of_plane_direction", outOfPlaneDirection, "The direction of the out-of-plane strain.");
  params.addParam<FunctionName>("out_of_plane_pressure",
                                "0",
                                "Function used to prescribe pressure in the out-of-plane direction "
                                "(y for 1D Axisymmetric or z for 2D Cartesian problems)");
  params.addParam<Real>("pressure_factor", 1.0, "Scale factor applied to prescribed pressure");
  params.addParamNamesToGroup("planar_formulation scalar_out_of_plane_strain out_of_plane_pressure "
                              "pressure_factor out_of_plane_direction out_of_plane_strain",
                              "Out-of-plane stress/strain");

  // Output
  params.addParam<MultiMooseEnum>("generate_output",
                                  TensorMechanicsActionBase::outputPropertiesType(),
                                  "Add scalar quantity output for stress and/or strain");
  params.addParamNamesToGroup("generate_output", "Output");

  return params;
}

TensorMechanicsActionBase::TensorMechanicsActionBase(const InputParameters & parameters)
  : Action(parameters), _use_ad(getParam<bool>("use_automatic_differentiation"))
{
  // check if a container block with common parameters is found
  auto action = _awh.getActions<CommonTensorMechanicsAction>();
  if (action.size() == 1)
    _pars.applyParameters(action[0]->parameters());

  // append additional_generate_output
  if (isParamValid("additional_generate_output"))
  {
    MultiMooseEnum generate_output = getParam<MultiMooseEnum>("generate_output");
    MultiMooseEnum additional_generate_output =
        getParam<MultiMooseEnum>("additional_generate_output");
    for (auto & output : additional_generate_output)
      generate_output.push_back(output);

    _pars.set<MultiMooseEnum>("generate_output") = generate_output;
  }
}

MultiMooseEnum
TensorMechanicsActionBase::outputPropertiesType()
{
  std::string options = "";
  for (auto & r2tc : _rank_two_cartesian_component_table)
    for (unsigned int a = 0; a < 3; ++a)
      for (unsigned int b = 0; b < 3; ++b)
        options += (options == "" ? "" : " ") + r2tc.first + '_' + _component_table[a] +
                   _component_table[b];

  for (auto & r2i : _rank_two_invariant_table)
    for (auto & t : r2i.second.second)
      options += " " + r2i.first + "_" + t;

  for (auto & r2sdc : _rank_two_directional_component_table)
    for (auto & r : r2sdc.second.second)
      options += " " + r2sdc.first + "_" + r;

  for (auto & r2cc : _rank_two_cylindrical_component_table)
    for (auto & r : r2cc.second.second)
      options += " " + r2cc.first + "_" + r;

  return MultiMooseEnum(options);
}
