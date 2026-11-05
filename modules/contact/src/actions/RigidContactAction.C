//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RigidContactAction.h"

#include "ActionFactory.h"
#include "ActionWarehouse.h"
#include "FEProblemBase.h"
#include "Factory.h"
#include "MooseApp.h"
#include "MooseObjectAction.h"

registerMooseAction("ContactApp", RigidContactAction, "append_mesh_generator");
registerMooseAction("ContactApp", RigidContactAction, "add_user_object");
registerMooseAction("ContactApp", RigidContactAction, "add_variable");
registerMooseAction("ContactApp", RigidContactAction, "add_aux_variable");
registerMooseAction("ContactApp", RigidContactAction, "add_bound");
registerMooseAction("ContactApp", RigidContactAction, "add_bounds_vectors");
registerMooseAction("ContactApp", RigidContactAction, "add_nodal_kernel");
registerMooseAction("ContactApp", RigidContactAction, "add_bc");
registerMooseAction("ContactApp", RigidContactAction, "add_scalar_kernel");
registerMooseAction("ContactApp", RigidContactAction, "add_ic");
registerMooseAction("ContactApp", RigidContactAction, "meta_action");

InputParameters
RigidContactAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Expand a `[RigidContact][<name>]` sub-block into the full rigid-body "
                             "analytic-level-set contact stack: LowerDBlockFromSidesetGenerator + "
                             "RigidBodyContactSparsity + normal_lm variable + bounds + "
                             "RigidBodyNodalNCPKernel + RigidBodyNormalMechanicalContact "
                             "(per displacement component), plus the problem-coverage relaxation "
                             "and default SMP preconditioning.  Setting the `force` parameter "
                             "additionally emits the force-control stack (NodalArea UO, scalar "
                             "translation variable, RigidBodyLoadControl kernel).  The contactor "
                             "UO itself is user-defined and referenced by name.");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "Contact sideset(s) on the deformable body's contact face.");
  params.addRequiredParam<std::vector<VariableName>>(
      "displacements", "Displacement variables (x, y[, z]).  Same list as SolidMechanics uses.");
  params.addRequiredParam<UserObjectName>(
      "contactor",
      "Name of a user-defined `LevelSetContactor` UO (`SphereContactor`, "
      "`SurfaceMeshContactor`, ...) supplied by the user in `[UserObjects]`.");
  params.addParam<bool>("add_lower_d_block",
                        true,
                        "Emit a `LowerDBlockFromSidesetGenerator` in the mesh so the LM "
                        "variable has a subdomain to live on.  Turn off if the input's mesh "
                        "already provides a lower-d block on the contact sideset.");
  params.addParam<bool>("add_sparsity_uo",
                        true,
                        "Emit the `RigidBodyContactSparsity` UO (LM/disp cross-node "
                        "preallocation + GhostEverything).  Turn off only for advanced "
                        "users hand-managing sparsity.");
  params.addParam<bool>("enforce_bounds",
                        true,
                        "Emit `bounds_dummy` aux var + `ConstantBounds` on `normal_lm` (>= 0 "
                        "and <= 1e12).  Required when the executioner uses `SNESVINEWTONSSLS`; "
                        "not needed for plain Newton without bounds.");
  params.addParam<bool>("set_problem_coverage_flags",
                        true,
                        "Set `[Problem] kernel_coverage_check = false, material_coverage_check "
                        "= false`.  Needed because the LM variable has no companion "
                        "material/kernel outside this action's NCP nodal kernel.");
  params.addParam<bool>("add_full_smp",
                        true,
                        "Emit `[Preconditioning] type = SMP, full = true` when no "
                        "`[Preconditioning]` block already exists.  Required for the LM "
                        "coupling to be preconditioned correctly.");
  params.addRangeCheckedParam<Real>(
      "c",
      1.0,
      "c > 0",
      "NCP scaling.  The value is passed to both the "
      "`RigidBodyNodalNCPKernel` and (when force-controlled) the "
      "`RigidBodyLoadControl` scalar kernel so their Jacobians remain "
      "consistent.");
  params.addParam<std::string>("lm_variable_name",
                               "",
                               "Override the default LM variable name (default = `normal_lm_<sub-"
                               "block-name>`).  Set to `normal_lm` for the ultra-common single-"
                               "block input to keep the classic name.");
  params.addParam<std::string>("lower_d_block_name",
                               "",
                               "Override the default lower-d subdomain name (default = "
                               "`contact_lower_<sub-block-name>`).");
  params.addParam<SubdomainID>("lower_d_block_id",
                               10001,
                               "Subdomain id given to the newly-created lower-d block.  Must be "
                               "unique across all `[RigidContact]` sub-blocks if you have more "
                               "than one.");

  // Force-control extensions.  All active only if `force` is set.
  params.addParam<FunctionName>("force",
                                "",
                                "Target integrated normal contact reaction F(t).  Setting this "
                                "activates the force-control stack: adds `NodalArea` UO, a scalar "
                                "`indenter_<axis>` variable, and a `RigidBodyLoadControl` kernel.  "
                                "The contactor UO must have its `disp_<axis>_scalar` set to the "
                                "same scalar name (default `indenter_<axis>`).");
  params.addParam<Point>("load_direction",
                         Point(0, 0, 0),
                         "Unit vector along which the contact reaction is measured.  Required "
                         "when `force` is set; ignored otherwise.  Must be axis-aligned.");
  params.addRangeCheckedParam<Real>(
      "kss_stiffness",
      0.0,
      "kss_stiffness >= 0",
      "Constant scalar variant of the effective contact stiffness for the "
      "(scalar, scalar) Jacobian preconditioning shift used by "
      "`RigidBodyLoadControl`.  See that kernel's own docstring; a sensible "
      "value is the deformable body's Young's modulus.  Only used when "
      "`force` is set.  Mutually exclusive with `kss_stiffness_function`.");
  params.addParam<FunctionName>(
      "kss_stiffness_function",
      "Function-of-time variant of the effective contact stiffness passed "
      "through to `RigidBodyLoadControl`.  Only used when `force` is set. "
      "Mutually exclusive with `kss_stiffness`.");
  params.addParam<std::string>("scalar_variable_name",
                               "",
                               "Override the default scalar-variable name for force control "
                               "(default = `indenter_<axis>` derived from `load_direction`).");
  params.addParam<std::string>("nodal_area_variable_name",
                               "",
                               "Override the default nodal-area aux-variable name (default = "
                               "`nodal_area_<sub-block-name>`).");
  params.addParam<Real>("scalar_initial_condition",
                        0.0,
                        "Initial value for the load-control scalar variable.  A small "
                        "nonzero seed (e.g. 5e-3) is often useful so the first Newton "
                        "step has some LM to bite on.  Only used when `force` is set.");
  return params;
}

RigidContactAction::RigidContactAction(const InputParameters & parameters)
  : Action(parameters),
    _boundary(getParam<std::vector<BoundaryName>>("boundary")),
    _displacements(getParam<std::vector<VariableName>>("displacements")),
    _contactor_name(getParam<UserObjectName>("contactor")),
    _add_lower_d_block(getParam<bool>("add_lower_d_block")),
    _add_sparsity_uo(getParam<bool>("add_sparsity_uo")),
    _enforce_bounds(getParam<bool>("enforce_bounds")),
    _set_problem_coverage_flags(getParam<bool>("set_problem_coverage_flags")),
    _add_full_smp(getParam<bool>("add_full_smp")),
    _c(getParam<Real>("c")),
    _lower_d_block_id(getParam<SubdomainID>("lower_d_block_id")),
    _force(getParam<FunctionName>("force")),
    _load_direction(getParam<Point>("load_direction")),
    _kss_stiffness(getParam<Real>("kss_stiffness")),
    _kss_stiffness_function(isParamValid("kss_stiffness_function")
                                ? getParam<FunctionName>("kss_stiffness_function")
                                : FunctionName()),
    _user_lm_name(getParam<std::string>("lm_variable_name")),
    _user_lower_d_name(getParam<std::string>("lower_d_block_name")),
    _user_scalar_name(getParam<std::string>("scalar_variable_name")),
    _user_nodal_area_name(getParam<std::string>("nodal_area_variable_name")),
    _scalar_initial_condition(getParam<Real>("scalar_initial_condition"))
{
  if (!_force.empty() && _load_direction.norm() < TOLERANCE)
    paramError("load_direction", "must be a nonzero axis-aligned unit vector when `force` is set.");
  if (!_kss_stiffness_function.empty() && isParamSetByUser("kss_stiffness"))
    paramError("kss_stiffness_function",
               "Set exactly one of `kss_stiffness` (constant) or "
               "`kss_stiffness_function` (function of time), not both.");
}

unsigned int
RigidContactAction::loadAxisIndex() const
{
  // _load_direction is user-supplied; normalize before axis-checking.
  Point n = _load_direction;
  const Real len = n.norm();
  if (len < TOLERANCE)
    mooseError("RigidContactAction '", name(), "': load_direction is zero.");
  n /= len;
  for (unsigned int k : {0u, 1u, 2u})
    if (std::abs(std::abs(n(k)) - 1.0) < TOLERANCE)
      return k;
  paramError("load_direction", "must be aligned with a Cartesian axis.");
  return libMesh::invalid_uint; // unreachable, silences warning
}

std::string
RigidContactAction::lmName() const
{
  return _user_lm_name.empty() ? "normal_lm_" + name() : _user_lm_name;
}

std::string
RigidContactAction::lowerDName() const
{
  return _user_lower_d_name.empty() ? "contact_lower_" + name() : _user_lower_d_name;
}

std::string
RigidContactAction::scalarName() const
{
  if (!_user_scalar_name.empty())
    return _user_scalar_name;
  const auto axis = loadAxisIndex();
  const std::array<const char *, 3> tags{"x", "y", "z"};
  return std::string("indenter_") + tags[axis];
}

std::string
RigidContactAction::nodalAreaName() const
{
  return _user_nodal_area_name.empty() ? "nodal_area_" + name() : _user_nodal_area_name;
}

void
RigidContactAction::act()
{
  if (_current_task == "append_mesh_generator")
    addMeshGenerators();
  else if (_current_task == "add_user_object")
    addUserObjects();
  else if (_current_task == "add_variable")
  {
    // First task with `_problem` available: also set the coverage
    // flags (they're read during `check_integrity`).  meta_action is
    // too early -- `_problem` is still null there.
    addProblemFlags();
    addVariables();
  }
  else if (_current_task == "add_aux_variable")
    addAuxVariables();
  else if (_current_task == "add_bound")
    addBoundsObjects();
  else if (_current_task == "add_bounds_vectors")
  {
    // Bounds enforcement requires the SNES to have `lower_bound` and
    // `upper_bound` vectors on the nonlinear system.  `AddBoundsVectorsAction`
    // registers these when a `[Bounds]` block is present; when the user has
    // no `[Bounds]` block but we're synthesizing the bounds ourselves, we
    // must add them here.  Guarded on `_enforce_bounds` and idempotent
    // across multiple `[RigidContact]` sub-blocks (only the first sub-block
    // to run this task actually adds; subsequent calls see the vectors
    // already present).
    if (_enforce_bounds && _problem->numNonlinearSystems())
    {
      auto & nl = _problem->getNonlinearSystemBase(0);
      if (!nl.hasVector("lower_bound"))
        nl.addVector("lower_bound", false, libMesh::GHOSTED);
      if (!nl.hasVector("upper_bound"))
        nl.addVector("upper_bound", false, libMesh::GHOSTED);
    }
  }
  else if (_current_task == "add_nodal_kernel")
    addNodalKernels();
  else if (_current_task == "add_bc")
    addBCs();
  else if (_current_task == "add_scalar_kernel")
    addScalarKernels();
  else if (_current_task == "add_ic")
    addInitialConditions();
  else if (_current_task == "meta_action")
  {
    // meta_action fires once early; use it to add default SMP
    // preconditioning if no `[Preconditioning]` block was written.
    // Coverage flags are set later, in `add_variable`, once
    // `_problem` is available.
    addPreconditioning();
  }
}

void
RigidContactAction::addMeshGenerators()
{
  if (!_add_lower_d_block)
    return;
  auto params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");
  // Chain on top of whatever mesh generator already exists so we don't
  // fight the user's `input = ...` chain -- MOOSE's mesh action stitches
  // append_mesh_generator entries into the pipeline in registration
  // order and picks up `input = ` from the previously-registered
  // generator automatically when appended via `appendMeshGenerator`.
  params.set<std::vector<BoundaryName>>("sidesets") = _boundary;
  params.set<SubdomainID>("new_block_id") = _lower_d_block_id;
  params.set<SubdomainName>("new_block_name") = lowerDName();
  const auto gen_name = "rigid_contact_lower_" + name();
  _app.appendMeshGenerator("LowerDBlockFromSidesetGenerator", gen_name, params);
}

void
RigidContactAction::addUserObjects()
{
  if (_add_sparsity_uo)
  {
    auto params = _factory.getValidParams("RigidBodyContactSparsity");
    params.set<VariableName>("lm_variable") = lmName();
    params.set<std::vector<VariableName>>("displacements") = _displacements;
    params.set<std::vector<BoundaryName>>("boundary") = _boundary;
    _problem->addUserObject("RigidBodyContactSparsity", "rigid_contact_sparsity_" + name(), params);
  }
  if (!_force.empty())
  {
    // NodalArea reads/writes the nodal_area aux var on the contact
    // boundary; RigidBodyLoadControl consumes it as tributary weight.
    auto params = _factory.getValidParams("NodalArea");
    params.set<std::vector<BoundaryName>>("boundary") = _boundary;
    params.set<std::vector<VariableName>>("variable") = {nodalAreaName()};
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR};
    _problem->addUserObject("NodalArea", "rigid_contact_nodal_area_" + name(), params);
  }
}

void
RigidContactAction::addVariables()
{
  // LM: LAGRANGE FIRST on the lower-d block.
  {
    auto params = _factory.getValidParams("MooseVariable");
    params.set<MooseEnum>("family") = "LAGRANGE";
    params.set<MooseEnum>("order") = "FIRST";
    params.set<std::vector<SubdomainName>>("block") = {lowerDName()};
    _problem->addVariable("MooseVariable", lmName(), params);
  }
  // Scalar translation variable (force control only).  The IC (if any)
  // is added separately below via `add_ic`; addVariable() doesn't run
  // the AddVariableAction initial_condition-processing path.
  if (!_force.empty())
  {
    auto params = _factory.getValidParams("MooseVariableScalar");
    params.set<MooseEnum>("family") = "SCALAR";
    params.set<MooseEnum>("order") = "FIRST";
    _problem->addVariable("MooseVariableScalar", scalarName(), params);
  }
}

void
RigidContactAction::addAuxVariables()
{
  if (_enforce_bounds)
  {
    auto params = _factory.getValidParams("MooseVariable");
    params.set<MooseEnum>("family") = "LAGRANGE";
    params.set<MooseEnum>("order") = "FIRST";
    params.set<std::vector<SubdomainName>>("block") = {lowerDName()};
    _problem->addAuxVariable("MooseVariable", "bounds_dummy_" + name(), params);
  }
  if (!_force.empty())
  {
    auto params = _factory.getValidParams("MooseVariable");
    params.set<MooseEnum>("family") = "LAGRANGE";
    params.set<MooseEnum>("order") = "FIRST";
    _problem->addAuxVariable("MooseVariable", nodalAreaName(), params);
  }
}

void
RigidContactAction::addBoundsObjects()
{
  if (!_enforce_bounds)
    return;
  // ConstantBounds is an AuxKernel (BoundsBase : AuxKernel), registered
  // through the `[Bounds]` syntax under the `add_bound` task.  We
  // create it via `addAuxKernel` accordingly.
  const std::string dummy = "bounds_dummy_" + name();
  {
    auto params = _factory.getValidParams("ConstantBounds");
    params.set<AuxVariableName>("variable") = dummy;
    params.set<NonlinearVariableName>("bounded_variable") = lmName();
    params.set<MooseEnum>("bound_type") = "lower";
    params.set<Real>("bound_value") = 0.0;
    _problem->addAuxKernel("ConstantBounds", "rigid_contact_lm_lo_" + name(), params);
  }
  {
    auto params = _factory.getValidParams("ConstantBounds");
    params.set<AuxVariableName>("variable") = dummy;
    params.set<NonlinearVariableName>("bounded_variable") = lmName();
    params.set<MooseEnum>("bound_type") = "upper";
    params.set<Real>("bound_value") = 1e12;
    _problem->addAuxKernel("ConstantBounds", "rigid_contact_lm_hi_" + name(), params);
  }
}

void
RigidContactAction::addNodalKernels()
{
  auto params = _factory.getValidParams("RigidBodyNodalNCPKernel");
  params.set<NonlinearVariableName>("variable") = lmName();
  params.set<UserObjectName>("contactor") = _contactor_name;
  params.set<std::vector<VariableName>>("displacements") = _displacements;
  params.set<std::vector<SubdomainName>>("block") = {lowerDName()};
  params.set<Real>("c") = _c;
  _problem->addNodalKernel("RigidBodyNodalNCPKernel", "rigid_contact_ncp_" + name(), params);
}

void
RigidContactAction::addBCs()
{
  const std::array<const char *, 3> comp_tags{"x", "y", "z"};
  for (std::size_t i = 0; i < _displacements.size(); ++i)
  {
    auto params = _factory.getValidParams("RigidBodyNormalMechanicalContact");
    params.set<NonlinearVariableName>("variable") = _displacements[i];
    params.set<std::vector<VariableName>>("lowerd_variable") = {lmName()};
    params.set<std::vector<BoundaryName>>("boundary") = _boundary;
    params.set<UserObjectName>("contactor") = _contactor_name;
    params.set<MooseEnum>("component") = std::string(comp_tags[i]);
    params.set<std::vector<VariableName>>("displacements") = _displacements;
    const std::string bc_name = "rigid_contact_bc_" + name() + "_" + std::string(comp_tags[i]);
    _problem->addBoundaryCondition("RigidBodyNormalMechanicalContact", bc_name, params);
  }
}

void
RigidContactAction::addScalarKernels()
{
  if (_force.empty())
    return;
  auto params = _factory.getValidParams("RigidBodyLoadControl");
  params.set<NonlinearVariableName>("variable") = scalarName();
  params.set<std::vector<BoundaryName>>("boundary") = _boundary;
  params.set<FunctionName>("force") = _force;
  params.set<UserObjectName>("contactor") = _contactor_name;
  params.set<UserObjectName>("nodal_area") = "rigid_contact_nodal_area_" + name();
  params.set<std::vector<VariableName>>("lm_variable") = {lmName()};
  params.set<std::vector<VariableName>>("displacements") = _displacements;
  params.set<Point>("direction") = _load_direction;
  params.set<Real>("c") = _c;
  // Pass through exactly one of the two forms.  RigidBodyLoadControl's own
  // ctor re-validates the mutual-exclusion, but keeping the constant unset
  // when the user gave a function keeps the passthrough clean.
  if (!_kss_stiffness_function.empty())
    params.set<FunctionName>("kss_stiffness_function") = _kss_stiffness_function;
  else
    params.set<Real>("kss_stiffness") = _kss_stiffness;
  _problem->addScalarKernel("RigidBodyLoadControl", "rigid_contact_load_control_" + name(), params);
}

void
RigidContactAction::addInitialConditions()
{
  if (_force.empty() || _scalar_initial_condition == 0.0)
    return;
  auto params = _factory.getValidParams("ScalarConstantIC");
  params.set<VariableName>("variable") = scalarName();
  params.set<Real>("value") = _scalar_initial_condition;
  _problem->addInitialCondition("ScalarConstantIC", "rigid_contact_ic_" + name(), params);
}

void
RigidContactAction::addProblemFlags()
{
  if (!_set_problem_coverage_flags)
    return;
  // _problem is not guaranteed to exist at meta_action; guard.
  if (!_problem)
    return;
  _problem->setKernelCoverageCheck(FEProblemBase::CoverageCheckMode::FALSE);
  _problem->setMaterialCoverageCheck(FEProblemBase::CoverageCheckMode::FALSE);
}

void
RigidContactAction::addPreconditioning()
{
  if (!_add_full_smp)
    return;
  // Don't stomp on a user-supplied `[Preconditioning]` block.  If any
  // add_preconditioning action is present in the warehouse (from a
  // user-written block), skip.
  if (_awh.hasActions("add_preconditioning"))
    return;
  // Build a stub SetupPreconditionerAction that installs an SMP
  // full=true.  This mirrors how the Preconditioning parser expands
  // `[Preconditioning][smp]` internally.
  auto pc_params = _action_factory.getValidParams("SetupPreconditionerAction");
  pc_params.set<std::string>("type") = "SMP";
  auto pc_action =
      _action_factory.create("SetupPreconditionerAction", "rigid_contact_smp_" + name(), pc_params);
  auto mo_action = std::dynamic_pointer_cast<MooseObjectAction>(pc_action);
  if (!mo_action)
    mooseError("RigidContactAction '",
               name(),
               "': SetupPreconditionerAction was not a MooseObjectAction; cannot "
               "install the default SMP preconditioner.  Add an explicit "
               "`[Preconditioning] type = SMP; full = true` block instead.");
  mo_action->getObjectParams().set<bool>("full") = true;
  _awh.addActionBlock(pc_action);
}
