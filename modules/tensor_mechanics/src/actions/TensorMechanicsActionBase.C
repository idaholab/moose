/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsActionBase.h"
#include "CommonTensorMechanicsAction.h"
#include "ActionWarehouse.h"
#include "ComputeFiniteStrain.h"

// map tensor name shortcuts to tensor material property names
const std::map<std::string, std::string> TensorMechanicsActionBase::_ranktwoaux_table = {
    {"strain", "total_strain"},
    {"stress", "stress"},
    {"elastic_strain", "elastic_strain"},
    {"plastic_strain", "plastic_strain"},
    {"creep_strain", "creep_strain"}};
const std::vector<char> TensorMechanicsActionBase::_component_table = {'x', 'y', 'z'};
// map aux variable name prefixes to RanTwoScalarAux option and list of permitted tensor name
// shortcuts
const std::map<std::string, std::pair<std::string, std::vector<std::string>>>
    TensorMechanicsActionBase::_ranktwoscalaraux_table = {
        {"vonmises", {"VonMisesStress", {"stress"}}},
        {"hydrostatic", {"Hydrostatic", {"stress"}}},
        {"max_principal", {"MaxPrincipal", {"stress"}}},
        {"mid_principal", {"MidPrincipal", {"stress"}}},
        {"min_principal", {"MinPrincipal", {"stress"}}},
        {"equivalent", {"EquivalentPlasticStrain", {"plastic_strain", "creep_strain"}}},
        {"firstinv", {"FirstInvariant", {"stress", "strain"}}},
        {"secondinv", {"SecondInvariant", {"stress", "strain"}}},
        {"thirdinv", {"ThirdInvariant", {"stress", "strain"}}}};

template <>
InputParameters
validParams<TensorMechanicsActionBase>()
{
  InputParameters params = validParams<Action>();

  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements", "The nonlinear displacement variables for the problem");
  params.addParam<NonlinearVariableName>("temp", "The temperature"); // Deprecated
  params.addParam<NonlinearVariableName>("temperature", "The temperature");

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

  // Advanced
  params.addParam<std::vector<AuxVariableName>>("save_in", "The displacement residuals");
  params.addParam<std::vector<AuxVariableName>>("diag_save_in",
                                                "The displacement diagonal preconditioner terms");
  params.addParam<MooseEnum>("decomposition_method",
                             ComputeFiniteStrain::decompositionType(),
                             "Methods to calculate the finite strain and rotation increments");
  params.addParamNamesToGroup("save_in diag_save_in", "Advanced");

  // Planar Formulation
  MooseEnum planarFormulationType("NONE PLANE_STRAIN GENERALIZED_PLANE_STRAIN",
                                  "NONE"); // PLANE_STRESS
  params.addParam<MooseEnum>(
      "planar_formulation", planarFormulationType, "Out-of-plane stress/strain formulation");
  params.addParam<NonlinearVariableName>("scalar_out_of_plane_strain",
                                         "Scalar variable for the out-of-plane strain (in y "
                                         "direction for 1D Axisymmetric or in z direction for 2D "
                                         "Cartesian problems)");
  params.addParam<FunctionName>("out_of_plane_pressure",
                                "0",
                                "Function used to prescribe pressure in the out-of-plane direction "
                                "(y for 1D Axisymmetric or z for 2D Cartesian problems)");
  params.addParam<Real>("pressure_factor", 1.0, "Scale factor applied to prescribed pressure");
  params.addParamNamesToGroup(
      "planar_formulation scalar_out_of_plane_strain out_of_plane_pressure pressure_factor",
      "Out-of-plane stress/strain");

  // Output
  params.addParam<MultiMooseEnum>("generate_output",
                                  TensorMechanicsActionBase::outputPropertiesType(),
                                  "Add scalar quantity output for stress and/or strain");
  params.addParamNamesToGroup("generate_output", "Output");

  return params;
}

TensorMechanicsActionBase::TensorMechanicsActionBase(const InputParameters & parameters)
  : Action(parameters)
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
  for (auto & r2a : _ranktwoaux_table)
    for (unsigned int a = 0; a < 3; ++a)
      for (unsigned int b = 0; b < 3; ++b)
        options += (options == "" ? "" : " ") + r2a.first + '_' + _component_table[a] +
                   _component_table[b];

  for (auto & r2sa : _ranktwoscalaraux_table)
    for (auto & t : r2sa.second.second)
      options += " " + r2sa.first + "_" + t;

  return MultiMooseEnum(options);
}
