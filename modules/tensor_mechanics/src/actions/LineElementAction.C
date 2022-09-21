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
#include "LineElementAction.h"
#include "CommonLineElementAction.h"
#include "MooseApp.h"
#include "InputParameterWarehouse.h"

#include "libmesh/string_to_enum.h"
#include <algorithm>

registerMooseAction("TensorMechanicsApp", LineElementAction, "create_problem");

registerMooseAction("TensorMechanicsApp", LineElementAction, "add_variable");

registerMooseAction("TensorMechanicsApp", LineElementAction, "add_aux_variable");

registerMooseAction("TensorMechanicsApp", LineElementAction, "add_kernel");

registerMooseAction("TensorMechanicsApp", LineElementAction, "add_aux_kernel");

registerMooseAction("TensorMechanicsApp", LineElementAction, "add_nodal_kernel");

registerMooseAction("TensorMechanicsApp", LineElementAction, "add_material");

InputParameters
LineElementAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Sets up variables, stress divergence kernels and materials required "
                             "for a static analysis with beam or truss elements. Also sets up aux "
                             "variables, aux kernels, and consistent or nodal inertia kernels for "
                             "dynamic analysis with beam elements.");

  params.addParam<bool>(
      "truss",
      false,
      "Set to true if the line elements are truss elements instead of the default beam elements.");
  params.addParam<bool>("add_variables",
                        false,
                        "Add the displacement variables for truss elements "
                        "and both displacement and rotation variables for "
                        "beam elements.");
  params.addParam<std::vector<VariableName>>(
      "displacements", "The nonlinear displacement variables for the problem");

  // Common geometry parameters between beam and truss
  params.addCoupledVar(
      "area",
      "Cross-section area of the beam. Can be supplied as either a number or a variable name.");

  // Beam Parameters
  params += LineElementAction::beamParameters();

  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  // parameters specified here only appear in the input file sub-blocks of the
  // Master action, not in the common parameters area
  params.addParam<std::vector<SubdomainName>>(
      "block",
      "The list of ids of the blocks (subdomain) "
      "that the stress divergence, inertia kernels and materials will be "
      "applied to");
  // Advanced
  params.addParam<std::vector<AuxVariableName>>("save_in",
                                                "The displacement and rotational residuals");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in", "The displacement and rotational diagonal preconditioner terms");
  params.addParamNamesToGroup("block", "Advanced");
  return params;
}

InputParameters
LineElementAction::beamParameters()
{
  InputParameters params = emptyInputParameters();

  params.addParam<std::vector<VariableName>>(
      "rotations", "The rotations appropriate for the simulation geometry and coordinate system");

  MooseEnum strainType("SMALL FINITE", "SMALL");
  params.addParam<MooseEnum>("strain_type", strainType, "Strain formulation");
  params.addParam<MooseEnum>("rotation_type", strainType, "Rotation formulation");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of beam eigenstrains to be applied in this strain calculation.");

  // Beam geometry
  params.addParam<RealGradient>("y_orientation",
                                "Orientation of the y direction along "
                                "which Iyy is provided. This should be "
                                "perpendicular to the axis of the beam.");
  params.addCoupledVar(
      "area",
      "Cross-section area of the beam. Can be supplied as either a number or a variable name.");
  params.addCoupledVar("Ay",
                       0.0,
                       "First moment of area of the beam about y axis. Can be supplied "
                       "as either a number or a variable name.");
  params.addCoupledVar("Az",
                       0.0,
                       "First moment of area of the beam about z axis. Can be supplied "
                       "as either a number or a variable name.");
  params.addCoupledVar("Ix",
                       "Second moment of area of the beam about x axis. Can be supplied as "
                       "either a number or a variable name.");
  params.addCoupledVar("Iy",
                       "Second moment of area of the beam about y axis. Can be supplied as "
                       "either a number or a variable name.");
  params.addCoupledVar("Iz",
                       "Second moment of area of the beam about z axis. Can be supplied as "
                       "either a number or a variable name.");

  // Common parameters for both dynamic consistent and nodal mass/inertia
  params.addParam<bool>("add_dynamic_variables",
                        "Adds translational and rotational velocity and acceleration aux variables "
                        "and sets up the corresponding AuxKernels for calculating these variables "
                        "using Newmark time integration. When dynamic_consistent_inertia, "
                        "dynamic_nodal_rotational_inertia or dynamic_nodal_translational_inertia "
                        "are set to true, these variables are automatically set up.");

  params.addParam<std::vector<VariableName>>("velocities", "Translational velocity variables");
  params.addParam<std::vector<VariableName>>("accelerations",
                                             "Translational acceleration variables");
  params.addParam<std::vector<VariableName>>("rotational_velocities",
                                             "Rotational velocity variables");
  params.addParam<std::vector<VariableName>>("rotational_accelerations",
                                             "Rotational acceleration variables");
  params.addRangeCheckedParam<Real>(
      "beta", "beta>0.0", "beta parameter for Newmark Time integration");
  params.addRangeCheckedParam<Real>(
      "gamma", "gamma>0.0", "gamma parameter for Newmark Time integration");
  params.addParam<MaterialPropertyName>("eta",
                                        0.0,
                                        "Name of material property or a constant real "
                                        "number defining the eta parameter for mass proportional "
                                        "Rayleigh damping.");
  params.addParam<MaterialPropertyName>(
      "zeta",
      0.0,
      "Name of material property or a constant real "
      "number defining the zeta parameter for stiffness proportional "
      "Rayleigh damping.");
  params.addRangeCheckedParam<Real>("alpha",
                                    0,
                                    "alpha>=-0.3333 & alpha<=0.0",
                                    "alpha parameter for mass dependent numerical damping induced "
                                    "by HHT time integration scheme");

  // dynamic consistent mass/inertia
  params.addParam<bool>("dynamic_consistent_inertia",
                        false,
                        "If set to true, consistent mass and "
                        "inertia matrices are used for the "
                        "inertial force/torque calculations.");
  params.addParam<MaterialPropertyName>(
      "density",
      "Name of Material Property or a constant real number defining the density of the beam.");

  // dynamic nodal translational inertia
  params.addParam<bool>(
      "dynamic_nodal_translational_inertia",
      false,
      "If set to true, nodal mass matrix is used for the inertial force calculation.");
  params.addRangeCheckedParam<Real>(
      "nodal_mass", "nodal_mass>0.0", "Mass associated with the node");
  params.addParam<FileName>(
      "nodal_mass_file",
      "The file containing the nodal positions and the corresponding nodal masses.");

  // dynamic nodal rotational inertia
  params.addParam<bool>(
      "dynamic_nodal_rotational_inertia",
      false,
      "If set to true, nodal inertia matrix is used for the inertial torque calculation.");
  params.addRangeCheckedParam<Real>(
      "nodal_Ixx", "nodal_Ixx>=0.0", "Nodal moment of inertia in the x direction.");
  params.addRangeCheckedParam<Real>(
      "nodal_Iyy", "nodal_Iyy>=0.0", "Nodal moment of inertia in the y direction.");
  params.addRangeCheckedParam<Real>(
      "nodal_Izz", "nodal_Izz>=0.0", "Nodal moment of inertia in the z direction.");
  params.addParam<Real>("nodal_Ixy", 0.0, "Nodal moment of inertia in the xy direction.");
  params.addParam<Real>("nodal_Ixz", 0.0, "Nodal moment of inertia in the xz direction.");
  params.addParam<Real>("nodal_Iyz", 0.0, "Nodal moment of inertia in the yz direction.");
  params.addParam<RealGradient>(
      "nodal_x_orientation",
      "Unit vector along the x direction if different from global x direction.");
  params.addParam<RealGradient>(
      "nodal_y_orientation",
      "Unit vector along the y direction if different from global y direction.");
  params.addParam<std::vector<BoundaryName>>(
      "boundary",
      "The list of boundary IDs from the mesh where the nodal "
      "mass/inertia will be applied.");
  return params;
}

LineElementAction::LineElementAction(const InputParameters & params)
  : Action(params),
    _rotations(0),
    _velocities(0),
    _accelerations(0),
    _rot_velocities(0),
    _rot_accelerations(0),
    _subdomain_names(getParam<std::vector<SubdomainName>>("block")),
    _subdomain_ids(),
    _add_dynamic_variables(false)
{
  // FIXME: suggest to use action of action to add this to avoid changing the input parameters in
  // the warehouse.
  const auto & parameters = _app.getInputParameterWarehouse().getInputParameters();
  InputParameters & pars(*(parameters.find(uniqueActionName())->second.get()));

  // check if a container block with common parameters is found
  auto action = _awh.getActions<CommonLineElementAction>();
  if (action.size() == 1)
    pars.applyParameters(action[0]->parameters());

  // Set values to variables after common parameters are applied
  _save_in = getParam<std::vector<AuxVariableName>>("save_in");
  _diag_save_in = getParam<std::vector<AuxVariableName>>("diag_save_in");
  _strain_type = getParam<MooseEnum>("strain_type").getEnum<Strain>();
  _rotation_type = getParam<MooseEnum>("rotation_type").getEnum<Strain>();
  _dynamic_consistent_inertia = getParam<bool>("dynamic_consistent_inertia");
  _dynamic_nodal_translational_inertia = getParam<bool>("dynamic_nodal_translational_inertia");
  _dynamic_nodal_rotational_inertia = getParam<bool>("dynamic_nodal_rotational_inertia");
  if (_dynamic_consistent_inertia || _dynamic_nodal_rotational_inertia ||
      _dynamic_nodal_translational_inertia)
    _add_dynamic_variables = true;

  if (params.isParamSetByUser("add_dynamic_variables"))
  {
    bool user_defined_add_dynamic_variables = getParam<bool>("add_dynamic_variables");
    if (!_add_dynamic_variables && user_defined_add_dynamic_variables)
      _add_dynamic_variables = true;
    else if (_add_dynamic_variables && !user_defined_add_dynamic_variables)
      mooseError("LineElementAction: When using 'dynamic_consistent_inertia', "
                 "'dynamic_nodal_rotational_inertia' or '_dynamic_nodal_translational_inertia', "
                 "the velocity and acceleration AuxVariables and the corresponding AuxKernels are "
                 "automatically set by the action and this cannot be turned off by setting "
                 "'add_dynamic_variables' to false.");
  }
  _truss = getParam<bool>("truss");

  if (!isParamValid("displacements"))
    paramError("displacements",
               "LineElementAction: A vector of displacement variable names should be provided as "
               "input using `displacements`.");

  _displacements = getParam<std::vector<VariableName>>("displacements");
  _ndisp = _displacements.size();

  // determine if displaced mesh is to be used
  _use_displaced_mesh = (_strain_type != Strain::SMALL && _rotation_type != Strain::SMALL);
  if (params.isParamSetByUser("use_displaced_mesh"))
  {
    bool use_displaced_mesh_param = getParam<bool>("use_displaced_mesh");
    if (use_displaced_mesh_param != _use_displaced_mesh && params.isParamSetByUser("strain_type") &&
        params.isParamSetByUser("rotation_type"))
      paramError("use_displaced_mesh",
                 "LineElementAction: Wrong combination of "
                 "`use_displaced_mesh`, `strain_type` and `rotation_type`.");
    _use_displaced_mesh = use_displaced_mesh_param;
  }

  if (_save_in.size() != 0 && _save_in.size() != _ndisp)
    paramError("save_in",
               "LineElementAction: Number of save_in variables should equal to the number of "
               "displacement variables ",
               _ndisp);

  if (_diag_save_in.size() != 0 && _diag_save_in.size() != _ndisp)
    paramError("diag_save_in",
               "LineElementAction: Number of diag_save_in variables should equal to the number of "
               "displacement variables ",
               _ndisp);

  // Check if all the parameters required for static and dynamic beam simulation are provided as
  // input
  if (!_truss)
  {
    // Parameters required for static simulation using beams
    if (!isParamValid("rotations"))
      paramError("rotations",
                 "LineElementAction: Rotational variable names should be provided for beam "
                 "elements using `rotations` parameter.");

    _rotations = getParam<std::vector<VariableName>>("rotations");

    if (_rotations.size() != _ndisp)
      paramError("rotations",
                 "LineElementAction: Number of rotational and displacement variable names provided "
                 "as input for beam should be same.");

    if (!isParamValid("y_orientation") || !isParamValid("area") || !isParamValid("Iy") ||
        !isParamValid("Iz"))
      mooseError("LineElementAction: `y_orientation`, `area`, `Iy` and `Iz` should be provided for "
                 "beam elements.");

    // Parameters required for dynamic simulation using beams
    if (_add_dynamic_variables)
    {
      if (!isParamValid("velocities") || !isParamValid("accelerations") ||
          !isParamValid("rotational_velocities") || !isParamValid("rotational_accelerations"))
        mooseError(
            "LineElementAction: Variable names for translational and rotational velocities "
            "and accelerations should be provided as input to perform dynamic simulation "
            "using beam elements using `velocities`, `accelerations`, `rotational_velocities` and "
            "`rotational_accelerations`.");

      _velocities = getParam<std::vector<VariableName>>("velocities");
      _accelerations = getParam<std::vector<VariableName>>("accelerations");
      _rot_velocities = getParam<std::vector<VariableName>>("rotational_velocities");
      _rot_accelerations = getParam<std::vector<VariableName>>("rotational_accelerations");

      if (_velocities.size() != _ndisp || _accelerations.size() != _ndisp ||
          _rot_velocities.size() != _ndisp || _rot_accelerations.size() != _ndisp)
        mooseError("LineElementAction: Number of translational and rotational velocity and "
                   "acceleration variable names provided as input for the beam should be same as "
                   "number of displacement variables.");

      if (!isParamValid("beta") || !isParamValid("gamma"))
        mooseError("LineElementAction: Newmark time integration parameters `beta` and `gamma` "
                   "should be provided as input to perform dynamic simulations using beams.");
    }

    if (_dynamic_consistent_inertia && !isParamValid("density"))
      paramError("density",
                 "LineElementAction: Either name of the density material property or a constant "
                 "density value should be provided as input using `density` for creating the "
                 "consistent mass/inertia matrix required for dynamic beam simulation.");

    if (_dynamic_nodal_translational_inertia &&
        (!isParamValid("nodal_mass") && !isParamValid("nodal_mass_file")))
      paramError("nodal_mass",
                 "LineElementAction: `nodal_mass` or `nodal_mass_file` should be provided as input "
                 "to calculate "
                 "inertial forces on beam due to nodal mass.");

    if (_dynamic_nodal_rotational_inertia &&
        ((!isParamValid("nodal_Ixx") || !isParamValid("nodal_Iyy") || !isParamValid("nodal_Izz"))))
      mooseError("LineElementAction: `nodal_Ixx`, `nodal_Iyy`, `nodal_Izz` should be provided as "
                 "input to calculate inertial torque on beam due to nodal inertia.");
  }
  else // if truss
  {
    if (!isParamValid("area"))
      paramError("area",
                 "LineElementAction: `area` should be provided as input for "
                 "truss elements.");

    if (isParamValid("rotations"))
      paramError("rotations",
                 "LineElementAction: Rotational variables cannot be set for truss elements.");
  }
}

void
LineElementAction::act()
{
  // Get the subdomain involved in the action once the mesh setup is complete
  if (_current_task == "create_problem")
  {
    // get subdomain IDs
    for (auto & name : _subdomain_names)
      _subdomain_ids.insert(_mesh->getSubdomainID(name));
  }

  if (_current_task == "add_variable")
  {
    //
    // Gather info from all other LineElementAction
    //
    actGatherActionParameters();

    //
    // Add variables (optional)
    //
    actAddVariables();
  }

  //
  // Add Materials - ComputeIncrementalBeamStrain or ComputeFiniteBeamStrain
  // for beam elements
  //
  if (_current_task == "add_material")
    actAddMaterials();

  //
  // Add Kernels - StressDivergenceBeam and InertialForceBeam (if dynamic_consistent_inertia is
  // turned on) for beams and StressDivergenceTensorsTruss for truss elements
  //
  if (_current_task == "add_kernel")
    actAddKernels();

  //
  // Add aux variables for translational and Rotational velocities and acceleration for dynamic
  // analysis using beams
  //
  if (_current_task == "add_aux_variable")
    actAddAuxVariables();

  //
  // Add NewmarkVelAux and NewarkAccelAux auxkernels for dynamic simulation using beams
  //
  if (_current_task == "add_aux_kernel")
    actAddAuxKernels();

  //
  // Add NodalKernels - NodalTranslationalInertia (if dynamic_nodal_translational_inertia is turned
  // on) and NodalRotattionalInertia (if dynamic_nodal_rotational_inertia) for dynamic simulations
  // using beams
  //
  if (_current_task == "add_nodal_kernel")
    actAddNodalKernels();
}

void
LineElementAction::actGatherActionParameters()
{
  //
  // Gather info about all other master actions when we add variables
  //
  if (getParam<bool>("add_variables"))
  {
    auto actions = _awh.getActions<LineElementAction>();
    for (const auto & action : actions)
    {
      const auto size_before = _subdomain_id_union.size();
      const auto added_size = action->_subdomain_ids.size();
      _subdomain_id_union.insert(action->_subdomain_ids.begin(), action->_subdomain_ids.end());
      const auto size_after = _subdomain_id_union.size();

      if (size_after != size_before + added_size)
        paramError("block",
                   "LineElementAction: The block restrictions in the LineElement actions must be "
                   "non-overlapping.");

      if (added_size == 0 && actions.size() > 1)
        paramError(
            "block",
            "LineElementAction: No LineElement action can be block unrestricted if more than one "
            "LineElement action is specified.");
    }
  }
}

void
LineElementAction::actAddVariables()
{
  if (getParam<bool>("add_variables"))
  {
    auto params = _factory.getValidParams("MooseVariable");

    // determine order of elements in mesh
    const bool second = _problem->mesh().hasSecondOrderElements();
    if (second)
      mooseError("LineElementAction: Only linear truss and beam elements are currently supported. "
                 "Please change the order of elements in the mesh to use first order elements.");

    params.set<MooseEnum>("order") = "FIRST";
    params.set<MooseEnum>("family") = "LAGRANGE";

    // Loop through the displacement variables
    for (const auto & disp : _displacements)
    {
      // Create displacement variables
      _problem->addVariable("MooseVariable", disp, params);
    }

    // Add rotation variables if line element is a beam.
    if (!_truss)
    {
      for (const auto & rot : _rotations)
      {
        // Create rotation variables
        _problem->addVariable("MooseVariable", rot, params);
      }
    }
  }
}

void
LineElementAction::actAddMaterials()
{
  if (!_truss)
  {
    // Add Strain
    if (_rotation_type == Strain::SMALL)
    {
      auto params = _factory.getValidParams("ComputeIncrementalBeamStrain");
      params.applyParameters(parameters(), {"boundary", "use_displaced_mesh"});
      params.set<bool>("use_displaced_mesh") = false;

      if (_strain_type == Strain::FINITE)
        params.set<bool>("large_strain") = true;

      _problem->addMaterial("ComputeIncrementalBeamStrain", name() + "_strain", params);
    }
    else if (_rotation_type == Strain::FINITE)
    {
      auto params = _factory.getValidParams("ComputeFiniteBeamStrain");
      params.applyParameters(parameters(), {"boundary", "use_displaced_mesh"});
      params.set<bool>("use_displaced_mesh") = false;

      if (_strain_type == Strain::FINITE)
        params.set<bool>("large_strain") = true;

      _problem->addMaterial("ComputeFiniteBeamStrain", name() + "_strain", params);
    }
  }
}

void
LineElementAction::actAddKernels()
{
  if (!_truss)
  {
    // add StressDivergenceBeam kernels
    auto params = _factory.getValidParams("StressDivergenceBeam");
    params.applyParameters(parameters(), {"use_displaced_mesh", "save_in", "diag_save_in"});
    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

    for (unsigned int i = 0; i < 2 * _ndisp; ++i)
    {
      std::string kernel_name = name() + "_stress_divergence_beam_" + Moose::stringify(i);

      if (i < _ndisp)
      {
        params.set<unsigned int>("component") = i;
        params.set<NonlinearVariableName>("variable") = _displacements[i];

        if (_save_in.size() == 2 * _ndisp)
          params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i]};
        if (_diag_save_in.size() == 2 * _ndisp)
          params.set<std::vector<AuxVariableName>>("diag_save_in") = {_diag_save_in[i]};

        _problem->addKernel("StressDivergenceBeam", kernel_name, params);
      }
      else
      {
        params.set<unsigned int>("component") = i;
        params.set<NonlinearVariableName>("variable") = _rotations[i - 3];

        if (_save_in.size() == 2 * _ndisp)
          params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i]};
        if (_diag_save_in.size() == 2 * _ndisp)
          params.set<std::vector<AuxVariableName>>("diag_save_in") = {_diag_save_in[i]};

        _problem->addKernel("StressDivergenceBeam", kernel_name, params);
      }
    }
    // Add InertialForceBeam if dynamic simulation using consistent mass/inertia matrix has to be
    // performed
    if (_dynamic_consistent_inertia)
    {
      // add InertialForceBeam
      params = _factory.getValidParams("InertialForceBeam");
      params.applyParameters(parameters(), {"use_displaced_mesh", "save_in", "diag_save_in"});
      params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

      for (unsigned int i = 0; i < 2 * _ndisp; ++i)
      {
        std::string kernel_name = name() + "_inertial_force_beam_" + Moose::stringify(i);

        if (i < _ndisp)
        {
          params.set<unsigned int>("component") = i;
          params.set<NonlinearVariableName>("variable") = _displacements[i];

          if (_save_in.size() == 2 * _ndisp)
            params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i]};
          if (_diag_save_in.size() == 2 * _ndisp)
            params.set<std::vector<AuxVariableName>>("diag_save_in") = {_diag_save_in[i]};

          _problem->addKernel("InertialForceBeam", kernel_name, params);
        }
        else
        {
          params.set<unsigned int>("component") = i;
          params.set<NonlinearVariableName>("variable") = _rotations[i - 3];

          if (_save_in.size() == 2 * _ndisp)
            params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i]};
          if (_diag_save_in.size() == 2 * _ndisp)
            params.set<std::vector<AuxVariableName>>("diag_save_in") = {_diag_save_in[i]};

          _problem->addKernel("InertialForceBeam", kernel_name, params);
        }
      }
    }
  }
  else
  {
    // Add StressDivergenceTensorsTruss kernels
    auto params = _factory.getValidParams("StressDivergenceTensorsTruss");
    params.applyParameters(parameters(), {"use_displaced_mesh", "save_in", "diag_save_in"});
    params.set<bool>("use_displaced_mesh") = true;

    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      std::string kernel_name = name() + "_stress_divergence_truss_" + Moose::stringify(i);
      params.set<unsigned int>("component") = i;
      params.set<NonlinearVariableName>("variable") = _displacements[i];

      if (_save_in.size() == _ndisp)
        params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i]};
      if (_diag_save_in.size() == _ndisp)
        params.set<std::vector<AuxVariableName>>("diag_save_in") = {_diag_save_in[i]};

      _problem->addKernel("StressDivergenceTensorsTruss", kernel_name, params);
    }
  }
}

void
LineElementAction::actAddAuxVariables()
{
  if (_add_dynamic_variables && !_truss)
  {
    auto params = _factory.getValidParams("MooseVariable");

    params.set<MooseEnum>("order") = "FIRST";
    params.set<MooseEnum>("family") = "LAGRANGE";

    for (auto vel : _velocities)
      _problem->addAuxVariable("MooseVariable", vel, params);

    for (auto accel : _accelerations)
      _problem->addAuxVariable("MooseVariable", accel, params);

    for (auto rot_vel : _rot_velocities)
      _problem->addAuxVariable("MooseVariable", rot_vel, params);

    for (auto rot_accel : _rot_accelerations)
      _problem->addAuxVariable("MooseVariable", rot_accel, params);
  }
}

void
LineElementAction::actAddAuxKernels()
{
  if (_add_dynamic_variables && !_truss)
  {
    auto params = _factory.getValidParams("NewmarkAccelAux");
    params.applyParameters(parameters(), {"boundary"});
    params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

    for (unsigned i = 0; i < 2 * _ndisp; ++i)
    {
      std::string aux_kernel_name = name() + "_newmark_accel_" + Moose::stringify(i);

      if (i < _ndisp)
      {
        params.set<AuxVariableName>("variable") = _accelerations[i];
        params.set<std::vector<VariableName>>("velocity") = {_velocities[i]};
        params.set<std::vector<VariableName>>("displacement") = {_displacements[i]};

        _problem->addAuxKernel("NewmarkAccelAux", aux_kernel_name, params);
      }
      else
      {
        params.set<AuxVariableName>("variable") = _rot_accelerations[i - _ndisp];
        params.set<std::vector<VariableName>>("velocity") = {_rot_velocities[i - _ndisp]};
        params.set<std::vector<VariableName>>("displacement") = {_rotations[i - _ndisp]};

        _problem->addAuxKernel("NewmarkAccelAux", aux_kernel_name, params);
      }
    }

    params = _factory.getValidParams("NewmarkVelAux");
    params.applyParameters(parameters(), {"boundary"});
    params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

    for (unsigned i = 0; i < 2 * _ndisp; ++i)
    {
      std::string aux_kernel_name = name() + "_newmark_vel_" + Moose::stringify(i);

      if (i < _ndisp)
      {
        params.set<AuxVariableName>("variable") = _velocities[i];
        params.set<std::vector<VariableName>>("acceleration") = {_accelerations[i]};
        _problem->addAuxKernel("NewmarkVelAux", aux_kernel_name, params);
      }
      else
      {
        params.set<AuxVariableName>("variable") = _rot_velocities[i - _ndisp];
        params.set<std::vector<VariableName>>("acceleration") = {_rot_accelerations[i - _ndisp]};
        _problem->addAuxKernel("NewmarkVelAux", aux_kernel_name, params);
      }
    }
  }
}

void
LineElementAction::actAddNodalKernels()
{
  if (!_truss)
  {
    // NodalTranslationalInertia and NodalRotattionalInertia currently accept only constant real
    // numbers for eta
    Real eta = 0.0;
    if (_dynamic_nodal_rotational_inertia || _dynamic_nodal_translational_inertia)
    {
      std::string ss(getParam<MaterialPropertyName>("eta"));
      Real real_value = MooseUtils::convert<Real>(ss);

      eta = real_value;
    }

    if (_dynamic_nodal_translational_inertia)
    {
      auto params = _factory.getValidParams("NodalTranslationalInertia");
      params.applyParameters(parameters(),
                             {"save_in", "diag_save_in", "use_displaced_mesh", "eta"});
      params.set<Real>("mass") = getParam<Real>("nodal_mass");
      params.set<Real>("eta") = eta;
      params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

      for (unsigned i = 0; i < _ndisp; ++i)
      {
        std::string nodal_kernel_name =
            name() + "_nodal_translational_inertia_" + Moose::stringify(i);

        params.set<NonlinearVariableName>("variable") = _displacements[i];
        params.set<std::vector<VariableName>>("velocity") = {_velocities[i]};
        params.set<std::vector<VariableName>>("acceleration") = {_accelerations[i]};

        if (_save_in.size() == 2 * _ndisp)
          params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i]};

        if (_diag_save_in.size() == 2 * _ndisp)
          params.set<std::vector<AuxVariableName>>("diag_save_in") = {_diag_save_in[i]};

        _problem->addNodalKernel("NodalTranslationalInertia", nodal_kernel_name, params);
      }
    }

    if (_dynamic_nodal_rotational_inertia)
    {
      auto params = _factory.getValidParams("NodalRotationalInertia");
      params.applyParameters(parameters(),
                             {"save_in",
                              "diag_save_in",
                              "use_displaced_mesh",
                              "eta",
                              "x_orientation",
                              "y_orientation"});
      params.set<Real>("Ixx") = getParam<Real>("nodal_Ixx");
      params.set<Real>("Iyy") = getParam<Real>("nodal_Iyy");
      params.set<Real>("Izz") = getParam<Real>("nodal_Izz");
      params.set<Real>("eta") = eta;
      params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

      if (isParamValid("nodal_Ixy"))
        params.set<Real>("Ixy") = getParam<Real>("nodal_Ixy");

      if (isParamValid("nodal_Ixz"))
        params.set<Real>("Ixz") = getParam<Real>("nodal_Ixz");

      if (isParamValid("nodal_Iyz"))
        params.set<Real>("Iyz") = getParam<Real>("nodal_Iyz");

      if (isParamValid("nodal_x_orientation"))
        params.set<Real>("x_orientation") = getParam<Real>("nodal_x_orientation");

      if (isParamValid("nodal_y_orientation"))
        params.set<Real>("y_orientation") = getParam<Real>("nodal_y_orientation");

      for (unsigned i = 0; i < _ndisp; ++i)
      {
        std::string nodal_kernel_name = name() + "_nodal_rotational_inertia_" + Moose::stringify(i);

        params.set<unsigned int>("component") = i;
        params.set<NonlinearVariableName>("variable") = _rotations[i];

        if (_save_in.size() == 2 * _ndisp)
          params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i + _ndisp]};

        if (_diag_save_in.size() == 2 * _ndisp)
          params.set<std::vector<AuxVariableName>>("diag_save_in") = {_diag_save_in[i + _ndisp]};

        _problem->addNodalKernel("NodalRotationalInertia", nodal_kernel_name, params);
      }
    }
  }
}
