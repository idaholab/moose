//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Conversion.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseMesh.h"
#include "MooseObjectAction.h"
#include "TensorMechanicsAction.h"
#include "Material.h"

#include "BlockRestrictable.h"

#include "HomogenizationConstraint.h"
#include "AddVariableAction.h"

#include "libmesh/string_to_enum.h"
#include <algorithm>

registerMooseAction("TensorMechanicsApp", TensorMechanicsAction, "meta_action");

registerMooseAction("TensorMechanicsApp", TensorMechanicsAction, "setup_mesh_complete");

registerMooseAction("TensorMechanicsApp", TensorMechanicsAction, "validate_coordinate_systems");

registerMooseAction("TensorMechanicsApp", TensorMechanicsAction, "add_variable");

registerMooseAction("TensorMechanicsApp", TensorMechanicsAction, "add_aux_variable");

registerMooseAction("TensorMechanicsApp", TensorMechanicsAction, "add_kernel");

registerMooseAction("TensorMechanicsApp", TensorMechanicsAction, "add_aux_kernel");

registerMooseAction("TensorMechanicsApp", TensorMechanicsAction, "add_material");

registerMooseAction("TensorMechanicsApp", TensorMechanicsAction, "add_master_action_material");

registerMooseAction("TensorMechanicsApp", TensorMechanicsAction, "add_scalar_kernel");

registerMooseAction("TensorMechanicsApp", TensorMechanicsAction, "add_user_object");

InputParameters
TensorMechanicsAction::validParams()
{
  InputParameters params = TensorMechanicsActionBase::validParams();
  params.addClassDescription("Set up stress divergence kernels with coordinate system aware logic");

  // parameters specified here only appear in the input file sub-blocks of the
  // Master action, not in the common parameters area
  params.addParam<std::vector<SubdomainName>>("block",
                                              "The list of ids of the blocks (subdomain) "
                                              "that the stress divergence kernels will be "
                                              "applied to");
  params.addParamNamesToGroup("block", "Advanced");

  params.addParam<MultiMooseEnum>("additional_generate_output",
                                  TensorMechanicsActionBase::outputPropertiesType(),
                                  "Add scalar quantity output for stress and/or strain (will be "
                                  "appended to the list in `generate_output`)");
  params.addParam<MultiMooseEnum>(
      "additional_material_output_order",
      TensorMechanicsActionBase::materialOutputOrders(),
      "Specifies the order of the FE shape function to use for this variable.");

  params.addParam<MultiMooseEnum>(
      "additional_material_output_family",
      TensorMechanicsActionBase::materialOutputFamilies(),
      "Specifies the family of FE shape functions to use for this variable.");

  params.addParamNamesToGroup("additional_generate_output additional_material_output_order "
                              "additional_material_output_family",
                              "Output");
  params.addParam<std::string>(
      "strain_base_name",
      "The base name used for the strain. If not provided, it will be set equal to base_name");
  params.addParam<std::vector<TagName>>(
      "extra_vector_tags",
      "The tag names for extra vectors that residual data should be saved into");
  params.addParam<std::vector<TagName>>("absolute_value_vector_tags",
                                        "The tag names for extra vectors that the absolute value "
                                        "of the residual should be accumulated into");
  params.addParam<Real>("scaling", "The scaling to apply to the displacement variables");
  params.addParam<Point>(
      "cylindrical_axis_point1",
      "Starting point for direction of axis of rotation for cylindrical stress/strain.");
  params.addParam<Point>(
      "cylindrical_axis_point2",
      "Ending point for direction of axis of rotation for cylindrical stress/strain.");
  params.addParam<Point>("spherical_center_point",
                         "Center point of the spherical coordinate system.");
  params.addParam<Point>("direction", "Direction stress/strain is calculated in");
  params.addParam<bool>("automatic_eigenstrain_names",
                        false,
                        "Collects all material eigenstrains and passes to required strain "
                        "calculator within TMA internally.");

  // Homogenization system input
  MultiMooseEnum constraintType("strain stress");
  params.addParam<MultiMooseEnum>("constraint_types",
                                  Homogenization::constraintType,
                                  "Type of each constraint: "
                                  "stress or strain.");
  params.addParam<std::vector<FunctionName>>("targets",
                                             "Functions giving the target "
                                             "values of each constraint.");

  return params;
}

TensorMechanicsAction::TensorMechanicsAction(const InputParameters & params)
  : TensorMechanicsActionBase(params),
    _displacements(getParam<std::vector<VariableName>>("displacements")),
    _ndisp(_displacements.size()),
    _coupled_displacements(_ndisp),
    _save_in(getParam<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in(getParam<std::vector<AuxVariableName>>("diag_save_in")),
    _subdomain_names(getParam<std::vector<SubdomainName>>("block")),
    _subdomain_ids(),
    _strain(getParam<MooseEnum>("strain").getEnum<Strain>()),
    _planar_formulation(getParam<MooseEnum>("planar_formulation").getEnum<PlanarFormulation>()),
    _out_of_plane_direction(
        getParam<MooseEnum>("out_of_plane_direction").getEnum<OutOfPlaneDirection>()),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _material_output_order(getParam<MultiMooseEnum>("material_output_order")),
    _material_output_family(getParam<MultiMooseEnum>("material_output_family")),
    _cylindrical_axis_point1_valid(params.isParamSetByUser("cylindrical_axis_point1")),
    _cylindrical_axis_point2_valid(params.isParamSetByUser("cylindrical_axis_point2")),
    _direction_valid(params.isParamSetByUser("direction")),
    _verbose(getParam<bool>("verbose")),
    _spherical_center_point_valid(params.isParamSetByUser("spherical_center_point")),
    _auto_eigenstrain(getParam<bool>("automatic_eigenstrain_names")),
    _lagrangian_kernels(getParam<bool>("new_system")),
    _lk_large_kinematics(_strain == Strain::Finite),
    _lk_formulation(getParam<MooseEnum>("formulation").getEnum<LKFormulation>()),
    _lk_locking(getParam<bool>("volumetric_locking_correction")),
    _lk_homogenization(false),
    _constraint_types(getParam<MultiMooseEnum>("constraint_types")),
    _targets(getParam<std::vector<FunctionName>>("targets"))
{
  // determine if incremental strains are to be used
  if (isParamValid("incremental"))
  {
    const bool incremental = getParam<bool>("incremental");
    if (!incremental && _strain == Strain::Small)
      _strain_and_increment = StrainAndIncrement::SmallTotal;
    else if (!incremental && _strain == Strain::Finite)
      _strain_and_increment = StrainAndIncrement::FiniteTotal;
    else if (incremental && _strain == Strain::Small)
      _strain_and_increment = StrainAndIncrement::SmallIncremental;
    else if (incremental && _strain == Strain::Finite)
      _strain_and_increment = StrainAndIncrement::FiniteIncremental;
    else
      mooseError("Internal error");
  }
  else
  {
    if (_strain == Strain::Small)
    {
      _strain_and_increment = StrainAndIncrement::SmallTotal;
      mooseInfo("TensorMechanics Action: selecting 'total small strain' formulation. Use "
                "`incremental = true` to select 'incremental small strain' instead.");
    }
    else if (_strain == Strain::Finite)
    {
      _strain_and_increment = StrainAndIncrement::FiniteIncremental;
      mooseInfo("TensorMechanics Action: selecting 'incremental finite strain' formulation.");
    }
    else
      mooseError("Internal error");
  }

  // determine if displaced mesh is to be used
  _use_displaced_mesh = (_strain == Strain::Finite);
  if (params.isParamSetByUser("use_displaced_mesh") && !_lagrangian_kernels)
  {
    bool use_displaced_mesh_param = getParam<bool>("use_displaced_mesh");
    if (use_displaced_mesh_param != _use_displaced_mesh && params.isParamSetByUser("strain"))
      mooseError("Wrong combination of use displaced mesh and strain model");
    _use_displaced_mesh = use_displaced_mesh_param;
  }

  // convert vector of VariableName to vector of VariableName
  for (unsigned int i = 0; i < _ndisp; ++i)
    _coupled_displacements[i] = _displacements[i];

  if (_save_in.size() != 0 && _save_in.size() != _ndisp)
    mooseError("Number of save_in variables should equal to the number of displacement variables ",
               _ndisp);

  if (_diag_save_in.size() != 0 && _diag_save_in.size() != _ndisp)
    mooseError(
        "Number of diag_save_in variables should equal to the number of displacement variables ",
        _ndisp);

  // plane strain consistency check
  if (_planar_formulation != PlanarFormulation::None)
  {
    if (params.isParamSetByUser("out_of_plane_strain") &&
        _planar_formulation != PlanarFormulation::WeakPlaneStress)
      mooseError(
          "out_of_plane_strain should only be specified with planar_formulation=WEAK_PLANE_STRESS");
    else if (!params.isParamSetByUser("out_of_plane_strain") &&
             _planar_formulation == PlanarFormulation::WeakPlaneStress)
      mooseError("out_of_plane_strain must be specified with planar_formulation=WEAK_PLANE_STRESS");
  }

  // convert output variable names to lower case
  for (const auto & out : getParam<MultiMooseEnum>("generate_output"))
  {
    std::string lower(out);
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    _generate_output.push_back(lower);
  }

  if (!_generate_output.empty())
    verifyOrderAndFamilyOutputs();

  // Error if volumetric locking correction is true for 1D problems
  if (_ndisp == 1 && getParam<bool>("volumetric_locking_correction"))
    mooseError("Volumetric locking correction should be set to false for 1D problems.");

  if (!getParam<bool>("add_variables") && params.isParamSetByUser("scaling"))
    paramError("scaling",
               "The scaling parameter has no effect unless add_variables is set to true. Did you "
               "mean to set 'add_variables = true'?");

  // Get cylindrical axis points if set by user
  if (_cylindrical_axis_point1_valid && _cylindrical_axis_point2_valid)
  {
    _cylindrical_axis_point1 = getParam<Point>("cylindrical_axis_point1");
    _cylindrical_axis_point2 = getParam<Point>("cylindrical_axis_point2");
  }

  // Get spherical center point if set by user
  if (_spherical_center_point_valid)
    _spherical_center_point = getParam<Point>("spherical_center_point");

  // Get direction for tensor component if set by user
  if (_direction_valid)
    _direction = getParam<Point>("direction");

  // Get eigenstrain names if passed by user
  _eigenstrain_names = getParam<std::vector<MaterialPropertyName>>("eigenstrain_names");

  // Determine if we're going to use the homogenization system for the new
  // lagrangian kernels
  bool ctype_set = params.isParamSetByUser("constraint_types");
  bool targets_set = params.isParamSetByUser("targets");
  if (ctype_set || targets_set)
  {
    if (!(ctype_set && targets_set))
      mooseError("To use the Lagrangian kernel homogenization system you "
                 "most provide both the constraint_types and the targets "
                 "parameters");
    _lk_homogenization = true;
    // Do consistency checking on the kernel options
    if (_lk_homogenization && (_lk_formulation == LKFormulation::Updated))
      mooseError("The Lagrangian kernel homogenization system requires the "
                 "use of formulation = TOTAL");
  }

  // Cross check options to weed out incompatible choices for the new lagrangian
  // kernel system
  if (_lagrangian_kernels)
  {
    if (_use_ad)
      mooseError("The Lagrangian kernel system is not yet compatible with AD. "
                 "Do not set the use_automatic_differentiation flag.");

    if (params.isParamSetByUser("use_finite_deform_jacobian"))
      mooseError("The Lagrangian kernel system always produces the exact "
                 "Jacobian.  use_finite_deform_jacobian is redundant and "
                 " should not be set");
    if (params.isParamSetByUser("global_strain"))
      mooseError("The Lagrangian kernel system is not compatible with "
                 "the global_strain option.  Use the homogenization "
                 " system instead");
    if (params.isParamSetByUser("decomposition_method"))
      mooseError("The decomposition_method parameter should not be used "
                 " with the Lagrangian kernel system.  Similar options "
                 " for native small deformation material models are "
                 " available as part of the ComputeLagrangianStress "
                 " material system.");
  }
  else
  {
    if (params.isParamSetByUser("formulation"))
      mooseError("The StressDiveregenceTensor system always uses an "
                 " updated Lagrangian formulation.  Do not set the "
                 " formulation parameter, it is only used with the "
                 " new Lagrangian kernel system.");
    if (_lk_homogenization)
      mooseError("The homogenization system can only be used with the "
                 "new Lagrangian kernels");
  }
}

void
TensorMechanicsAction::act()
{
  std::string ad_prepend = "";
  if (_use_ad)
    ad_prepend = "AD";

  // Consistency checks across subdomains
  actSubdomainChecks();

  // Gather info from all other TensorMechanicsAction
  actGatherActionParameters();

  // Deal with the optional AuxVariable based tensor quantity output
  actOutputGeneration();

  // Meta action which optionally spawns other actions
  if (_current_task == "meta_action")
  {
    if (_planar_formulation == PlanarFormulation::GeneralizedPlaneStrain)
    {
      if (_lagrangian_kernels)
        paramError("Plane formulation not available with Lagrangian kernels");

      if (_use_ad)
        paramError("use_automatic_differentiation", "AD not setup for use with PlaneStrain");
      // Set the action parameters
      const std::string type = "GeneralizedPlaneStrainAction";
      auto action_params = _action_factory.getValidParams(type);
      action_params.set<bool>("_built_by_moose") = true;
      action_params.set<std::string>("registered_identifier") = "(AutoBuilt)";

      // Skipping selected parameters in applyParameters() and then manually setting them only if
      // they are set by the user is just to prevent both the current and deprecated variants of
      // these parameters from both getting passed to the UserObject. Once we get rid of the
      // deprecated versions, we can just set them all with applyParameters().
      action_params.applyParameters(parameters(),
                                    {"use_displaced_mesh",
                                     "out_of_plane_pressure",
                                     "out_of_plane_pressure_function",
                                     "factor",
                                     "pressure_factor"});
      action_params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

      if (parameters().isParamSetByUser("out_of_plane_pressure"))
        action_params.set<FunctionName>("out_of_plane_pressure") =
            getParam<FunctionName>("out_of_plane_pressure");
      if (parameters().isParamSetByUser("out_of_plane_pressure_function"))
        action_params.set<FunctionName>("out_of_plane_pressure_function") =
            getParam<FunctionName>("out_of_plane_pressure_function");
      if (parameters().isParamSetByUser("factor"))
        action_params.set<Real>("factor") = getParam<Real>("factor");
      if (parameters().isParamSetByUser("pressure_factor"))
        action_params.set<Real>("pressure_factor") = getParam<Real>("pressure_factor");

      // Create and add the action to the warehouse
      auto action = MooseSharedNamespace::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(type, name() + "_gps", action_params));
      _awh.addActionBlock(action);
      if (isParamValid("extra_vector_tags"))
        action_params.set<std::vector<TagName>>("extra_vector_tags") =
            getParam<std::vector<TagName>>("extra_vector_tags");
      if (isParamValid("absolute_value_vector_tags"))
        action_params.set<std::vector<TagName>>("absolute_value_vector_tags") =
            getParam<std::vector<TagName>>("absolute_value_vector_tags");
    }
  }

  // Add variables
  else if (_current_task == "add_variable")
  {
    if (getParam<bool>("add_variables"))
    {
      auto params = _factory.getValidParams("MooseVariable");
      // determine necessary order
      const bool second = _problem->mesh().hasSecondOrderElements();

      params.set<MooseEnum>("order") = second ? "SECOND" : "FIRST";
      params.set<MooseEnum>("family") = "LAGRANGE";
      if (isParamValid("scaling"))
        params.set<std::vector<Real>>("scaling") = {getParam<Real>("scaling")};

      // Loop through the displacement variables
      for (const auto & disp : _displacements)
      {
        // Create displacement variables
        _problem->addVariable("MooseVariable", disp, params);
      }
    }

    // Homogenization scalar
    if (_lk_homogenization)
    {
      InputParameters params = _factory.getValidParams("MooseVariable");
      const std::map<bool, std::vector<unsigned int>> mg_order{{true, {1, 4, 9}},
                                                               {false, {1, 3, 6}}};
      params.set<MooseEnum>("family") = "SCALAR";
      params.set<MooseEnum>("order") = mg_order.at(_lk_large_kinematics)[_ndisp - 1];
      auto fe_type = AddVariableAction::feType(params);
      auto var_type = AddVariableAction::variableType(fe_type);
      _problem->addVariable(var_type, _hname, params);
    }
  }
  // Add Materials
  else if (_current_task == "add_master_action_material")
  {
    // Automatic eigenstrain names
    if (_auto_eigenstrain)
      actEigenstrainNames();

    // Easiest just to branch on type here, as the strain systems are completely
    // different
    if (_lagrangian_kernels)
      actLagrangianKernelStrain();
    else
      actStressDivergenceTensorsStrain();
  }

  // Add Stress Divergence (and optionally WeakPlaneStress) Kernels
  else if (_current_task == "add_kernel")
  {
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      auto tensor_kernel_type = getKernelType();
      auto params = getKernelParameters(ad_prepend + tensor_kernel_type);

      std::string kernel_name = "TM_" + name() + Moose::stringify(i);

      // Set appropriate components for kernels, including in the cases where a planar model is
      // running in planes other than the x-y plane (defined by _out_of_plane_strain_direction).
      if (_out_of_plane_direction == OutOfPlaneDirection::x && i == 0)
        continue;
      else if (_out_of_plane_direction == OutOfPlaneDirection::y && i == 1)
        continue;

      params.set<unsigned int>("component") = i;

      params.set<NonlinearVariableName>("variable") = _displacements[i];

      if (_save_in.size() == _ndisp)
        params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i]};
      if (_diag_save_in.size() == _ndisp)
        params.set<std::vector<AuxVariableName>>("diag_save_in") = {_diag_save_in[i]};
      if (isParamValid("out_of_plane_strain") && !_lagrangian_kernels)
        params.set<std::vector<VariableName>>("out_of_plane_strain") = {
            getParam<VariableName>("out_of_plane_strain")};

      if (_lk_homogenization)
      {
        params.set<std::vector<VariableName>>("macro_gradient") = {_hname};
        params.set<UserObjectName>("homogenization_constraint") = _integrator_name;
      }

      _problem->addKernel(ad_prepend + tensor_kernel_type, kernel_name, params);
    }

    if (_planar_formulation == PlanarFormulation::WeakPlaneStress)
    {
      auto params = getKernelParameters(ad_prepend + "WeakPlaneStress");
      std::string wps_kernel_name = "TM_WPS_" + name();
      params.set<NonlinearVariableName>("variable") = getParam<VariableName>("out_of_plane_strain");

      _problem->addKernel(ad_prepend + "WeakPlaneStress", wps_kernel_name, params);
    }
  }
  else if (_current_task == "add_scalar_kernel")
  {
    if (_lk_homogenization)
    {
      InputParameters params = _factory.getValidParams("HomogenizationConstraintScalarKernel");
      params.set<NonlinearVariableName>("variable") = _hname;
      params.set<UserObjectName>("homogenization_constraint") = _integrator_name;

      _problem->addScalarKernel(
          "HomogenizationConstraintScalarKernel", "HomogenizationConstraints", params);
    }
  }
  else if (_current_task == "add_user_object")
  {
    if (_lk_homogenization)
    {
      InputParameters params = _factory.getValidParams("HomogenizationConstraint");
      params.set<MultiMooseEnum>("constraint_types") = _constraint_types;
      params.set<std::vector<FunctionName>>("targets") = _targets;
      params.set<bool>("large_kinematics") = _lk_large_kinematics;
      params.set<std::vector<SubdomainName>>("block") = _subdomain_names;
      params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

      _problem->addUserObject("HomogenizationConstraint", _integrator_name, params);
    }
  }
}

void
TensorMechanicsAction::actSubdomainChecks()
{
  // Do the coordinate system check only once the problem is created
  if (_current_task == "setup_mesh_complete")
  {
    // get subdomain IDs
    for (auto & name : _subdomain_names)
      _subdomain_ids.insert(_mesh->getSubdomainID(name));
  }

  if (_current_task == "validate_coordinate_systems")
  {
    // use either block restriction list or list of all subdomains in the mesh
    const auto & check_subdomains =
        _subdomain_ids.empty() ? _problem->mesh().meshSubdomains() : _subdomain_ids;
    if (check_subdomains.empty())
      mooseError("No subdomains found");

    // make sure all subdomains are using the same coordinate system
    _coord_system = _problem->getCoordSystem(*check_subdomains.begin());
    for (auto subdomain : check_subdomains)
      if (_problem->getCoordSystem(subdomain) != _coord_system)
        mooseError("The TensorMechanics action requires all subdomains to have the same coordinate "
                   "system.");

    if (_coord_system == Moose::COORD_RZ)
    {
      if (_out_of_plane_direction != OutOfPlaneDirection::z)
        mooseError("'out_of_plane_direction' must be 'z' for axisymmetric simulations");
    }
    else if (_planar_formulation != PlanarFormulation::None)
    {
      if (_out_of_plane_direction == OutOfPlaneDirection::z && _ndisp != 2)
        mooseError(
            "Must specify two displacements for plane strain when the out of plane direction is z");
      else if (_out_of_plane_direction != OutOfPlaneDirection::z && _ndisp != 3)
        mooseError("Must specify three displacements for plane strain when the out of plane "
                   "direction is x or y");
    }
  }
}

void
TensorMechanicsAction::actOutputGeneration()
{
  if (_current_task == "add_material")
    actOutputMatProp();

  // Add variables (optional)
  if (_current_task == "add_aux_variable")
  {
    unsigned int index = 0;
    for (auto out : _generate_output)
    {
      const auto & order = _material_output_order[index];
      const auto & family = _material_output_family[index];

      std::string type = (order == "CONSTANT" && family == "MONOMIAL")
                             ? "MooseVariableConstMonomial"
                             : "MooseVariable";

      // Create output helper aux variables
      auto params = _factory.getValidParams(type);
      params.set<MooseEnum>("order") = order;
      params.set<MooseEnum>("family") = family;

      if (family == "MONOMIAL")
        _problem->addAuxVariable(type, _base_name + out, params);
      else
        _problem->addVariable(type, _base_name + out, params);

      index++;
    }
  }

  // Add output AuxKernels
  else if (_current_task == "add_aux_kernel")
  {
    std::string ad_prepend = _use_ad ? "AD" : "";
    // Loop through output aux variables
    unsigned int index = 0;
    for (auto out : _generate_output)
    {
      if (_material_output_family[index] == "MONOMIAL")
      {
        InputParameters params = emptyInputParameters();

        params = _factory.getValidParams("MaterialRealAux");
        params.applyParameters(parameters());
        params.set<MaterialPropertyName>("property") = _base_name + out;
        params.set<AuxVariableName>("variable") = _base_name + out;
        params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

        _problem->addAuxKernel(
            ad_prepend + "MaterialRealAux", _base_name + out + '_' + name(), params);
      }
      index++;
    }
  }
  else if (_current_task == "add_kernel")
  {
    std::string ad_prepend = _use_ad ? "AD" : "";
    // Loop through output aux variables
    unsigned int index = 0;
    for (auto out : _generate_output)
    {
      if (_material_output_family[index] != "MONOMIAL")
      {
        InputParameters params = emptyInputParameters();

        params = _factory.getValidParams("MaterialPropertyValue");
        params.applyParameters(parameters());
        params.set<MaterialPropertyName>("prop_name") = _base_name + out;
        params.set<NonlinearVariableName>("variable") = _base_name + out;

        _problem->addKernel(
            ad_prepend + "MaterialPropertyValue", _base_name + out + '_' + name(), params);
      }
      index++;
    }
  }
}

void
TensorMechanicsAction::actEigenstrainNames()
{
  // Create containers for collecting blockIDs and eigenstrain names from materials
  std::map<std::string, std::set<SubdomainID>> material_eigenstrain_map;
  std::set<std::string> eigenstrain_set;

  std::set<MaterialPropertyName> verified_eigenstrain_names;

  std::map<std::string, std::string> remove_add_map;
  std::set<std::string> remove_reduced_set;

  // Loop over all the materials(eigenstrains) already created
  auto materials = _problem->getMaterialWarehouse().getObjects();
  for (auto & mat : materials)
  {
    std::shared_ptr<BlockRestrictable> blk = std::dynamic_pointer_cast<BlockRestrictable>(mat);
    const InputParameters & mat_params = mat->parameters();
    auto & mat_name = mat->type();

    // Check for eigenstrain names, only deal with those materials
    if (mat_params.isParamValid("eigenstrain_name"))
    {
      std::shared_ptr<MaterialData> mat_dat;
      auto name = mat_params.get<std::string>("eigenstrain_name");

      // Check for base_name prefix
      if (mat_params.isParamValid("base_name"))
        name = mat_params.get<std::string>("base_name") + '_' + name;

      // Check block restrictions
      if (!blk)
        mooseError("Internal error, Material object that does not inherit form BlockRestricted");
      const std::set<SubdomainID> & blocks =
          blk->blockRestricted() ? blk->blockIDs() : blk->meshBlockIDs();

      if (std::includes(blocks.begin(), blocks.end(), _subdomain_ids.begin(), _subdomain_ids.end()))
      {
        material_eigenstrain_map[name].insert(blocks.begin(), blocks.end());
        eigenstrain_set.insert(name);
      }
    }

    // Account for reduced eigenstrains and CompositeEigenstrains
    if (mat_name == "ComputeReducedOrderEigenstrain")
    {
      auto input_eigenstrain_names =
          mat_params.get<std::vector<MaterialPropertyName>>("input_eigenstrain_names");
      remove_reduced_set.insert(input_eigenstrain_names.begin(), input_eigenstrain_names.end());
    }
    // Account for CompositeEigenstrains
    if (mat_name == "CompositeEigenstrain")
    {
      auto remove_list = mat_params.get<std::vector<MaterialPropertyName>>("tensors");
      for (auto i : remove_list)
        remove_reduced_set.insert(i);
    }

    // Account for MaterialADConverter , add or remove later
    if (mat_name == "RankTwoTensorMaterialADConverter")
    {
      std::vector<MaterialPropertyName> remove_list;
      std::vector<MaterialPropertyName> add_list;

      if (mat_params.isParamValid("ad_props_out") && mat_params.isParamValid("reg_props_in") &&
          _use_ad)
      {
        remove_list = mat_params.get<std::vector<MaterialPropertyName>>("reg_props_in");
        add_list = mat_params.get<std::vector<MaterialPropertyName>>("ad_props_out");
      }
      if (mat_params.isParamValid("ad_props_in") && mat_params.isParamValid("reg_props_out") &&
          !_use_ad)
      {
        remove_list = mat_params.get<std::vector<MaterialPropertyName>>("ad_props_in");
        add_list = mat_params.get<std::vector<MaterialPropertyName>>("reg_props_out");
      }

      // These vectors are the same size as checked in MaterialADConverter
      for (unsigned int index = 0; index < remove_list.size(); index++)
        remove_add_map.emplace(remove_list[index], add_list[index]);
    }
  }
  // All the materials have been accounted for, now remove or add parts

  // Remove names which aren't eigenstrains (converter properties)
  for (auto remove_add_index : remove_add_map)
  {
    const bool is_in = eigenstrain_set.find(remove_add_index.first) != eigenstrain_set.end();
    if (is_in)
    {
      eigenstrain_set.erase(remove_add_index.first);
      eigenstrain_set.insert(remove_add_index.second);
    }
  }
  for (auto index : remove_reduced_set)
    eigenstrain_set.erase(index);

  // Compare the blockIDs set of eigenstrain names with the vector of _eigenstrain_names for the
  // current subdomainID
  std::set_union(eigenstrain_set.begin(),
                 eigenstrain_set.end(),
                 _eigenstrain_names.begin(),
                 _eigenstrain_names.end(),
                 std::inserter(verified_eigenstrain_names, verified_eigenstrain_names.begin()));

  // Ensure the eigenstrain names previously passed include any missing names
  _eigenstrain_names.resize(verified_eigenstrain_names.size());
  std::copy(verified_eigenstrain_names.begin(),
            verified_eigenstrain_names.end(),
            _eigenstrain_names.begin());

  Moose::out << COLOR_CYAN << "*** Automatic Eigenstrain Names ***"
             << "\n"
             << _name << ": " << Moose::stringify(_eigenstrain_names) << "\n"
             << COLOR_DEFAULT << std::flush;
}

void
TensorMechanicsAction::verifyOrderAndFamilyOutputs()
{
  // Ensure material output order and family vectors are same size as generate output

  // check number of supplied orders and families
  if (_material_output_order.size() > 1 && _material_output_order.size() < _generate_output.size())
    paramError("material_output_order",
               "The number of orders assigned to material outputs must be: 0 to be assigned "
               "CONSTANT; 1 to assign all outputs the same value, or the same size as the number "
               "of generate outputs listed.");

  if (_material_output_family.size() > 1 &&
      _material_output_family.size() < _generate_output.size())
    paramError("material_output_family",
               "The number of families assigned to material outputs must be: 0 to be assigned "
               "MONOMIAL; 1 to assign all outputs the same value, or the same size as the number "
               "of generate outputs listed.");

  // if no value was provided, chose the default CONSTANT
  if (_material_output_order.size() == 0)
    _material_output_order.push_back("CONSTANT");

  // For only one order, make all orders the same magnitude
  if (_material_output_order.size() == 1)
    _material_output_order =
        std::vector<std::string>(_generate_output.size(), _material_output_order[0]);

  if (_verbose)
    Moose::out << COLOR_CYAN << "*** Automatic applied material output orders ***"
               << "\n"
               << _name << ": " << Moose::stringify(_material_output_order) << "\n"
               << COLOR_DEFAULT << std::flush;

  // if no value was provided, chose the default MONOMIAL
  if (_material_output_family.size() == 0)
    _material_output_family.push_back("MONOMIAL");

  // For only one family, make all families that value
  if (_material_output_family.size() == 1)
    _material_output_family =
        std::vector<std::string>(_generate_output.size(), _material_output_family[0]);

  if (_verbose)
    Moose::out << COLOR_CYAN << "*** Automatic applied material output families ***"
               << "\n"
               << _name << ": " << Moose::stringify(_material_output_family) << "\n"
               << COLOR_DEFAULT << std::flush;
}

void
TensorMechanicsAction::actOutputMatProp()
{
  std::string ad_prepend = _use_ad ? "AD" : "";

  if (_current_task == "add_material")
  {
    // Add output Materials
    for (auto out : _generate_output)
    {
      InputParameters params = emptyInputParameters();

      // RankTwoCartesianComponent
      if (
          [&]()
          {
            for (const auto & r2q : _rank_two_cartesian_component_table)
              for (unsigned int a = 0; a < 3; ++a)
                for (unsigned int b = 0; b < 3; ++b)
                  if (r2q.first + '_' + _component_table[a] + _component_table[b] == out)
                  {
                    auto type = ad_prepend + "RankTwoCartesianComponent";
                    params = _factory.getValidParams(type);
                    params.set<MaterialPropertyName>("rank_two_tensor") = _base_name + r2q.second;
                    params.set<unsigned int>("index_i") = a;
                    params.set<unsigned int>("index_j") = b;

                    params.applyParameters(parameters());
                    params.set<MaterialPropertyName>("property_name") = _base_name + out;
                    _problem->addMaterial(type, _base_name + out + '_' + name(), params);
                    return true;
                  }
            return false;
          }())
        continue;

      // RankTwoDirectionalComponent
      if (setupOutput(out,
                      _rank_two_directional_component_table,
                      [&](std::string prop_name, std::string invariant)
                      {
                        auto type = ad_prepend + "RankTwoDirectionalComponent";
                        params = _factory.getValidParams(type);
                        params.set<MaterialPropertyName>("rank_two_tensor") =
                            _base_name + prop_name;
                        params.set<MooseEnum>("invariant") = invariant;
                        params.applyParameters(parameters());
                        params.set<MaterialPropertyName>("property_name") = _base_name + out;
                        _problem->addMaterial(type, _base_name + out + '_' + name(), params);
                      }))
        continue;

      // RankTwoInvariant
      if (setupOutput(out,
                      _rank_two_invariant_table,
                      [&](std::string prop_name, std::string invariant)
                      {
                        auto type = ad_prepend + "RankTwoInvariant";
                        params = _factory.getValidParams(type);
                        params.set<MaterialPropertyName>("rank_two_tensor") =
                            _base_name + prop_name;
                        params.set<MooseEnum>("invariant") = invariant;
                        params.applyParameters(parameters());
                        params.set<MaterialPropertyName>("property_name") = _base_name + out;
                        _problem->addMaterial(type, _base_name + out + '_' + name(), params);
                      }))
        continue;

      // RankTwoCylindricalComponent
      if (setupOutput(
              out,
              _rank_two_cylindrical_component_table,
              [&](std::string prop_name, std::string component)
              {
                if (_coord_system == Moose::COORD_RSPHERICAL)
                  mooseError(
                      "Cannot use cylindrical component output in a spherical coordinate system.");
                auto type = ad_prepend + "RankTwoCylindricalComponent";
                params = _factory.getValidParams(type);
                params.set<MaterialPropertyName>("rank_two_tensor") = _base_name + prop_name;
                params.set<MooseEnum>("cylindrical_component") = component;
                params.applyParameters(parameters());
                params.set<MaterialPropertyName>("property_name") = _base_name + out;
                _problem->addMaterial(type, _base_name + out + '_' + name(), params);
              }))
        continue;

      // RankTwoSphericalComponent
      if (setupOutput(out,
                      _rank_two_spherical_component_table,
                      [&](std::string prop_name, std::string component)
                      {
                        auto type = ad_prepend + "RankTwoSphericalComponent";
                        params = _factory.getValidParams(type);
                        params.set<MaterialPropertyName>("rank_two_tensor") =
                            _base_name + prop_name;
                        params.set<MooseEnum>("spherical_component") = component;
                        params.applyParameters(parameters());
                        params.set<MaterialPropertyName>("property_name") = _base_name + out;
                        _problem->addMaterial(type, _base_name + out + '_' + name(), params);
                      }))
        continue;

      paramError("generate_output", "Unable to add output Material for '", out, "'");
    }
  }
}

void
TensorMechanicsAction::actGatherActionParameters()
{
  // Gather info about all other master actions when we add variables
  if (_current_task == "validate_coordinate_systems" && getParam<bool>("add_variables"))
  {
    auto actions = _awh.getActions<TensorMechanicsAction>();
    for (const auto & action : actions)
    {
      const auto size_before = _subdomain_id_union.size();
      const auto added_size = action->_subdomain_ids.size();
      _subdomain_id_union.insert(action->_subdomain_ids.begin(), action->_subdomain_ids.end());
      const auto size_after = _subdomain_id_union.size();

      if (size_after != size_before + added_size)
        mooseError("The block restrictions in the TensorMechanics/Master actions must be "
                   "non-overlapping.");

      if (added_size == 0 && actions.size() > 1)
        mooseError("No TensorMechanics/Master action can be block unrestricted if more than one "
                   "TensorMechanics/Master action is specified.");
    }
  }
}

void
TensorMechanicsAction::actLagrangianKernelStrain()
{
  std::string type;
  if (_coord_system == Moose::COORD_XYZ)
    type = "ComputeLagrangianStrain";
  else if (_coord_system == Moose::COORD_RZ)
    type = "ComputeLagrangianStrainAxisymmetricCylindrical";
  else if (_coord_system == Moose::COORD_RSPHERICAL)
    type = "ComputeLagrangianStrainCentrosymmetricSpherical";
  else
    mooseError("Unsupported coordinate system");

  auto params = _factory.getValidParams(type);

  if (isParamValid("strain_base_name"))
    params.set<std::string>("base_name") = getParam<std::string>("strain_base_name");

  params.set<std::vector<VariableName>>("displacements") = _coupled_displacements;
  params.set<std::vector<MaterialPropertyName>>("eigenstrain_names") = _eigenstrain_names;
  params.set<bool>("large_kinematics") = _lk_large_kinematics;
  params.set<std::vector<SubdomainName>>("block") = _subdomain_names;

  // Error if volumetric locking correction is on for higher-order elements
  if (_problem->mesh().hasSecondOrderElements() && _lk_locking)
    mooseError("Volumetric locking correction should not be used for "
               "higher-order elements.");

  params.set<bool>("stabilize_strain") = _lk_locking;

  if (_lk_homogenization)
  {
    params.set<std::vector<MaterialPropertyName>>("homogenization_gradient_names") = {
        _homogenization_strain_name};
  }

  _problem->addMaterial(type, name() + "_strain", params);

  // Add the homogenization strain calculator
  if (_lk_homogenization)
  {
    std::string type = "ComputeHomogenizedLagrangianStrain";
    auto params = _factory.getValidParams(type);

    params.set<MaterialPropertyName>("homogenization_gradient_name") = _homogenization_strain_name;
    params.set<std::vector<VariableName>>("macro_gradient") = {_hname};
    params.set<UserObjectName>("homogenization_constraint") = _integrator_name;

    _problem->addMaterial(type, name() + "_compute_" + _homogenization_strain_name, params);
  }
}

void
TensorMechanicsAction::actStressDivergenceTensorsStrain()
{
  std::string ad_prepend = _use_ad ? "AD" : "";

  std::string type;

  // no plane strain
  if (_planar_formulation == PlanarFormulation::None)
  {
    std::map<std::pair<Moose::CoordinateSystemType, StrainAndIncrement>, std::string> type_map = {
        {{Moose::COORD_XYZ, StrainAndIncrement::SmallTotal}, "ComputeSmallStrain"},
        {{Moose::COORD_XYZ, StrainAndIncrement::SmallIncremental}, "ComputeIncrementalSmallStrain"},
        {{Moose::COORD_XYZ, StrainAndIncrement::FiniteIncremental}, "ComputeFiniteStrain"},
        {{Moose::COORD_RZ, StrainAndIncrement::SmallTotal}, "ComputeAxisymmetricRZSmallStrain"},
        {{Moose::COORD_RZ, StrainAndIncrement::SmallIncremental},
         "ComputeAxisymmetricRZIncrementalStrain"},
        {{Moose::COORD_RZ, StrainAndIncrement::FiniteIncremental},
         "ComputeAxisymmetricRZFiniteStrain"},
        {{Moose::COORD_RSPHERICAL, StrainAndIncrement::SmallTotal}, "ComputeRSphericalSmallStrain"},
        {{Moose::COORD_RSPHERICAL, StrainAndIncrement::SmallIncremental},
         "ComputeRSphericalIncrementalStrain"},
        {{Moose::COORD_RSPHERICAL, StrainAndIncrement::FiniteIncremental},
         "ComputeRSphericalFiniteStrain"}};

    auto type_it = type_map.find(std::make_pair(_coord_system, _strain_and_increment));
    if (type_it != type_map.end())
      type = type_it->second;
    else
      mooseError("Unsupported strain formulation");
  }
  else if (_planar_formulation == PlanarFormulation::WeakPlaneStress ||
           _planar_formulation == PlanarFormulation::PlaneStrain ||
           _planar_formulation == PlanarFormulation::GeneralizedPlaneStrain)
  {
    if (_use_ad && (_planar_formulation == PlanarFormulation::PlaneStrain ||
                    _planar_formulation == PlanarFormulation::GeneralizedPlaneStrain))
      paramError("use_automatic_differentiation",
                 "AD not setup for use with PlaneStrain or GeneralizedPlaneStrain");

    std::map<std::pair<Moose::CoordinateSystemType, StrainAndIncrement>, std::string> type_map = {
        {{Moose::COORD_XYZ, StrainAndIncrement::SmallTotal}, "ComputePlaneSmallStrain"},
        {{Moose::COORD_XYZ, StrainAndIncrement::SmallIncremental}, "ComputePlaneIncrementalStrain"},
        {{Moose::COORD_XYZ, StrainAndIncrement::FiniteIncremental}, "ComputePlaneFiniteStrain"},
        {{Moose::COORD_RZ, StrainAndIncrement::SmallTotal}, "ComputeAxisymmetric1DSmallStrain"},
        {{Moose::COORD_RZ, StrainAndIncrement::SmallIncremental},
         "ComputeAxisymmetric1DIncrementalStrain"},
        {{Moose::COORD_RZ, StrainAndIncrement::FiniteIncremental},
         "ComputeAxisymmetric1DFiniteStrain"}};

    // choose kernel type based on coordinate system
    auto type_it = type_map.find(std::make_pair(_coord_system, _strain_and_increment));
    if (type_it != type_map.end())
      type = type_it->second;
    else
      mooseError("Unsupported coordinate system for plane strain.");
  }
  else
    mooseError("Unsupported planar formulation");

  // set material parameters
  auto params = _factory.getValidParams(ad_prepend + type);
  params.applyParameters(
      parameters(),
      {"displacements", "use_displaced_mesh", "out_of_plane_strain", "scalar_out_of_plane_strain"});

  if (isParamValid("strain_base_name"))
    params.set<std::string>("base_name") = getParam<std::string>("strain_base_name");

  params.set<std::vector<VariableName>>("displacements") = _coupled_displacements;
  params.set<bool>("use_displaced_mesh") = false;

  if (isParamValid("scalar_out_of_plane_strain"))
    params.set<std::vector<VariableName>>("scalar_out_of_plane_strain") = {
        getParam<VariableName>("scalar_out_of_plane_strain")};

  if (isParamValid("out_of_plane_strain"))
    params.set<std::vector<VariableName>>("out_of_plane_strain") = {
        getParam<VariableName>("out_of_plane_strain")};

  params.set<std::vector<MaterialPropertyName>>("eigenstrain_names") = _eigenstrain_names;

  _problem->addMaterial(ad_prepend + type, name() + "_strain", params);
}

std::string
TensorMechanicsAction::getKernelType()
{
  if (_lagrangian_kernels)
  {
    std::string type;
    if (_coord_system == Moose::COORD_XYZ)
    {
      if (_lk_homogenization)
        type = "HomogenizedTotalLagrangianStressDivergence";
      else if (_lk_formulation == LKFormulation::Total)
        type = "TotalLagrangianStressDivergence";
      else if (_lk_formulation == LKFormulation::Updated)
        type = "UpdatedLagrangianStressDivergence";
      else
        mooseError("Unknown formulation type");
    }
    else if (_coord_system == Moose::COORD_RZ)
    {
      if (_lk_homogenization)
        mooseError("The Lagrangian mechanics kernels do not yet support homogenization in "
                   "coordinate systems other than Cartesian.");
      else if (_lk_formulation == LKFormulation::Total)
        type = "TotalLagrangianStressDivergenceAxisymmetricCylindrical";
      else if (_lk_formulation == LKFormulation::Updated)
        mooseError("The Lagrangian mechanics kernels do not yet support the updated Lagrangian "
                   "formulation in RZ coordinates.");
    }
    else if (_coord_system == Moose::COORD_RSPHERICAL)
    {
      if (_lk_homogenization)
        mooseError("The Lagrangian mechanics kernels do not yet support homogenization in "
                   "coordinate systems other than Cartesian.");
      else if (_lk_formulation == LKFormulation::Total)
        type = "TotalLagrangianStressDivergenceCentrosymmetricSpherical";
      else if (_lk_formulation == LKFormulation::Updated)
        mooseError("The Lagrangian mechanics kernels do not yet support the updated Lagrangian "
                   "formulation in RZ coordinates.");
    }
    else
      mooseError("Unsupported coordinate system");
    return type;
  }
  else
  {
    std::map<Moose::CoordinateSystemType, std::string> type_map = {
        {Moose::COORD_XYZ, "StressDivergenceTensors"},
        {Moose::COORD_RZ, "StressDivergenceRZTensors"},
        {Moose::COORD_RSPHERICAL, "StressDivergenceRSphericalTensors"}};

    // choose kernel type based on coordinate system
    auto type_it = type_map.find(_coord_system);
    if (type_it != type_map.end())
      return type_it->second;
    else
      mooseError("Unsupported coordinate system");
  }
}

InputParameters
TensorMechanicsAction::getKernelParameters(std::string type)
{
  InputParameters params = _factory.getValidParams(type);
  params.applyParameters(
      parameters(),
      {"displacements", "use_displaced_mesh", "save_in", "diag_save_in", "out_of_plane_strain"});

  params.set<std::vector<VariableName>>("displacements") = _coupled_displacements;

  if (_lagrangian_kernels)
  {
    params.set<bool>("use_displaced_mesh") =
        (_lk_large_kinematics && (_lk_formulation == LKFormulation::Updated));
    params.set<bool>("large_kinematics") = _lk_large_kinematics;
    params.set<bool>("stabilize_strain") = _lk_locking;
  }
  else
    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

  return params;
}
