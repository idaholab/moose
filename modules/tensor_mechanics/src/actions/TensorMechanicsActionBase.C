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
#include "AddAuxVariableAction.h"
#include "ComputeFiniteStrain.h"
#include "MooseApp.h"
#include "InputParameterWarehouse.h"

// map tensor name shortcuts to tensor material property names
std::map<std::string, std::string> TensorMechanicsActionBase::_rank_two_cartesian_component_table =
    {{"strain", "total_strain"},
     {"mechanical_strain", "mechanical_strain"},
     {"stress", "stress"},
     {"cauchy_stress", "cauchy_stress"},
     {"deformation_gradient", "deformation_gradient"},
     {"pk1_stress", "pk1_stress"},
     {"pk2_stress", "pk2_stress"},
     {"small_stress", "small_stress"},
     {"elastic_strain", "elastic_strain"},
     {"plastic_strain", "plastic_strain"},
     {"creep_strain", "creep_strain"},
     {"creep_stress", "creep_stress"}};
const std::vector<char> TensorMechanicsActionBase::_component_table = {'x', 'y', 'z'};

// map aux variable name prefixes to RankTwoInvariant option and list of permitted tensor name
// shortcuts
const std::map<std::string, std::pair<std::string, std::vector<std::string>>>
    TensorMechanicsActionBase::_rank_two_invariant_table = {
        {"vonmises", {"VonMisesStress", {"stress", "cauchy_stress", "pk1_stress", "pk2_stress"}}},
        {"effective", {"EffectiveStrain", {"plastic_strain", "creep_strain"}}},
        {"hydrostatic",
         {"Hydrostatic", {"stress", "cauchy_stress", "pk1_stress", "pk2_stress", "small_stress"}}},
        {"l2norm",
         {"L2norm",
          {"mechanical_strain",
           "stress",
           "cauchy_stress",
           "pk1_stress",
           "strain",
           "elastic_strain",
           "plastic_strain",
           "creep_strain"}}},
        {"volumetric", {"VolumetricStrain", {"mechanical_strain", "strain"}}},
        {"firstinv",
         {"FirstInvariant",
          {"stress", "cauchy_stress", "pk1_stress", "pk2_stress", "small_stress", "strain"}}},
        {"secondinv",
         {"SecondInvariant",
          {"stress", "cauchy_stress", "pk1_stress", "pk2_stress", "small_stress", "strain"}}},
        {"thirdinv",
         {"ThirdInvariant",
          {"stress", "cauchy_stress", "pk1_stress", "pk2_stress", "small_stress", "strain"}}},
        {"triaxiality",
         {"TriaxialityStress",
          {
              "stress",
              "cauchy_stress",
              "pk1_stress",
              "pk2_stress",
              "small_stress",
          }}},
        {"maxshear",
         {"MaxShear",
          {
              "stress",
              "cauchy_stress",
              "pk1_stress",
              "pk2_stress",
              "small_stress",
          }}},
        {"intensity",
         {"StressIntensity",
          {
              "stress",
              "cauchy_stress",
              "pk1_stress",
              "pk2_stress",
              "small_stress",
          }}},
        {"max_principal",
         {"MaxPrincipal",
          {"mechanical_strain",
           "stress",
           "cauchy_stress",
           "pk1_stress",
           "pk2_stress",
           "small_stress",
           "strain"}}},
        {"mid_principal",
         {"MidPrincipal",
          {"mechanical_strain",
           "stress",
           "cauchy_stress",
           "pk1_stress",
           "pk2_stress",
           "small_stress",
           "strain"}}},
        {"min_principal",
         {"MinPrincipal",
          {"mechanical_strain",
           "stress",
           "cauchy_stress",
           "pk1_stress",
           "pk2_stress",
           "small_stress",
           "strain"}}}};

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

const std::map<std::string, std::pair<std::string, std::vector<std::string>>>
    TensorMechanicsActionBase::_rank_two_spherical_component_table = {
        {"spherical_hoop",
         {"HoopStress", {"stress", "strain", "plastic_strain", "creep_strain", "elastic_strain"}}},
        {"spherical_radial", {"RadialStress", {"stress", "strain"}}}};

InputParameters
TensorMechanicsActionBase::validParams()
{
  InputParameters params = Action::validParams();

  params.addRequiredParam<std::vector<VariableName>>(
      "displacements", "The nonlinear displacement variables for the problem");
  params.addParam<std::vector<VariableName>>("temperature", "The temperature");

  MooseEnum strainType("SMALL FINITE", "SMALL");
  params.addParam<MooseEnum>("strain", strainType, "Strain formulation");
  params.addParam<bool>("incremental",
                        "Use incremental or total strain (if not explicitly specified this "
                        "defaults to incremental for finite strain and total for small strain)");

  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<bool>(
      "volumetric_locking_correction", false, "Flag to correct volumetric locking");
  params.addParam<bool>(
      "use_finite_deform_jacobian", false, "Jacobian for corrotational finite strain");
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
  params.addDeprecatedParam<FunctionName>(
      "out_of_plane_pressure",
      "Function used to prescribe pressure (applied toward the body) in the out-of-plane direction "
      "(y for 1D Axisymmetric or z for 2D Cartesian problems)",
      "This has been replaced by 'out_of_plane_pressure_function'");
  params.addParam<FunctionName>(
      "out_of_plane_pressure_function",
      "Function used to prescribe pressure (applied toward the body) in the out-of-plane direction "
      "(y for 1D Axisymmetric or z for 2D Cartesian problems)");
  params.addParam<Real>(
      "pressure_factor",
      "Scale factor applied to prescribed out-of-plane pressure (both material and function)");
  params.addParam<MaterialPropertyName>("out_of_plane_pressure_material",
                                        "0",
                                        "Material used to prescribe pressure (applied toward the "
                                        "body) in the out-of-plane direction");
  params.addParamNamesToGroup("planar_formulation scalar_out_of_plane_strain out_of_plane_pressure "
                              "out_of_plane_pressure_material out_of_plane_pressure_function "
                              "pressure_factor out_of_plane_direction out_of_plane_strain",
                              "Out-of-plane stress/strain");

  // Output
  params.addParam<MultiMooseEnum>("generate_output",
                                  TensorMechanicsActionBase::outputPropertiesType(),
                                  "Add scalar quantity output for stress and/or strain");

  params.addParam<MultiMooseEnum>(
      "material_output_order",
      TensorMechanicsActionBase::materialOutputOrders(),
      "Specifies the order of the FE shape function to use for this variable.");

  params.addParam<MultiMooseEnum>(
      "material_output_family",
      TensorMechanicsActionBase::materialOutputFamilies(),
      "Specifies the family of FE shape functions to use for this variable.");
  params.addParamNamesToGroup("generate_output material_output_order material_output_family",
                              "Output");
  params.addParam<bool>("verbose", false, "Display extra information.");

  params.addParam<bool>("new_system",
                        false,
                        "If true use the new "
                        "LagrangianStressDiverence kernels.");

  MooseEnum formulationType("TOTAL UPDATED", "TOTAL");
  params.addParam<MooseEnum>("formulation",
                             formulationType,
                             "Select between the total Lagrangian (TOTAL) "
                             "and updated Lagrangian (UPDATED) formulations "
                             "for the new kernel system.");

  return params;
}

TensorMechanicsActionBase::TensorMechanicsActionBase(const InputParameters & parameters)
  : Action(parameters), _use_ad(getParam<bool>("use_automatic_differentiation"))
{
  const auto & params = _app.getInputParameterWarehouse().getInputParameters();
  InputParameters & pars(*(params.find(uniqueActionName())->second.get()));

  // check if a container block with common parameters is found
  auto action = _awh.getActions<CommonTensorMechanicsAction>();
  if (action.size() == 1)
    pars.applyParameters(action[0]->parameters());

  // append additional_generate_output
  if (isParamValid("additional_generate_output"))
  {
    MultiMooseEnum generate_output = getParam<MultiMooseEnum>("generate_output");
    MultiMooseEnum additional_generate_output =
        getParam<MultiMooseEnum>("additional_generate_output");

    MultiMooseEnum material_output_order = getParam<MultiMooseEnum>("material_output_order");
    MultiMooseEnum additional_material_output_order =
        getParam<MultiMooseEnum>("additional_material_output_order");

    MultiMooseEnum material_output_family = getParam<MultiMooseEnum>("material_output_family");
    MultiMooseEnum additional_material_output_family =
        getParam<MultiMooseEnum>("additional_material_output_family");

    for (auto & output : additional_generate_output)
      generate_output.push_back(output);
    for (auto & order : additional_material_output_order)
      material_output_order.push_back(order);
    for (auto & family : additional_material_output_family)
      material_output_family.push_back(family);

    pars.set<MultiMooseEnum>("generate_output") = generate_output;
    pars.set<MultiMooseEnum>("material_output_order") = material_output_order;
    pars.set<MultiMooseEnum>("material_output_family") = material_output_family;
  }
}

MultiMooseEnum
TensorMechanicsActionBase::materialOutputOrders()
{
  auto orders = AddAuxVariableAction::getAuxVariableOrders().getRawNames();

  return MultiMooseEnum(orders);
}

MultiMooseEnum
TensorMechanicsActionBase::materialOutputFamilies()
{
  return MultiMooseEnum("MONOMIAL LAGRANGE");
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

  for (auto & r2sc : _rank_two_spherical_component_table)
    for (auto & r : r2sc.second.second)
      options += " " + r2sc.first + "_" + r;

  return MultiMooseEnum(options, "", true);
}

void
TensorMechanicsActionBase::addCartesianComponentOutput(const std::string & enum_name,
                                                       const std::string & prop_name)
{
  if (prop_name.empty())
    // the enum name is the actual tensor material property name
    _rank_two_cartesian_component_table.emplace(enum_name, enum_name);
  else
    // supply a different name for the enum options (this is done for
    // 'strain' -> 'mechanical_strain' in the TMA)
    _rank_two_cartesian_component_table.emplace(enum_name, prop_name);
}
