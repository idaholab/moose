//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEProblemBase.h"
#include "AuxiliarySystem.h"
#include "MaterialPropertyStorage.h"
#include "MooseEnum.h"
#include "Factory.h"
#include "MooseUtils.h"
#include "DisplacedProblem.h"
#include "SystemBase.h"
#include "MaterialData.h"
#include "ComputeUserObjectsThread.h"
#include "ComputeNodalUserObjectsThread.h"
#include "ComputeThreadedGeneralUserObjectsThread.h"
#include "ComputeMaterialsObjectThread.h"
#include "ProjectMaterialProperties.h"
#include "ComputeIndicatorThread.h"
#include "ComputeMarkerThread.h"
#include "ComputeInitialConditionThread.h"
#include "ComputeFVInitialConditionThread.h"
#include "ComputeBoundaryInitialConditionThread.h"
#include "MaxQpsThread.h"
#include "ActionWarehouse.h"
#include "Conversion.h"
#include "Material.h"
#include "FunctorMaterial.h"
#include "ConstantIC.h"
#include "Parser.h"
#include "ElementH1Error.h"
#include "Function.h"
#include "Convergence.h"
#include "NonlinearSystem.h"
#include "LinearSystem.h"
#include "SolverSystem.h"
#include "Distribution.h"
#include "Sampler.h"
#include "PetscSupport.h"
#include "RandomInterface.h"
#include "RandomData.h"
#include "MooseEigenSystem.h"
#include "MooseParsedFunction.h"
#include "MeshChangedInterface.h"
#include "ComputeJacobianBlocksThread.h"
#include "ScalarInitialCondition.h"
#include "FVInitialConditionTempl.h"
#include "ElementPostprocessor.h"
#include "NodalPostprocessor.h"
#include "SidePostprocessor.h"
#include "InternalSidePostprocessor.h"
#include "InterfacePostprocessor.h"
#include "GeneralPostprocessor.h"
#include "ElementVectorPostprocessor.h"
#include "NodalVectorPostprocessor.h"
#include "SideVectorPostprocessor.h"
#include "InternalSideVectorPostprocessor.h"
#include "GeneralVectorPostprocessor.h"
#include "Positions.h"
#include "Indicator.h"
#include "Marker.h"
#include "MultiApp.h"
#include "MultiAppTransfer.h"
#include "TransientMultiApp.h"
#include "ElementUserObject.h"
#include "DomainUserObject.h"
#include "NodalUserObject.h"
#include "SideUserObject.h"
#include "InternalSideUserObject.h"
#include "InterfaceUserObject.h"
#include "GeneralUserObject.h"
#include "ThreadedGeneralUserObject.h"
#include "InternalSideIndicator.h"
#include "Transfer.h"
#include "MultiAppTransfer.h"
#include "MultiMooseEnum.h"
#include "Predictor.h"
#include "Assembly.h"
#include "Control.h"
#include "XFEMInterface.h"
#include "ConsoleUtils.h"
#include "NonlocalKernel.h"
#include "NonlocalIntegratedBC.h"
#include "ShapeElementUserObject.h"
#include "ShapeSideUserObject.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "InputParameterWarehouse.h"
#include "TimeIntegrator.h"
#include "LineSearch.h"
#include "FloatingPointExceptionGuard.h"
#include "MaxVarNDofsPerElem.h"
#include "MaxVarNDofsPerNode.h"
#include "FVKernel.h"
#include "LinearFVKernel.h"
#include "FVTimeKernel.h"
#include "MooseVariableFV.h"
#include "MooseLinearVariableFV.h"
#include "FVBoundaryCondition.h"
#include "LinearFVBoundaryCondition.h"
#include "FVInterfaceKernel.h"
#include "Reporter.h"
#include "ADUtils.h"
#include "Executioner.h"
#include "VariadicTable.h"
#include "BoundaryNodeIntegrityCheckThread.h"
#include "BoundaryElemIntegrityCheckThread.h"
#include "NodalBCBase.h"
#include "MortarUserObject.h"
#include "MortarUserObjectThread.h"
#include "RedistributeProperties.h"
#include "Checkpoint.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/quadrature.h"
#include "libmesh/coupling_matrix.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe_interface.h"
#include "libmesh/enum_norm_type.h"
#include "libmesh/petsc_solver_exception.h"

#include "metaphysicl/dualnumber.h"

using namespace libMesh;

// Anonymous namespace for helper function
namespace
{
/**
 * Method for sorting the MooseVariableFEBases based on variable numbers
 */
bool
sortMooseVariables(const MooseVariableFEBase * a, const MooseVariableFEBase * b)
{
  return a->number() < b->number();
}
} // namespace

Threads::spin_mutex get_function_mutex;

InputParameters
FEProblemBase::validParams()
{
  InputParameters params = SubProblem::validParams();
  params.addParam<unsigned int>("null_space_dimension", 0, "The dimension of the nullspace");
  params.addParam<unsigned int>(
      "transpose_null_space_dimension", 0, "The dimension of the transpose nullspace");
  params.addParam<unsigned int>(
      "near_null_space_dimension", 0, "The dimension of the near nullspace");
  params.addParam<bool>("solve",
                        true,
                        "Whether or not to actually solve the Nonlinear system.  "
                        "This is handy in the case that all you want to do is "
                        "execute AuxKernels, Transfers, etc. without actually "
                        "solving anything");
  params.addParam<bool>("use_nonlinear",
                        true,
                        "Determines whether to use a Nonlinear vs a "
                        "Eigenvalue system (Automatically determined based "
                        "on executioner)");
  params.addParam<bool>("error_on_jacobian_nonzero_reallocation",
                        "This causes PETSc to error if it had to reallocate memory in the Jacobian "
                        "matrix due to not having enough nonzeros");
  params.addParam<bool>("ignore_zeros_in_jacobian",
                        false,
                        "Do not explicitly store zero values in "
                        "the Jacobian matrix if true");
  params.addParam<bool>("force_restart",
                        false,
                        "EXPERIMENTAL: If true, a sub_app may use a "
                        "restart file instead of using of using the master "
                        "backup file");
  params.addDeprecatedParam<bool>("skip_additional_restart_data",
                                  false,
                                  "True to skip additional data in equation system for restart.",
                                  "This parameter is no longer used, as we do not load additional "
                                  "vectors by default with restart");
  params.addParam<bool>("skip_nl_system_check",
                        false,
                        "True to skip the NonlinearSystem check for work to do (e.g. Make sure "
                        "that there are variables to solve for).");
  params.addParam<bool>("allow_initial_conditions_with_restart",
                        false,
                        "True to allow the user to specify initial conditions when restarting. "
                        "Initial conditions can override any restarted field");

  /// One entry of coord system per block, the size of _blocks and _coord_sys has to match, except:
  /// 1. _blocks.size() == 0, then there needs to be just one entry in _coord_sys, which will
  ///    be set for the whole domain
  /// 2. _blocks.size() > 0 and no coordinate system was specified, then the whole domain will be XYZ.
  /// 3. _blocks.size() > 0 and one coordinate system was specified, then the whole domain will be that system.
  params.addDeprecatedParam<std::vector<SubdomainName>>(
      "block", {}, "Block IDs for the coordinate systems", "Please use 'Mesh/coord_block' instead");
  MultiMooseEnum coord_types("XYZ RZ RSPHERICAL", "XYZ");
  MooseEnum rz_coord_axis("X=0 Y=1", "Y");
  params.addDeprecatedParam<MultiMooseEnum>("coord_type",
                                            coord_types,
                                            "Type of the coordinate system per block param",
                                            "Please use 'Mesh/coord_type' instead");
  params.addDeprecatedParam<MooseEnum>("rz_coord_axis",
                                       rz_coord_axis,
                                       "The rotation axis (X | Y) for axisymetric coordinates",
                                       "Please use 'Mesh/rz_coord_axis' instead");
  auto coverage_check_description = [](std::string scope, std::string list_param_name)
  {
    return "Controls, if and how a " + scope +
           " subdomain coverage check is performed. "
           "With 'TRUE' or 'ON' all subdomains are checked (the default). Setting 'FALSE' or 'OFF' "
           "will disable the check for all subdomains. "
           "To exclude a predefined set of subdomains 'SKIP_LIST' is to "
           "be used, while the subdomains to skip are to be defined in the parameter '" +
           list_param_name +
           "'. To limit the check to a list of subdomains, 'ONLY_LIST' is to "
           "be used (again, using the parameter '" +
           list_param_name + "').";
  };
  MooseEnum kernel_coverage_check_modes("FALSE TRUE OFF ON SKIP_LIST ONLY_LIST", "TRUE");
  params.addParam<MooseEnum>("kernel_coverage_check",
                             kernel_coverage_check_modes,
                             coverage_check_description("kernel", "kernel_coverage_block_list"));
  params.addParam<std::vector<SubdomainName>>(
      "kernel_coverage_block_list",
      {},
      "List of subdomains for kernel coverage check. The meaning of this list is controlled by the "
      "parameter 'kernel_coverage_check' (whether this is the list of subdomains to be checked, "
      "not to be checked or not taken into account).");
  params.addParam<bool>(
      "boundary_restricted_node_integrity_check",
      true,
      "Set to false to disable checking of boundary restricted nodal object variable dependencies, "
      "e.g. are the variable dependencies defined on the selected boundaries?");
  params.addParam<bool>("boundary_restricted_elem_integrity_check",
                        true,
                        "Set to false to disable checking of boundary restricted elemental object "
                        "variable dependencies, e.g. are the variable dependencies defined on the "
                        "selected boundaries?");
  MooseEnum material_coverage_check_modes("FALSE TRUE OFF ON SKIP_LIST ONLY_LIST", "TRUE");
  params.addParam<MooseEnum>(
      "material_coverage_check",
      material_coverage_check_modes,
      coverage_check_description("material", "material_coverage_block_list"));
  params.addParam<std::vector<SubdomainName>>(
      "material_coverage_block_list",
      {},
      "List of subdomains for material coverage check. The meaning of this list is controlled by "
      "the parameter 'material_coverage_check' (whether this is the list of subdomains to be "
      "checked, not to be checked or not taken into account).");

  params.addParam<bool>("fv_bcs_integrity_check",
                        true,
                        "Set to false to disable checking of overlapping Dirichlet and Flux BCs "
                        "and/or multiple DirichletBCs per sideset");

  params.addParam<bool>(
      "material_dependency_check", true, "Set to false to disable material dependency check");
  params.addParam<bool>("parallel_barrier_messaging",
                        false,
                        "Displays messaging from parallel "
                        "barrier notifications when executing "
                        "or transferring to/from Multiapps "
                        "(default: false)");

  MooseEnum verbosity("false true extra", "false");
  params.addParam<MooseEnum>("verbose_setup",
                             verbosity,
                             "Set to 'true' to have the problem report on any object created. Set "
                             "to 'extra' to also display all parameters.");
  params.addParam<bool>("verbose_multiapps",
                        false,
                        "Set to True to enable verbose screen printing related to MultiApps");

  params.addParam<FileNameNoExtension>("restart_file_base",
                                       "File base name used for restart (e.g. "
                                       "<path>/<filebase> or <path>/LATEST to "
                                       "grab the latest file available)");

  params.addParam<std::vector<std::vector<TagName>>>(
      "extra_tag_vectors",
      {},
      "Extra vectors to add to the system that can be filled by objects which compute residuals "
      "and Jacobians (Kernels, BCs, etc.) by setting tags on them. The outer index is for which "
      "nonlinear system the extra tag vectors should be added for");

  params.addParam<std::vector<std::vector<TagName>>>(
      "not_zeroed_tag_vectors",
      {},
      "Extra vector tags which the sytem will not zero when other vector tags are zeroed. "
      "The outer index is for which nonlinear system the extra tag vectors should be added for");

  params.addParam<std::vector<std::vector<TagName>>>(
      "extra_tag_matrices",
      {},
      "Extra matrices to add to the system that can be filled "
      "by objects which compute residuals and Jacobians "
      "(Kernels, BCs, etc.) by setting tags on them. The outer index is for which "
      "nonlinear system the extra tag vectors should be added for");

  params.addParam<std::vector<TagName>>(
      "extra_tag_solutions",
      {},
      "Extra solution vectors to add to the system that can be used by "
      "objects for coupling variable values stored in them.");

  params.addParam<bool>("previous_nl_solution_required",
                        false,
                        "True to indicate that this calculation requires a solution vector for "
                        "storing the previous nonlinear iteration.");

  params.addParam<std::vector<NonlinearSystemName>>(
      "nl_sys_names", std::vector<NonlinearSystemName>{"nl0"}, "The nonlinear system names");

  params.addParam<std::vector<LinearSystemName>>("linear_sys_names", {}, "The linear system names");

  params.addParam<bool>("check_uo_aux_state",
                        false,
                        "True to turn on a check that no state presents during the evaluation of "
                        "user objects and aux kernels");

  params.addPrivateParam<MooseMesh *>("mesh");

  params.declareControllable("solve");

  params.addParam<bool>(
      "allow_invalid_solution",
      false,
      "Set to true to allow convergence even though the solution has been marked as 'invalid'");
  params.addParam<bool>("show_invalid_solution_console",
                        true,
                        "Set to true to show the invalid solution occurance summary in console");
  params.addParam<bool>("immediately_print_invalid_solution",
                        false,
                        "Whether or not to report invalid solution warnings at the time the "
                        "warning is produced instead of after the calculation");

  params.addParam<bool>(
      "identify_variable_groups_in_nl",
      true,
      "Whether to identify variable groups in nonlinear systems. This affects dof ordering");

  params.addParam<bool>(
      "regard_general_exceptions_as_errors",
      false,
      "If we catch an exception during residual/Jacobian evaluaton for which we don't have "
      "specific handling, immediately error instead of allowing the time step to be cut");

  params.addParamNamesToGroup(
      "skip_nl_system_check kernel_coverage_check kernel_coverage_block_list "
      "boundary_restricted_node_integrity_check "
      "boundary_restricted_elem_integrity_check material_coverage_check "
      "material_coverage_block_list fv_bcs_integrity_check "
      "material_dependency_check check_uo_aux_state error_on_jacobian_nonzero_reallocation",
      "Simulation checks");
  params.addParamNamesToGroup("use_nonlinear previous_nl_solution_required nl_sys_names "
                              "ignore_zeros_in_jacobian identify_variable_groups_in_nl",
                              "Nonlinear system(s)");
  params.addParamNamesToGroup(
      "restart_file_base force_restart allow_initial_conditions_with_restart", "Restart");
  params.addParamNamesToGroup("verbose_setup verbose_multiapps parallel_barrier_messaging",
                              "Verbosity");
  params.addParamNamesToGroup(
      "null_space_dimension transpose_null_space_dimension near_null_space_dimension",
      "Null space removal");
  params.addParamNamesToGroup("extra_tag_vectors extra_tag_matrices extra_tag_solutions",
                              "Tagging");
  params.addParamNamesToGroup(
      "allow_invalid_solution show_invalid_solution_console immediately_print_invalid_solution",
      "Solution validity control");

  return params;
}

FEProblemBase::FEProblemBase(const InputParameters & parameters)
  : SubProblem(parameters),
    Restartable(this, "FEProblemBase"),
    _mesh(*getCheckedPointerParam<MooseMesh *>("mesh")),
    _req(declareManagedRestartableDataWithContext<RestartableEquationSystems>(
        "equation_systems", nullptr, _mesh)),
    _initialized(false),
    _solve(getParam<bool>("solve")),
    _transient(false),
    _time(declareRestartableData<Real>("time")),
    _time_old(declareRestartableData<Real>("time_old")),
    _t_step(declareRecoverableData<int>("t_step")),
    _dt(declareRestartableData<Real>("dt")),
    _dt_old(declareRestartableData<Real>("dt_old")),
    _set_nonlinear_convergence_names(false),
    _need_to_add_default_nonlinear_convergence(false),
    _linear_sys_names(getParam<std::vector<LinearSystemName>>("linear_sys_names")),
    _num_linear_sys(_linear_sys_names.size()),
    _linear_systems(_num_linear_sys, nullptr),
    _current_linear_sys(nullptr),
    _using_default_nl(!isParamSetByUser("nl_sys_names")),
    _nl_sys_names(!_using_default_nl || (_using_default_nl && !_linear_sys_names.size())
                      ? getParam<std::vector<NonlinearSystemName>>("nl_sys_names")
                      : std::vector<NonlinearSystemName>()),
    _num_nl_sys(_nl_sys_names.size()),
    _nl(_num_nl_sys, nullptr),
    _current_nl_sys(nullptr),
    _solver_systems(_num_nl_sys + _num_linear_sys, nullptr),
    _aux(nullptr),
    _coupling(Moose::COUPLING_DIAG),
    _mesh_divisions(/*threaded=*/true),
    _material_props(declareRestartableDataWithContext<MaterialPropertyStorage>(
        "material_props", &_mesh, _material_prop_registry)),
    _bnd_material_props(declareRestartableDataWithContext<MaterialPropertyStorage>(
        "bnd_material_props", &_mesh, _material_prop_registry)),
    _neighbor_material_props(declareRestartableDataWithContext<MaterialPropertyStorage>(
        "neighbor_material_props", &_mesh, _material_prop_registry)),
    _reporter_data(_app),
    // TODO: delete the following line after apps have been updated to not call getUserObjects
    _all_user_objects(_app.getExecuteOnEnum()),
    _multi_apps(_app.getExecuteOnEnum()),
    _transient_multi_apps(_app.getExecuteOnEnum()),
    _transfers(_app.getExecuteOnEnum(), /*threaded=*/false),
    _to_multi_app_transfers(_app.getExecuteOnEnum(), /*threaded=*/false),
    _from_multi_app_transfers(_app.getExecuteOnEnum(), /*threaded=*/false),
    _between_multi_app_transfers(_app.getExecuteOnEnum(), /*threaded=*/false),
#ifdef LIBMESH_ENABLE_AMR
    _adaptivity(*this),
    _cycles_completed(0),
#endif
    _displaced_mesh(nullptr),
    _geometric_search_data(*this, _mesh),
    _mortar_data(*this),
    _reinit_displaced_elem(false),
    _reinit_displaced_face(false),
    _reinit_displaced_neighbor(false),
    _input_file_saved(false),
    _has_dampers(false),
    _has_constraints(false),
    _snesmf_reuse_base(true),
    _skip_exception_check(false),
    _snesmf_reuse_base_set_by_user(false),
    _has_initialized_stateful(false),
    _const_jacobian(false),
    _has_jacobian(false),
    _needs_old_newton_iter(false),
    _previous_nl_solution_required(getParam<bool>("previous_nl_solution_required")),
    _has_nonlocal_coupling(false),
    _calculate_jacobian_in_uo(false),
    _kernel_coverage_check(
        getParam<MooseEnum>("kernel_coverage_check").getEnum<CoverageCheckMode>()),
    _kernel_coverage_blocks(getParam<std::vector<SubdomainName>>("kernel_coverage_block_list")),
    _boundary_restricted_node_integrity_check(
        getParam<bool>("boundary_restricted_node_integrity_check")),
    _boundary_restricted_elem_integrity_check(
        getParam<bool>("boundary_restricted_elem_integrity_check")),
    _material_coverage_check(
        getParam<MooseEnum>("material_coverage_check").getEnum<CoverageCheckMode>()),
    _material_coverage_blocks(getParam<std::vector<SubdomainName>>("material_coverage_block_list")),
    _fv_bcs_integrity_check(getParam<bool>("fv_bcs_integrity_check")),
    _material_dependency_check(getParam<bool>("material_dependency_check")),
    _uo_aux_state_check(getParam<bool>("check_uo_aux_state")),
    _max_qps(std::numeric_limits<unsigned int>::max()),
    _max_scalar_order(INVALID_ORDER),
    _has_time_integrator(false),
    _has_exception(false),
    _parallel_barrier_messaging(getParam<bool>("parallel_barrier_messaging")),
    _verbose_setup(getParam<MooseEnum>("verbose_setup")),
    _verbose_multiapps(getParam<bool>("verbose_multiapps")),
    _current_execute_on_flag(EXEC_NONE),
    _control_warehouse(_app.getExecuteOnEnum(), /*threaded=*/false),
    _is_petsc_options_inserted(false),
    _line_search(nullptr),
    _using_ad_mat_props(false),
    _current_ic_state(0),
    _error_on_jacobian_nonzero_reallocation(
        isParamValid("error_on_jacobian_nonzero_reallocation")
            ? getParam<bool>("error_on_jacobian_nonzero_reallocation")
            : _app.errorOnJacobianNonzeroReallocation()),
    _ignore_zeros_in_jacobian(getParam<bool>("ignore_zeros_in_jacobian")),
    _preserve_matrix_sparsity_pattern(true),
    _force_restart(getParam<bool>("force_restart")),
    _allow_ics_during_restart(getParam<bool>("allow_initial_conditions_with_restart")),
    _skip_nl_system_check(getParam<bool>("skip_nl_system_check")),
    _fail_next_nonlinear_convergence_check(false),
    _allow_invalid_solution(getParam<bool>("allow_invalid_solution")),
    _show_invalid_solution_console(getParam<bool>("show_invalid_solution_console")),
    _immediately_print_invalid_solution(getParam<bool>("immediately_print_invalid_solution")),
    _started_initial_setup(false),
    _has_internal_edge_residual_objects(false),
    _u_dot_requested(false),
    _u_dotdot_requested(false),
    _u_dot_old_requested(false),
    _u_dotdot_old_requested(false),
    _has_mortar(false),
    _num_grid_steps(0),
    _print_execution_on(),
    _identify_variable_groups_in_nl(getParam<bool>("identify_variable_groups_in_nl")),
    _regard_general_exceptions_as_errors(getParam<bool>("regard_general_exceptions_as_errors"))
{
  //  Initialize static do_derivatives member. We initialize this to true so that all the default AD
  //  things that we setup early in the simulation actually get their derivative vectors initalized.
  //  We will toggle this to false when doing residual evaluations
  ADReal::do_derivatives = true;

  for (const auto i : index_range(_nl_sys_names))
  {
    const auto & name = _nl_sys_names[i];
    _nl_sys_name_to_num[name] = i;
    _solver_sys_name_to_num[name] = i;
    _solver_sys_names.push_back(name);
  }

  for (const auto i : index_range(_linear_sys_names))
  {
    const auto & name = _linear_sys_names[i];
    _linear_sys_name_to_num[name] = i;
    _solver_sys_name_to_num[name] = i + _num_nl_sys;
    _solver_sys_names.push_back(name);
  }

  _nonlocal_cm.resize(_nl_sys_names.size());
  _cm.resize(_nl_sys_names.size());

  _time = 0.0;
  _time_old = 0.0;
  _t_step = 0;
  _dt = 0;
  _dt_old = _dt;

  unsigned int n_threads = libMesh::n_threads();

  _real_zero.resize(n_threads, 0.);
  _scalar_zero.resize(n_threads);
  _zero.resize(n_threads);
  _phi_zero.resize(n_threads);
  _ad_zero.resize(n_threads);
  _grad_zero.resize(n_threads);
  _ad_grad_zero.resize(n_threads);
  _grad_phi_zero.resize(n_threads);
  _second_zero.resize(n_threads);
  _ad_second_zero.resize(n_threads);
  _second_phi_zero.resize(n_threads);
  _point_zero.resize(n_threads);
  _vector_zero.resize(n_threads);
  _vector_curl_zero.resize(n_threads);
  _uo_jacobian_moose_vars.resize(n_threads);

  _has_active_material_properties.resize(n_threads, 0);

  _block_mat_side_cache.resize(n_threads);
  _bnd_mat_side_cache.resize(n_threads);
  _interface_mat_side_cache.resize(n_threads);

  es().parameters.set<FEProblemBase *>("_fe_problem_base") = this;

  if (parameters.isParamSetByUser("coord_type"))
    setCoordSystem(getParam<std::vector<SubdomainName>>("block"),
                   getParam<MultiMooseEnum>("coord_type"));
  if (parameters.isParamSetByUser("rz_coord_axis"))
    setAxisymmetricCoordAxis(getParam<MooseEnum>("rz_coord_axis"));

  if (isParamValid("restart_file_base"))
  {
    std::string restart_file_base = getParam<FileNameNoExtension>("restart_file_base");

    // This check reverts to old behavior of providing "restart_file_base=" to mean
    // don't restart... BISON currently relies on this. It could probably be removed.
    // The new MooseUtils::convertLatestCheckpoint will error out if a checkpoint file
    // is not found, which I think makes sense. Which means, without this, if you
    // set "restart_file_base=", you'll get a "No checkpoint file found" error
    if (restart_file_base.size())
    {
      restart_file_base = MooseUtils::convertLatestCheckpoint(restart_file_base);
      setRestartFile(restart_file_base);
    }
  }

  // // Generally speaking, the mesh is prepared for use, and consequently remote elements are deleted
  // // well before our Problem(s) are constructed. Historically, in MooseMesh we have a bunch of
  // // needs_prepare type flags that make it so we never call prepare_for_use (and consequently
  // // delete_remote_elements) again. So the below line, historically, has had no impact. HOWEVER:
  // // I've added some code in SetupMeshCompleteAction for deleting remote elements post
  // // EquationSystems::init. If I execute that code without default ghosting, then I get > 40 MOOSE
  // // test failures, so we clearly have some simulations that are not yet covered properly by
  // // relationship managers. Until that is resolved, I am going to retain default geometric ghosting
  // if (!_default_ghosting)
  //   _mesh.getMesh().remove_ghosting_functor(_mesh.getMesh().default_ghosting());

#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  // Main app should hold the default database to handle system petsc options
  if (!_app.isUltimateMaster())
    LibmeshPetscCall(PetscOptionsCreate(&_petsc_option_data_base));
#endif

  if (!_solve)
  {
    // If we are not solving, we do not care about seeing unused petsc options
    Moose::PetscSupport::setSinglePetscOption("-options_left", "0");
    // We don't want petscSetOptions being called in solve and clearing the option that was just set
    _is_petsc_options_inserted = true;
  }
}

const MooseMesh &
FEProblemBase::mesh(bool use_displaced) const
{
  if (use_displaced && !_displaced_problem)
    mooseWarning("Displaced mesh was requested but the displaced problem does not exist. "
                 "Regular mesh will be returned");
  return ((use_displaced && _displaced_problem) ? _displaced_problem->mesh() : mesh());
}

void
FEProblemBase::createTagVectors()
{
  // add vectors and their tags to system
  auto & vectors = getParam<std::vector<std::vector<TagName>>>("extra_tag_vectors");
  for (const auto nl_sys_num : index_range(vectors))
    for (auto & vector : vectors[nl_sys_num])
    {
      auto tag = addVectorTag(vector);
      _nl[nl_sys_num]->addVector(tag, false, libMesh::GHOSTED);
    }

  auto & not_zeroed_vectors = getParam<std::vector<std::vector<TagName>>>("not_zeroed_tag_vectors");
  for (const auto nl_sys_num : index_range(not_zeroed_vectors))
    for (auto & vector : not_zeroed_vectors[nl_sys_num])
    {
      auto tag = addVectorTag(vector);
      _nl[nl_sys_num]->addVector(tag, false, GHOSTED);
      addNotZeroedVectorTag(tag);
    }

  // add matrices and their tags
  auto & matrices = getParam<std::vector<std::vector<TagName>>>("extra_tag_matrices");
  for (const auto nl_sys_num : index_range(matrices))
    for (auto & matrix : matrices[nl_sys_num])
    {
      auto tag = addMatrixTag(matrix);
      _nl[nl_sys_num]->addMatrix(tag);
    }
}

void
FEProblemBase::createTagSolutions()
{
  for (auto & vector : getParam<std::vector<TagName>>("extra_tag_solutions"))
  {
    auto tag = addVectorTag(vector, Moose::VECTOR_TAG_SOLUTION);
    for (auto & sys : _solver_systems)
      sys->addVector(tag, false, libMesh::GHOSTED);
    _aux->addVector(tag, false, libMesh::GHOSTED);
  }

  if (_previous_nl_solution_required)
  {
    // We'll populate the zeroth state of the nonlinear iterations with the current solution for
    // ease of use in doing things like copying solutions backwards. We're just storing pointers in
    // the solution states containers so populating the zeroth state does not cost us the memory of
    // a new vector
    needSolutionState(2, Moose::SolutionIterationType::Nonlinear);
  }

  auto tag = addVectorTag(Moose::SOLUTION_TAG, Moose::VECTOR_TAG_SOLUTION);
  for (auto & sys : _solver_systems)
    sys->associateVectorToTag(*sys->system().current_local_solution.get(), tag);
  _aux->associateVectorToTag(*_aux->system().current_local_solution.get(), tag);
}

void
FEProblemBase::needSolutionState(unsigned int oldest_needed,
                                 Moose::SolutionIterationType iteration_type)
{
  for (const auto i : make_range((unsigned)0, oldest_needed))
  {
    for (auto & sys : _solver_systems)
      sys->needSolutionState(i, iteration_type);
    _aux->needSolutionState(i, iteration_type);
  }
}

void
FEProblemBase::newAssemblyArray(std::vector<std::shared_ptr<SolverSystem>> & solver_systems)
{
  unsigned int n_threads = libMesh::n_threads();

  _assembly.resize(n_threads);
  for (const auto i : make_range(n_threads))
  {
    _assembly[i].resize(solver_systems.size());
    for (const auto j : index_range(solver_systems))
      _assembly[i][j] = std::make_unique<Assembly>(*solver_systems[j], i);
  }
}

void
FEProblemBase::initNullSpaceVectors(const InputParameters & parameters,
                                    std::vector<std::shared_ptr<NonlinearSystemBase>> & nls)
{
  TIME_SECTION("initNullSpaceVectors", 5, "Initializing Null Space Vectors");

  unsigned int dimNullSpace = parameters.get<unsigned int>("null_space_dimension");
  unsigned int dimTransposeNullSpace =
      parameters.get<unsigned int>("transpose_null_space_dimension");
  unsigned int dimNearNullSpace = parameters.get<unsigned int>("near_null_space_dimension");
  for (unsigned int i = 0; i < dimNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near nullspace
    // builder might march over all nodes
    for (auto & nl : nls)
      nl->addVector("NullSpace" + oss.str(), false, libMesh::GHOSTED);
  }
  _subspace_dim["NullSpace"] = dimNullSpace;
  for (unsigned int i = 0; i < dimTransposeNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near nullspace
    // builder might march over all nodes
    for (auto & nl : nls)
      nl->addVector("TransposeNullSpace" + oss.str(), false, libMesh::GHOSTED);
  }
  _subspace_dim["TransposeNullSpace"] = dimTransposeNullSpace;
  for (unsigned int i = 0; i < dimNearNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near-nullspace
    // builder might march over all semilocal nodes
    for (auto & nl : nls)
      nl->addVector("NearNullSpace" + oss.str(), false, libMesh::GHOSTED);
  }
  _subspace_dim["NearNullSpace"] = dimNearNullSpace;
}

FEProblemBase::~FEProblemBase()
{
  // Flush the Console stream, the underlying call to Console::mooseConsole
  // relies on a call to Output::checkInterval that has references to
  // _time, etc. If it is not flushed here memory problems arise if you have
  // an unflushed stream and start destructing things.
  _console << std::flush;

  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    _zero[i].release();
    _phi_zero[i].release();
    _scalar_zero[i].release();
    _grad_zero[i].release();
    _grad_phi_zero[i].release();
    _second_zero[i].release();
    _second_phi_zero[i].release();
    _vector_zero[i].release();
    _vector_curl_zero[i].release();
    _ad_zero[i].release();
    _ad_grad_zero[i].release();
    _ad_second_zero[i].release();
  }

#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  if (!_app.isUltimateMaster())
  {
    auto ierr = PetscOptionsDestroy(&_petsc_option_data_base);
    // Don't throw on destruction
    CHKERRABORT(this->comm().get(), ierr);
  }
#endif
}

void
FEProblemBase::setCoordSystem(const std::vector<SubdomainName> & blocks,
                              const MultiMooseEnum & coord_sys)
{
  TIME_SECTION("setCoordSystem", 5, "Setting Coordinate System");
  _mesh.setCoordSystem(blocks, coord_sys);
}

void
FEProblemBase::setAxisymmetricCoordAxis(const MooseEnum & rz_coord_axis)
{
  _mesh.setAxisymmetricCoordAxis(rz_coord_axis);
}

const ConstElemRange &
FEProblemBase::getEvaluableElementRange()
{
  if (!_evaluable_local_elem_range)
  {
    std::vector<const DofMap *> dof_maps(es().n_systems());
    for (const auto i : make_range(es().n_systems()))
    {
      const auto & sys = es().get_system(i);
      dof_maps[i] = &sys.get_dof_map();
    }
    _evaluable_local_elem_range =
        std::make_unique<ConstElemRange>(_mesh.getMesh().multi_evaluable_elements_begin(dof_maps),
                                         _mesh.getMesh().multi_evaluable_elements_end(dof_maps));
  }
  return *_evaluable_local_elem_range;
}

const ConstElemRange &
FEProblemBase::getNonlinearEvaluableElementRange()
{
  if (!_nl_evaluable_local_elem_range)
  {
    std::vector<const DofMap *> dof_maps(_nl.size());
    for (const auto i : index_range(dof_maps))
      dof_maps[i] = &_nl[i]->dofMap();
    _nl_evaluable_local_elem_range =
        std::make_unique<ConstElemRange>(_mesh.getMesh().multi_evaluable_elements_begin(dof_maps),
                                         _mesh.getMesh().multi_evaluable_elements_end(dof_maps));
  }

  return *_nl_evaluable_local_elem_range;
}

void
FEProblemBase::initialSetup()
{
  TIME_SECTION("initialSetup", 2, "Performing Initial Setup");

  SubProblem::initialSetup();

  if (_app.isRecovering() + _app.isRestarting() + bool(_app.getExReaderForRestart()) > 1)
    mooseError("Checkpoint recovery and restart and exodus restart are all mutually exclusive.");

  if (_skip_exception_check)
    mooseWarning("MOOSE may fail to catch an exception when the \"skip_exception_check\" parameter "
                 "is used. If you receive a terse MPI error during execution, remove this "
                 "parameter and rerun your simulation");

  // set state flag indicating that we are in or beyond initialSetup.
  // This can be used to throw errors in methods that _must_ be called at construction time.
  _started_initial_setup = true;
  setCurrentExecuteOnFlag(EXEC_INITIAL);

  // Setup the solution states (current, old, etc) in each system based on
  // its default and the states requested of each of its variables
  for (const auto i : index_range(_solver_systems))
  {
    _solver_systems[i]->initSolutionState();
    if (getDisplacedProblem())
      getDisplacedProblem()->solverSys(i).initSolutionState();
  }
  _aux->initSolutionState();
  if (getDisplacedProblem())
    getDisplacedProblem()->auxSys().initSolutionState();

  // always execute to get the max number of DoF per element and node needed to initialize phi_zero
  // variables
  dof_id_type global_max_var_n_dofs_per_elem = 0;
  for (const auto i : index_range(_solver_systems))
  {
    auto & sys = *_solver_systems[i];
    dof_id_type max_var_n_dofs_per_elem;
    dof_id_type max_var_n_dofs_per_node;
    {
      TIME_SECTION("computingMaxDofs", 3, "Computing Max Dofs Per Element");

      MaxVarNDofsPerElem mvndpe(*this, sys);
      Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), mvndpe);
      max_var_n_dofs_per_elem = mvndpe.max();
      _communicator.max(max_var_n_dofs_per_elem);

      MaxVarNDofsPerNode mvndpn(*this, sys);
      Threads::parallel_reduce(*_mesh.getLocalNodeRange(), mvndpn);
      max_var_n_dofs_per_node = mvndpn.max();
      _communicator.max(max_var_n_dofs_per_node);
      global_max_var_n_dofs_per_elem =
          std::max(global_max_var_n_dofs_per_elem, max_var_n_dofs_per_elem);
    }

    {
      TIME_SECTION("assignMaxDofs", 5, "Assigning Maximum Dofs Per Elem");

      sys.assignMaxVarNDofsPerElem(max_var_n_dofs_per_elem);
      auto displaced_problem = getDisplacedProblem();
      if (displaced_problem)
        displaced_problem->solverSys(i).assignMaxVarNDofsPerElem(max_var_n_dofs_per_elem);

      sys.assignMaxVarNDofsPerNode(max_var_n_dofs_per_node);
      if (displaced_problem)
        displaced_problem->solverSys(i).assignMaxVarNDofsPerNode(max_var_n_dofs_per_node);
    }
  }

  {
    TIME_SECTION("resizingVarValues", 5, "Resizing Variable Values");

    for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      _phi_zero[tid].resize(global_max_var_n_dofs_per_elem, std::vector<Real>(getMaxQps(), 0.));
      _grad_phi_zero[tid].resize(global_max_var_n_dofs_per_elem,
                                 std::vector<RealGradient>(getMaxQps(), RealGradient(0.)));
      _second_phi_zero[tid].resize(global_max_var_n_dofs_per_elem,
                                   std::vector<RealTensor>(getMaxQps(), RealTensor(0.)));
    }
  }

  // Set up stateful material property redistribution, if we suspect
  // it may be necessary later.
  addAnyRedistributers();

  if (_app.isRestarting() || _app.isRecovering() || _force_restart)
  {
    // Only load all of the vectors if we're recovering
    _req.set().setLoadAllVectors(_app.isRecovering());

    // This forces stateful material property loading to be an exact one-to-one match
    if (_app.isRecovering())
      for (auto props : {&_material_props, &_bnd_material_props, &_neighbor_material_props})
        props->setRecovering();

    TIME_SECTION("restore", 3, "Restoring from backup");

    // We could have a cached backup when this app is a sub-app and has been given a Backup
    if (!_app.hasInitialBackup())
      _app.restore(_app.restartFolderBase(_app.getRestartRecoverFileBase()), _app.isRestarting());
    else
      _app.restoreFromInitialBackup(_app.isRestarting());

    /**
     * If this is a restart run, the user may want to override the start time, which we already set
     * in the constructor. "_time" however will have been "restored" from the restart file. We need
     * to honor the original request of the developer now that the restore has been completed.
     */
    if (_app.isRestarting())
    {
      if (_app.hasStartTime())
        _time = _time_old = _app.getStartTime();
      else
        _time_old = _time;
    }
  }
  else
  {
    libMesh::ExodusII_IO * reader = _app.getExReaderForRestart();

    if (reader)
    {
      TIME_SECTION("copyingFromExodus", 3, "Copying Variables From Exodus");

      for (auto & sys : _solver_systems)
        sys->copyVars(*reader);
      _aux->copyVars(*reader);
    }
    else
    {
      if (_solver_systems[0]->hasVarCopy() || _aux->hasVarCopy())
        mooseError("Need Exodus reader to restart variables but the reader is not available\n"
                   "Use either FileMesh with an Exodus mesh file or FileMeshGenerator with an "
                   "Exodus mesh file and with use_for_exodus_restart equal to true");
    }
  }

  // Perform output related setups
  _app.getOutputWarehouse().initialSetup();

  // Flush all output to _console that occur during construction and initialization of objects
  _app.getOutputWarehouse().mooseConsole();

  // Build Refinement and Coarsening maps for stateful material projections if necessary
  if ((_adaptivity.isOn() || _num_grid_steps) &&
      (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties() ||
       _neighbor_material_props.hasStatefulProperties()))
  {
    if (_has_internal_edge_residual_objects)
      mooseError("Stateful neighbor material properties do not work with mesh adaptivity");

    _mesh.buildRefinementAndCoarseningMaps(_assembly[0][0].get());
  }

  if (!_app.isRecovering())
  {
    /**
     * If we are not recovering but we are doing restart (_app.getExodusFileRestart() == true) with
     * additional uniform refinements. We have to delay the refinement until this point
     * in time so that the equation systems are initialized and projections can be performed.
     */
    if (_mesh.uniformRefineLevel() > 0 && _app.getExodusFileRestart())
    {
      if (!_app.isUltimateMaster())
        mooseError(
            "Doing extra refinements when restarting is NOT supported for sub-apps of a MultiApp");

      adaptivity().uniformRefineWithProjection();
    }
  }

  unsigned int n_threads = libMesh::n_threads();

  // Convergence initial setup
  {
    TIME_SECTION("convergenceInitialSetup", 5, "Initializing Convergence objects");

    for (THREAD_ID tid = 0; tid < n_threads; tid++)
      _convergences.initialSetup(tid);
  }

  // UserObject initialSetup
  std::set<std::string> depend_objects_ic = _ics.getDependObjects();
  std::set<std::string> depend_objects_aux = _aux->getDependObjects();

  // This replaces all prior updateDependObjects calls on the old user object warehouses.
  TheWarehouse::Query uo_query = theWarehouse().query().condition<AttribSystem>("UserObject");
  std::vector<UserObject *> userobjs;
  uo_query.queryInto(userobjs);
  groupUserObjects(
      theWarehouse(), getAuxiliarySystem(), _app.getExecuteOnEnum(), userobjs, depend_objects_ic);

  std::map<int, std::vector<UserObject *>> group_userobjs;
  for (auto obj : userobjs)
    group_userobjs[obj->getParam<int>("execution_order_group")].push_back(obj);

  for (auto & [group, objs] : group_userobjs)
    for (auto obj : objs)
      obj->initialSetup();

  // check if jacobian calculation is done in userobject
  for (THREAD_ID tid = 0; tid < n_threads; ++tid)
    checkUserObjectJacobianRequirement(tid);

  // Check whether nonlocal couling is required or not
  checkNonlocalCoupling();
  if (_requires_nonlocal_coupling)
    setVariableAllDoFMap(_uo_jacobian_moose_vars[0]);

  {
    TIME_SECTION("initializingFunctions", 5, "Initializing Functions");

    // Call the initialSetup methods for functions
    for (THREAD_ID tid = 0; tid < n_threads; tid++)
    {
      reinitScalars(tid); // initialize scalars so they are properly sized for use as input into
                          // ParsedFunctions
      _functions.initialSetup(tid);
    }
  }

  {
    TIME_SECTION("initializingRandomObjects", 5, "Initializing Random Objects");

    // Random interface objects
    for (const auto & it : _random_data_objects)
      it.second->updateSeeds(EXEC_INITIAL);
  }

  if (!_app.isRecovering())
  {
    computeUserObjects(EXEC_INITIAL, Moose::PRE_IC);

    {
      TIME_SECTION("ICinitialSetup", 5, "Setting Up Initial Conditions");

      for (THREAD_ID tid = 0; tid < n_threads; tid++)
        _ics.initialSetup(tid);

      _scalar_ics.initialSetup();
    }

    projectSolution();
  }

  // Materials
  if (_all_materials.hasActiveObjects(0))
  {
    TIME_SECTION("materialInitialSetup", 3, "Setting Up Materials");

    for (THREAD_ID tid = 0; tid < n_threads; tid++)
    {
      // Sort the Material objects, these will be actually computed by MOOSE in reinit methods.
      _materials.sort(tid);
      _interface_materials.sort(tid);

      // Call initialSetup on all material objects
      _all_materials.initialSetup(tid);

      // Discrete materials may insert additional dependencies on materials during the initial
      // setup. Therefore we resolve the dependencies once more, now with the additional
      // dependencies due to discrete materials.
      if (_discrete_materials.hasActiveObjects())
      {
        _materials.sort(tid);
        _interface_materials.sort(tid);
      }
    }

    {
      TIME_SECTION("computingInitialStatefulProps", 3, "Computing Initial Material Values");

      initElementStatefulProps(*_mesh.getActiveLocalElementRange(), true);

      if (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties() ||
          _neighbor_material_props.hasStatefulProperties())
        _has_initialized_stateful = true;
    }
  }

  // setRestartInPlace() is set because the property maps have now been setup and we can
  // dataLoad() them directly in place
  // setRecovering() is set because from now on we require a one-to-one mapping of
  // stateful properties because we shouldn't be declaring any more
  for (auto props : {&_material_props, &_bnd_material_props, &_neighbor_material_props})
  {
    props->setRestartInPlace();
    props->setRecovering();
  }

  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _internal_side_indicators.initialSetup(tid);
    _indicators.initialSetup(tid);
    _markers.sort(tid);
    _markers.initialSetup(tid);
  }

#ifdef LIBMESH_ENABLE_AMR

  if (!_app.isRecovering())
  {
    unsigned int n = adaptivity().getInitialSteps();
    if (n && !_app.isUltimateMaster() && _app.isRestarting())
      mooseError("Cannot perform initial adaptivity during restart on sub-apps of a MultiApp!");

    initialAdaptMesh();
  }

#endif // LIBMESH_ENABLE_AMR

  if (!_app.isRecovering() && !_app.isRestarting())
  {
    // During initial setup the solution is copied to the older solution states (old, older, etc)
    copySolutionsBackwards();

    // Check if there are old state initial conditions
    auto ics = _ics.getActiveObjects();
    auto fv_ics = _fv_ics.getActiveObjects();
    auto scalar_ics = _scalar_ics.getActiveObjects();
    unsigned short ic_state_max = 0;

    auto findMax = [&ic_state_max](const auto & obj_list)
    {
      for (auto ic : obj_list.getActiveObjects())
        ic_state_max = std::max(ic_state_max, ic->getState());
    };
    findMax(_ics);
    findMax(_fv_ics);
    findMax(_scalar_ics);

    // if there are old state ICs, compute them and write to old states accordingly
    if (ic_state_max > 0)
    {
      // state 0 copy (we'll overwrite current state when evaluating ICs and need to restore it once
      // we're done with the old/older state ICs)
      std::vector<std::unique_ptr<NumericVector<Real>>> state0_sys_buffers(_solver_systems.size());
      std::unique_ptr<NumericVector<Real>> state0_aux_buffer;

      // save state 0
      for (const auto i : index_range(_solver_systems))
        state0_sys_buffers[i] = _solver_systems[i]->solutionState(0).clone();

      state0_aux_buffer = _aux->solutionState(0).clone();

      // compute old state ICs
      for (_current_ic_state = 1; _current_ic_state <= ic_state_max; _current_ic_state++)
      {
        projectSolution();

        for (auto & sys : _solver_systems)
          sys->solutionState(_current_ic_state) = sys->solutionState(0);

        _aux->solutionState(_current_ic_state) = _aux->solutionState(0);
      }
      _current_ic_state = 0;

      // recover state 0
      for (const auto i : index_range(_solver_systems))
      {
        _solver_systems[i]->solutionState(0) = *state0_sys_buffers[i];
        _solver_systems[i]->solutionState(0).close();
        _solver_systems[i]->update();
      }
      _aux->solutionState(0) = *state0_aux_buffer;
      _aux->solutionState(0).close();
      _aux->update();
    }
  }

  if (!_app.isRecovering())
  {
    if (haveXFEM())
      updateMeshXFEM();
  }

  // Call initialSetup on the solver systems
  for (auto & sys : _solver_systems)
    sys->initialSetup();

  // Auxilary variable initialSetup calls
  _aux->initialSetup();

  if (_displaced_problem)
    // initialSetup for displaced systems
    _displaced_problem->initialSetup();

  for (auto & sys : _solver_systems)
    sys->setSolution(*(sys->system().current_local_solution.get()));

  // Update the nearest node searches (has to be called after the problem is all set up)
  // We do this here because this sets up the Element's DoFs to ghost
  updateGeomSearch(GeometricSearchData::NEAREST_NODE);

  _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);
  if (_displaced_mesh)
    _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);

  // We need to move the mesh in order to build a map between mortar secondary and primary
  // interfaces. This map will then be used by the AgumentSparsityOnInterface ghosting functor to
  // know which dofs we need ghosted when we call EquationSystems::reinit
  if (_displaced_problem && _mortar_data.hasDisplacedObjects())
    _displaced_problem->updateMesh();

  // Possibly reinit one more time to get ghosting correct
  reinitBecauseOfGhostingOrNewGeomObjects();

  if (_displaced_mesh)
    _displaced_problem->updateMesh();

  updateGeomSearch(); // Call all of the rest of the geometric searches

  for (auto & sys : _solver_systems)
  {
    const auto & tis = sys->getTimeIntegrators();

    {
      TIME_SECTION("timeIntegratorInitialSetup", 5, "Initializing Time Integrator");
      for (auto & ti : tis)
        ti->initialSetup();
    }
  }

  // HUGE NOTE: MultiApp initialSetup() MUST... I repeat MUST be _after_ main-app restartable data
  // has been restored

  // Call initialSetup on the MultiApps
  if (_multi_apps.hasObjects())
  {
    TIME_SECTION("initialSetupMultiApps", 2, "Initializing MultiApps", false);
    _multi_apps.initialSetup();
  }

  // Call initialSetup on the transfers
  {
    TIME_SECTION("initialSetupTransfers", 2, "Initializing Transfers");

    _transfers.initialSetup();

    // Call initialSetup on the MultiAppTransfers to be executed on TO_MULTIAPP
    const auto & to_multi_app_objects = _to_multi_app_transfers.getActiveObjects();
    for (const auto & transfer : to_multi_app_objects)
    {
      transfer->setCurrentDirection(Transfer::DIRECTION::TO_MULTIAPP);
      transfer->initialSetup();
    }

    // Call initialSetup on the MultiAppTransfers to be executed on FROM_MULTIAPP
    const auto & from_multi_app_objects = _from_multi_app_transfers.getActiveObjects();
    for (const auto & transfer : from_multi_app_objects)
    {
      transfer->setCurrentDirection(Transfer::DIRECTION::FROM_MULTIAPP);
      transfer->initialSetup();
    }

    // Call initialSetup on the MultiAppTransfers to be executed on BETWEEN_MULTIAPP
    const auto & between_multi_app_objects = _between_multi_app_transfers.getActiveObjects();
    for (const auto & transfer : between_multi_app_objects)
    {
      transfer->setCurrentDirection(Transfer::DIRECTION::BETWEEN_MULTIAPP);
      transfer->initialSetup();
    }
  }

  if (_boundary_restricted_node_integrity_check)
  {
    TIME_SECTION("BoundaryRestrictedNodeIntegrityCheck", 5);

    // check that variables are defined along boundaries of boundary restricted nodal objects
    ConstBndNodeRange & bnd_nodes = *mesh().getBoundaryNodeRange();
    BoundaryNodeIntegrityCheckThread bnict(*this, uo_query);
    Threads::parallel_reduce(bnd_nodes, bnict);

    // Nodal bcs aren't threaded
    const auto & node_to_elem_map = _mesh.nodeToActiveSemilocalElemMap();
    for (const auto & bnode : bnd_nodes)
    {
      const auto boundary_id = bnode->_bnd_id;
      const Node * const node = bnode->_node;

      if (node->processor_id() != this->processor_id())
        continue;

      // Only check vertices. Variables may not be defined on non-vertex nodes (think first order
      // Lagrange on a second order mesh) and user-code can often handle that
      const Elem * const an_elem =
          _mesh.getMesh().elem_ptr(libmesh_map_find(node_to_elem_map, node->id()).front());
      if (!an_elem->is_vertex(an_elem->get_node_index(node)))
        continue;

      const auto & bnd_name = _mesh.getBoundaryName(boundary_id);

      for (auto & nl : _nl)
      {
        const auto & nodal_bcs = nl->getNodalBCWarehouse();
        if (!nodal_bcs.hasBoundaryObjects(boundary_id, 0))
          continue;

        const auto & bnd_objects = nodal_bcs.getBoundaryObjects(boundary_id, 0);
        for (const auto & bnd_object : bnd_objects)
          // Skip if this object uses geometric search because coupled variables may be defined on
          // paired boundaries instead of the boundary this node is on
          if (!bnd_object->requiresGeometricSearch() &&
              bnd_object->checkVariableBoundaryIntegrity())
          {
            std::set<MooseVariableFieldBase *> vars_to_omit = {
                &static_cast<MooseVariableFieldBase &>(
                    const_cast<MooseVariableBase &>(bnd_object->variable()))};

            boundaryIntegrityCheckError(
                *bnd_object, bnd_object->checkAllVariables(*node, vars_to_omit), bnd_name);
          }
      }
    }
  }

  if (_boundary_restricted_elem_integrity_check)
  {
    TIME_SECTION("BoundaryRestrictedElemIntegrityCheck", 5);

    // check that variables are defined along boundaries of boundary restricted elemental objects
    ConstBndElemRange & bnd_elems = *mesh().getBoundaryElementRange();
    BoundaryElemIntegrityCheckThread beict(*this, uo_query);
    Threads::parallel_reduce(bnd_elems, beict);
  }

  if (!_app.isRecovering())
  {
    execTransfers(EXEC_INITIAL);

    bool converged = execMultiApps(EXEC_INITIAL);
    if (!converged)
      mooseError("failed to converge initial MultiApp");

    // We'll backup the Multiapp here
    backupMultiApps(EXEC_INITIAL);

    for (THREAD_ID tid = 0; tid < n_threads; tid++)
      reinitScalars(tid);

    execute(EXEC_INITIAL);

    // The FEProblemBase::execute method doesn't call all the systems on EXEC_INITIAL, but it does
    // set/unset the current flag. Therefore, this resets the current flag to EXEC_INITIAL so that
    // subsequent calls (e.g., executeControls) have the proper flag.
    setCurrentExecuteOnFlag(EXEC_INITIAL);
  }

  // Here we will initialize the stateful properties once more since they may have been updated
  // during initialSetup by calls to computeProperties.
  //
  // It's really bad that we don't allow this during restart.  It means that we can't add new
  // stateful materials
  // during restart.  This is only happening because this _has_ to be below initial userobject
  // execution.
  // Otherwise this could be done up above... _before_ restoring restartable data... which would
  // allow you to have
  // this happen during restart.  I honestly have no idea why this has to happen after initial user
  // object computation.
  // THAT is something we should fix... so I've opened this ticket: #5804
  if (!_app.isRecovering() && !_app.isRestarting() &&
      (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties() ||
       _neighbor_material_props.hasStatefulProperties()))
  {
    TIME_SECTION("computeMaterials", 2, "Computing Initial Material Properties");

    initElementStatefulProps(*_mesh.getActiveLocalElementRange(), true);
  }

  // Control Logic
  executeControls(EXEC_INITIAL);

  // Scalar variables need to reinited for the initial conditions to be available for output
  for (unsigned int tid = 0; tid < n_threads; tid++)
    reinitScalars(tid);

  if (_displaced_mesh)
    _displaced_problem->syncSolutions();

  // Writes all calls to _console from initialSetup() methods
  _app.getOutputWarehouse().mooseConsole();

  if (_requires_nonlocal_coupling)
  {
    setNonlocalCouplingMatrix();
    for (THREAD_ID tid = 0; tid < n_threads; ++tid)
      for (auto & assembly : _assembly[tid])
        assembly->initNonlocalCoupling();
  }

  {
    TIME_SECTION("lineSearchInitialSetup", 5, "Initializing Line Search");

    if (_line_search)
      _line_search->initialSetup();
  }

  // Perform Reporter get/declare check
  _reporter_data.check();

  // We do this late to allow objects to get late restartable data
  if (_app.isRestarting() || _app.isRecovering() || _force_restart)
    _app.finalizeRestore();

  setCurrentExecuteOnFlag(EXEC_NONE);
}

void
FEProblemBase::checkDuplicatePostprocessorVariableNames()
{
  for (const auto & pp : _reporter_data.getPostprocessorNames())
    if (hasScalarVariable(pp))
      mooseError("Postprocessor \"" + pp +
                 "\" has the same name as a scalar variable in the system.");
}

void
FEProblemBase::timestepSetup()
{
  SubProblem::timestepSetup();

  if (_t_step > 1 && _num_grid_steps)
  {
    libMesh::MeshRefinement mesh_refinement(_mesh);
    std::unique_ptr<libMesh::MeshRefinement> displaced_mesh_refinement(nullptr);
    if (_displaced_mesh)
      displaced_mesh_refinement = std::make_unique<libMesh::MeshRefinement>(*_displaced_mesh);

    for (MooseIndex(_num_grid_steps) i = 0; i < _num_grid_steps; ++i)
    {
      if (_displaced_problem)
        // If the DisplacedProblem is active, undisplace the DisplacedMesh in preparation for
        // refinement.  We can't safely refine the DisplacedMesh directly, since the Hilbert keys
        // computed on the inconsistenly-displaced Mesh are different on different processors,
        // leading to inconsistent Hilbert keys.  We must do this before the undisplaced Mesh is
        // coarsensed, so that the element and node numbering is still consistent. We also have to
        // make sure this is done during every step of coarsening otherwise different partitions
        // will be generated for the reference and displaced meshes (even for replicated)
        _displaced_problem->undisplaceMesh();

      mesh_refinement.uniformly_coarsen();
      if (_displaced_mesh)
        displaced_mesh_refinement->uniformly_coarsen();

      // Mark this as an intermediate change because we do not yet want to reinit_systems. E.g. we
      // need things to happen in the following order for the undisplaced problem:
      // u1) EquationSystems::reinit_solutions. This will restrict the solution vectors and then
      //     contract the mesh
      // u2) MooseMesh::meshChanged. This will update the node/side lists and other
      //     things which needs to happen after the contraction
      // u3) GeometricSearchData::reinit. Once the node/side lists are updated we can perform our
      //     geometric searches which will aid in determining sparsity patterns
      //
      // We do these things for the displaced problem (if it exists)
      // d1) EquationSystems::reinit. Restrict the displaced problem vector copies and then contract
      //     the mesh. It's safe to do a full reinit with the displaced because there are no
      //     matrices that sparsity pattern calculations will be conducted for
      // d2) MooseMesh::meshChanged. This will update the node/side lists and other
      //     things which needs to happen after the contraction
      // d3) UpdateDisplacedMeshThread::operator(). Re-displace the mesh using the *displaced*
      //     solution vector copy because we don't know the state of the reference solution vector.
      //     It's safe to use the displaced copy because we are outside of a non-linear solve,
      //     and there is no concern about differences between solution and current_local_solution
      // d4) GeometricSearchData::reinit. With the node/side lists updated and the mesh
      //     re-displaced, we can perform our geometric searches, which will aid in determining the
      //     sparsity pattern of the matrix held by the libMesh::ImplicitSystem held by the
      //     NonlinearSystem held by this
      meshChangedHelper(/*intermediate_change=*/true);
    }

    // u4) Now that all the geometric searches have been done (both undisplaced and displaced),
    //     we're ready to update the sparsity pattern
    es().reinit_systems();
  }

  if (_line_search)
    _line_search->timestepSetup();

  // Random interface objects
  for (const auto & it : _random_data_objects)
    it.second->updateSeeds(EXEC_TIMESTEP_BEGIN);

  unsigned int n_threads = libMesh::n_threads();
  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _all_materials.timestepSetup(tid);
    _functions.timestepSetup(tid);
  }

  _aux->timestepSetup();
  for (auto & sys : _solver_systems)
    sys->timestepSetup();

  if (_displaced_problem)
    // timestepSetup for displaced systems
    _displaced_problem->timestepSetup();

  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _internal_side_indicators.timestepSetup(tid);
    _indicators.timestepSetup(tid);
    _markers.timestepSetup(tid);
  }

  std::vector<UserObject *> userobjs;
  theWarehouse().query().condition<AttribSystem>("UserObject").queryIntoUnsorted(userobjs);
  for (auto obj : userobjs)
    obj->timestepSetup();

  // Timestep setup of output objects
  _app.getOutputWarehouse().timestepSetup();

  if (_requires_nonlocal_coupling)
    if (_nonlocal_kernels.hasActiveObjects() || _nonlocal_integrated_bcs.hasActiveObjects())
      _has_nonlocal_coupling = true;
}

unsigned int
FEProblemBase::getMaxQps() const
{
  if (_max_qps == std::numeric_limits<unsigned int>::max())
    mooseError("Max QPS uninitialized");
  return _max_qps;
}

Order
FEProblemBase::getMaxScalarOrder() const
{
  return _max_scalar_order;
}

void
FEProblemBase::checkNonlocalCoupling()
{
  TIME_SECTION("checkNonlocalCoupling", 5, "Checking Nonlocal Coupling");

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    for (auto & nl : _nl)
    {
      const auto & all_kernels = nl->getKernelWarehouse();
      const auto & kernels = all_kernels.getObjects(tid);
      for (const auto & kernel : kernels)
      {
        std::shared_ptr<NonlocalKernel> nonlocal_kernel =
            std::dynamic_pointer_cast<NonlocalKernel>(kernel);
        if (nonlocal_kernel)
        {
          if (_calculate_jacobian_in_uo)
            _requires_nonlocal_coupling = true;
          _nonlocal_kernels.addObject(kernel, tid);
        }
      }
      const MooseObjectWarehouse<IntegratedBCBase> & all_integrated_bcs =
          nl->getIntegratedBCWarehouse();
      const auto & integrated_bcs = all_integrated_bcs.getObjects(tid);
      for (const auto & integrated_bc : integrated_bcs)
      {
        std::shared_ptr<NonlocalIntegratedBC> nonlocal_integrated_bc =
            std::dynamic_pointer_cast<NonlocalIntegratedBC>(integrated_bc);
        if (nonlocal_integrated_bc)
        {
          if (_calculate_jacobian_in_uo)
            _requires_nonlocal_coupling = true;
          _nonlocal_integrated_bcs.addObject(integrated_bc, tid);
        }
      }
    }
}

void
FEProblemBase::checkUserObjectJacobianRequirement(THREAD_ID tid)
{
  std::set<const MooseVariableFEBase *> uo_jacobian_moose_vars;
  {
    std::vector<ShapeElementUserObject *> objs;
    theWarehouse()
        .query()
        .condition<AttribInterfaces>(Interfaces::ShapeElementUserObject)
        .condition<AttribThread>(tid)
        .queryInto(objs);

    for (const auto & uo : objs)
    {
      _calculate_jacobian_in_uo = uo->computeJacobianFlag();
      const auto & mv_deps = uo->jacobianMooseVariables();
      uo_jacobian_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }
  }
  {
    std::vector<ShapeSideUserObject *> objs;
    theWarehouse()
        .query()
        .condition<AttribInterfaces>(Interfaces::ShapeSideUserObject)
        .condition<AttribThread>(tid)
        .queryInto(objs);
    for (const auto & uo : objs)
    {
      _calculate_jacobian_in_uo = uo->computeJacobianFlag();
      const auto & mv_deps = uo->jacobianMooseVariables();
      uo_jacobian_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }
  }

  _uo_jacobian_moose_vars[tid].assign(uo_jacobian_moose_vars.begin(), uo_jacobian_moose_vars.end());
  std::sort(
      _uo_jacobian_moose_vars[tid].begin(), _uo_jacobian_moose_vars[tid].end(), sortMooseVariables);
}

void
FEProblemBase::setVariableAllDoFMap(const std::vector<const MooseVariableFEBase *> & moose_vars)
{
  for (unsigned int i = 0; i < moose_vars.size(); ++i)
  {
    VariableName var_name = moose_vars[i]->name();
    auto & sys = _solver_systems[moose_vars[i]->sys().number()];
    sys->setVariableGlobalDoFs(var_name);
    _var_dof_map[var_name] = sys->getVariableGlobalDoFs();
  }
}

void
FEProblemBase::prepare(const Elem * elem, const THREAD_ID tid)
{
  for (const auto i : index_range(_solver_systems))
  {
    _assembly[tid][i]->reinit(elem);
    _solver_systems[i]->prepare(tid);

    if (i < _num_nl_sys)
    {
      // This method is called outside of residual/Jacobian callbacks during initial condition
      // evaluation
      if ((!_has_jacobian || !_const_jacobian) && currentlyComputingJacobian())
        _assembly[tid][i]->prepareJacobianBlock();
      _assembly[tid][i]->prepareResidual();
      if (_has_nonlocal_coupling && currentlyComputingJacobian())
        _assembly[tid][i]->prepareNonlocal();
    }
  }
  _aux->prepare(tid);

  if (_displaced_problem &&
      // _reinit_displaced_neighbor applies to interface type objects which will do computations
      // based on both elem and neighbor. Consequently, despite what you might think by its name, we
      // must make sure we prepare the displaced elem
      (_reinit_displaced_elem || _reinit_displaced_face || _reinit_displaced_neighbor))
  {
    _displaced_problem->prepare(_displaced_mesh->elemPtr(elem->id()), tid);
    if (_has_nonlocal_coupling)
      _displaced_problem->prepareNonlocal(tid);
  }
}

void
FEProblemBase::prepareFace(const Elem * elem, const THREAD_ID tid)
{
  for (auto & nl : _nl)
    nl->prepareFace(tid, true);
  _aux->prepareFace(tid, false);

  if (_displaced_problem && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepareFace(_displaced_mesh->elemPtr(elem->id()), tid);
}

void
FEProblemBase::prepare(const Elem * elem,
                       unsigned int ivar,
                       unsigned int jvar,
                       const std::vector<dof_id_type> & dof_indices,
                       const THREAD_ID tid)
{
  for (const auto i : index_range(_nl))
  {
    _assembly[tid][i]->reinit(elem);
    _nl[i]->prepare(tid);
  }

  _aux->prepare(tid);
  const auto current_nl_sys_num = _current_nl_sys->number();
  _assembly[tid][current_nl_sys_num]->prepareBlock(ivar, jvar, dof_indices);
  if (_has_nonlocal_coupling)
    if (_nonlocal_cm[current_nl_sys_num](ivar, jvar) != 0)
    {
      MooseVariableFEBase & jv = _current_nl_sys->getVariable(tid, jvar);
      _assembly[tid][current_nl_sys_num]->prepareBlockNonlocal(
          ivar, jvar, dof_indices, jv.allDofIndices());
    }

  if (_displaced_problem && (_reinit_displaced_elem || _reinit_displaced_face))
  {
    _displaced_problem->prepare(_displaced_mesh->elemPtr(elem->id()), ivar, jvar, dof_indices, tid);
    if (_has_nonlocal_coupling)
      if (_nonlocal_cm[current_nl_sys_num](ivar, jvar) != 0)
      {
        MooseVariableFEBase & jv = _current_nl_sys->getVariable(tid, jvar);
        _displaced_problem->prepareBlockNonlocal(ivar, jvar, dof_indices, jv.allDofIndices(), tid);
      }
  }
}

void
FEProblemBase::setCurrentSubdomainID(const Elem * elem, const THREAD_ID tid)
{
  SubdomainID did = elem->subdomain_id();
  for (const auto i : index_range(_solver_systems))
  {
    _assembly[tid][i]->setCurrentSubdomainID(did);
    if (_displaced_problem &&
        (_reinit_displaced_elem || _reinit_displaced_face || _reinit_displaced_neighbor))
      _displaced_problem->assembly(tid, i).setCurrentSubdomainID(did);
  }
}

void
FEProblemBase::setNeighborSubdomainID(const Elem * elem, unsigned int side, const THREAD_ID tid)
{
  SubdomainID did = elem->neighbor_ptr(side)->subdomain_id();
  for (const auto i : index_range(_nl))
  {
    _assembly[tid][i]->setCurrentNeighborSubdomainID(did);
    if (_displaced_problem &&
        (_reinit_displaced_elem || _reinit_displaced_face || _reinit_displaced_neighbor))
      _displaced_problem->assembly(tid, i).setCurrentNeighborSubdomainID(did);
  }
}

void
FEProblemBase::setNeighborSubdomainID(const Elem * elem, const THREAD_ID tid)
{
  SubdomainID did = elem->subdomain_id();
  for (const auto i : index_range(_nl))
  {
    _assembly[tid][i]->setCurrentNeighborSubdomainID(did);
    if (_displaced_problem &&
        (_reinit_displaced_elem || _reinit_displaced_face || _reinit_displaced_neighbor))
      _displaced_problem->assembly(tid, i).setCurrentNeighborSubdomainID(did);
  }
}

void
FEProblemBase::prepareAssembly(const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->prepare();
  if (_has_nonlocal_coupling)
    _assembly[tid][_current_nl_sys->number()]->prepareNonlocal();

  if (_displaced_problem && (_reinit_displaced_elem || _reinit_displaced_face))
  {
    _displaced_problem->prepareAssembly(tid);
    if (_has_nonlocal_coupling)
      _displaced_problem->prepareNonlocal(tid);
  }
}

void
FEProblemBase::addResidual(const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->addResidual(Assembly::GlobalDataKey{},
                                                         currentResidualVectorTags());

  if (_displaced_problem)
    _displaced_problem->addResidual(tid);
}

void
FEProblemBase::addResidualNeighbor(const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->addResidualNeighbor(Assembly::GlobalDataKey{},
                                                                 currentResidualVectorTags());

  if (_displaced_problem)
    _displaced_problem->addResidualNeighbor(tid);
}

void
FEProblemBase::addResidualLower(const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->addResidualLower(Assembly::GlobalDataKey{},
                                                              currentResidualVectorTags());

  if (_displaced_problem)
    _displaced_problem->addResidualLower(tid);
}

void
FEProblemBase::addResidualScalar(const THREAD_ID tid /* = 0*/)
{
  _assembly[tid][_current_nl_sys->number()]->addResidualScalar(Assembly::GlobalDataKey{},
                                                               currentResidualVectorTags());
}

void
FEProblemBase::cacheResidual(const THREAD_ID tid)
{
  SubProblem::cacheResidual(tid);
  if (_displaced_problem)
    _displaced_problem->cacheResidual(tid);
}

void
FEProblemBase::cacheResidualNeighbor(const THREAD_ID tid)
{
  SubProblem::cacheResidualNeighbor(tid);
  if (_displaced_problem)
    _displaced_problem->cacheResidualNeighbor(tid);
}

void
FEProblemBase::addCachedResidual(const THREAD_ID tid)
{
  SubProblem::addCachedResidual(tid);
  if (_displaced_problem)
    _displaced_problem->addCachedResidual(tid);
}

void
FEProblemBase::addCachedResidualDirectly(NumericVector<Number> & residual, const THREAD_ID tid)
{
  if (_current_nl_sys->hasVector(_current_nl_sys->timeVectorTag()))
    _assembly[tid][_current_nl_sys->number()]->addCachedResidualDirectly(
        residual, Assembly::GlobalDataKey{}, getVectorTag(_current_nl_sys->timeVectorTag()));

  if (_current_nl_sys->hasVector(_current_nl_sys->nonTimeVectorTag()))
    _assembly[tid][_current_nl_sys->number()]->addCachedResidualDirectly(
        residual, Assembly::GlobalDataKey{}, getVectorTag(_current_nl_sys->nonTimeVectorTag()));

  // We do this because by adding the cached residual directly, we cannot ensure that all of the
  // cached residuals are emptied after only the two add calls above
  _assembly[tid][_current_nl_sys->number()]->clearCachedResiduals(Assembly::GlobalDataKey{});

  if (_displaced_problem)
    _displaced_problem->addCachedResidualDirectly(residual, tid);
}

void
FEProblemBase::setResidual(NumericVector<Number> & residual, const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->setResidual(
      residual,
      Assembly::GlobalDataKey{},
      getVectorTag(_nl[_current_nl_sys->number()]->residualVectorTag()));
  if (_displaced_problem)
    _displaced_problem->setResidual(residual, tid);
}

void
FEProblemBase::setResidualNeighbor(NumericVector<Number> & residual, const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->setResidualNeighbor(
      residual, Assembly::GlobalDataKey{}, getVectorTag(_current_nl_sys->residualVectorTag()));
  if (_displaced_problem)
    _displaced_problem->setResidualNeighbor(residual, tid);
}

void
FEProblemBase::addJacobian(const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->addJacobian(Assembly::GlobalDataKey{});
  if (_has_nonlocal_coupling)
    _assembly[tid][_current_nl_sys->number()]->addJacobianNonlocal(Assembly::GlobalDataKey{});
  if (_displaced_problem)
  {
    _displaced_problem->addJacobian(tid);
    if (_has_nonlocal_coupling)
      _displaced_problem->addJacobianNonlocal(tid);
  }
}

void
FEProblemBase::addJacobianNeighbor(const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->addJacobianNeighbor(Assembly::GlobalDataKey{});
  if (_displaced_problem)
    _displaced_problem->addJacobianNeighbor(tid);
}

void
FEProblemBase::addJacobianNeighborLowerD(const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->addJacobianNeighborLowerD(Assembly::GlobalDataKey{});
  if (_displaced_problem)
    _displaced_problem->addJacobianNeighborLowerD(tid);
}

void
FEProblemBase::addJacobianLowerD(const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->addJacobianLowerD(Assembly::GlobalDataKey{});
  if (_displaced_problem)
    _displaced_problem->addJacobianLowerD(tid);
}

void
FEProblemBase::addJacobianScalar(const THREAD_ID tid /* = 0*/)
{
  _assembly[tid][_current_nl_sys->number()]->addJacobianScalar(Assembly::GlobalDataKey{});
}

void
FEProblemBase::addJacobianOffDiagScalar(unsigned int ivar, const THREAD_ID tid /* = 0*/)
{
  _assembly[tid][_current_nl_sys->number()]->addJacobianOffDiagScalar(ivar,
                                                                      Assembly::GlobalDataKey{});
}

void
FEProblemBase::cacheJacobian(const THREAD_ID tid)
{
  SubProblem::cacheJacobian(tid);
  if (_displaced_problem)
    _displaced_problem->cacheJacobian(tid);
}

void
FEProblemBase::cacheJacobianNeighbor(const THREAD_ID tid)
{
  SubProblem::cacheJacobianNeighbor(tid);
  if (_displaced_problem)
    _displaced_problem->cacheJacobianNeighbor(tid);
}

void
FEProblemBase::addCachedJacobian(const THREAD_ID tid)
{
  SubProblem::addCachedJacobian(tid);
  if (_displaced_problem)
    _displaced_problem->addCachedJacobian(tid);
}

void
FEProblemBase::addJacobianBlockTags(SparseMatrix<Number> & jacobian,
                                    unsigned int ivar,
                                    unsigned int jvar,
                                    const DofMap & dof_map,
                                    std::vector<dof_id_type> & dof_indices,
                                    const std::set<TagID> & tags,
                                    const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->addJacobianBlockTags(
      jacobian, ivar, jvar, dof_map, dof_indices, Assembly::GlobalDataKey{}, tags);

  if (_has_nonlocal_coupling)
    if (_nonlocal_cm[_current_nl_sys->number()](ivar, jvar) != 0)
    {
      MooseVariableFEBase & jv = _current_nl_sys->getVariable(tid, jvar);
      _assembly[tid][_current_nl_sys->number()]->addJacobianBlockNonlocalTags(
          jacobian,
          ivar,
          jvar,
          dof_map,
          dof_indices,
          jv.allDofIndices(),
          Assembly::GlobalDataKey{},
          tags);
    }

  if (_displaced_problem)
  {
    _displaced_problem->addJacobianBlockTags(jacobian, ivar, jvar, dof_map, dof_indices, tags, tid);
    if (_has_nonlocal_coupling)
      if (_nonlocal_cm[_current_nl_sys->number()](ivar, jvar) != 0)
      {
        MooseVariableFEBase & jv = _current_nl_sys->getVariable(tid, jvar);
        _displaced_problem->addJacobianBlockNonlocal(
            jacobian, ivar, jvar, dof_map, dof_indices, jv.allDofIndices(), tags, tid);
      }
  }
}

void
FEProblemBase::addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DofMap & dof_map,
                                   std::vector<dof_id_type> & dof_indices,
                                   std::vector<dof_id_type> & neighbor_dof_indices,
                                   const std::set<TagID> & tags,
                                   const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->addJacobianNeighborTags(jacobian,
                                                                     ivar,
                                                                     jvar,
                                                                     dof_map,
                                                                     dof_indices,
                                                                     neighbor_dof_indices,
                                                                     Assembly::GlobalDataKey{},
                                                                     tags);
  if (_displaced_problem)
    _displaced_problem->addJacobianNeighbor(
        jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices, tags, tid);
}

void
FEProblemBase::prepareShapes(unsigned int var, const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->copyShapes(var);
}

void
FEProblemBase::prepareFaceShapes(unsigned int var, const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->copyFaceShapes(var);
}

void
FEProblemBase::prepareNeighborShapes(unsigned int var, const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->copyNeighborShapes(var);
}

void
FEProblemBase::addGhostedElem(dof_id_type elem_id)
{
  if (_mesh.elemPtr(elem_id)->processor_id() != processor_id())
    _ghosted_elems.insert(elem_id);
}

void
FEProblemBase::addGhostedBoundary(BoundaryID boundary_id)
{
  _mesh.addGhostedBoundary(boundary_id);
  if (_displaced_problem)
    _displaced_mesh->addGhostedBoundary(boundary_id);
}

void
FEProblemBase::ghostGhostedBoundaries()
{
  TIME_SECTION("ghostGhostedBoundaries", 3, "Ghosting Ghosted Boundaries");

  _mesh.ghostGhostedBoundaries();

  if (_displaced_problem)
    _displaced_mesh->ghostGhostedBoundaries();
}

void
FEProblemBase::sizeZeroes(unsigned int /*size*/, const THREAD_ID /*tid*/)
{
  mooseDoOnce(mooseWarning(
      "This function is deprecated and no longer performs any function. Please do not call it."));
}

bool
FEProblemBase::reinitDirac(const Elem * elem, const THREAD_ID tid)
{
  std::vector<Point> & points = _dirac_kernel_info.getPoints()[elem].first;

  unsigned int n_points = points.size();

  if (n_points)
  {
    if (n_points > _max_qps)
    {
      _max_qps = n_points;

      /**
       * The maximum number of qps can rise if several Dirac points are added to a single element.
       * In that case we need to resize the zeros to compensate.
       */
      unsigned int max_qpts = getMaxQps();
      for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
      {
        // the highest available order in libMesh is 43
        _scalar_zero[tid].resize(FORTYTHIRD, 0);
        _zero[tid].resize(max_qpts, 0);
        _grad_zero[tid].resize(max_qpts, RealGradient(0.));
        _second_zero[tid].resize(max_qpts, RealTensor(0.));
        _vector_zero[tid].resize(max_qpts, RealGradient(0.));
        _vector_curl_zero[tid].resize(max_qpts, RealGradient(0.));
      }
    }

    for (const auto i : index_range(_nl))
    {
      _assembly[tid][i]->reinitAtPhysical(elem, points);
      _nl[i]->prepare(tid);
    }
    _aux->prepare(tid);

    reinitElem(elem, tid);
  }

  _assembly[tid][_current_nl_sys->number()]->prepare();
  if (_has_nonlocal_coupling)
    _assembly[tid][_current_nl_sys->number()]->prepareNonlocal();

  bool have_points = n_points > 0;
  if (_displaced_problem && (_reinit_displaced_elem))
  {
    have_points |= _displaced_problem->reinitDirac(_displaced_mesh->elemPtr(elem->id()), tid);
    if (_has_nonlocal_coupling)
      _displaced_problem->prepareNonlocal(tid);
  }

  return have_points;
}

void
FEProblemBase::reinitElem(const Elem * elem, const THREAD_ID tid)
{
  for (auto & sys : _solver_systems)
    sys->reinitElem(elem, tid);
  _aux->reinitElem(elem, tid);

  if (_displaced_problem && _reinit_displaced_elem)
    _displaced_problem->reinitElem(_displaced_mesh->elemPtr(elem->id()), tid);
}

void
FEProblemBase::reinitElemPhys(const Elem * const elem,
                              const std::vector<Point> & phys_points_in_elem,
                              const THREAD_ID tid)
{
  mooseAssert(_mesh.queryElemPtr(elem->id()) == elem,
              "Are you calling this method with a displaced mesh element?");

  for (const auto i : index_range(_solver_systems))
  {
    _assembly[tid][i]->reinitAtPhysical(elem, phys_points_in_elem);
    _solver_systems[i]->prepare(tid);
    _assembly[tid][i]->prepare();
    if (_has_nonlocal_coupling)
      _assembly[tid][i]->prepareNonlocal();
  }
  _aux->prepare(tid);

  reinitElem(elem, tid);
}

void
FEProblemBase::reinitElemFace(const Elem * const elem,
                              const unsigned int side,
                              const BoundaryID,
                              const THREAD_ID tid)
{
  mooseDeprecated(
      "reinitElemFace with a BoundaryID argument is deprecated because the boundary id was never "
      "used. Please call reinitElemFace without the BoundaryID argument instead");

  reinitElemFace(elem, side, tid);
}

void
FEProblemBase::reinitElemFace(const Elem * const elem, const unsigned int side, const THREAD_ID tid)
{
  for (const auto i : index_range(_solver_systems))
  {
    _assembly[tid][i]->reinit(elem, side);
    _solver_systems[i]->reinitElemFace(elem, side, tid);
  }
  _aux->reinitElemFace(elem, side, tid);

  if (_displaced_problem && _reinit_displaced_face)
    _displaced_problem->reinitElemFace(_displaced_mesh->elemPtr(elem->id()), side, tid);
}

void
FEProblemBase::reinitLowerDElem(const Elem * lower_d_elem,
                                const THREAD_ID tid,
                                const std::vector<Point> * const pts,
                                const std::vector<Real> * const weights)
{
  SubProblem::reinitLowerDElem(lower_d_elem, tid, pts, weights);

  if (_displaced_problem && _displaced_mesh)
    _displaced_problem->reinitLowerDElem(
        _displaced_mesh->elemPtr(lower_d_elem->id()), tid, pts, weights);
}

void
FEProblemBase::reinitNode(const Node * node, const THREAD_ID tid)
{
  if (_displaced_problem && _reinit_displaced_elem)
    _displaced_problem->reinitNode(&_displaced_mesh->nodeRef(node->id()), tid);

  for (const auto i : index_range(_nl))
  {
    _assembly[tid][i]->reinit(node);
    _nl[i]->reinitNode(node, tid);
  }
  _aux->reinitNode(node, tid);
}

void
FEProblemBase::reinitNodeFace(const Node * node, BoundaryID bnd_id, const THREAD_ID tid)
{
  if (_displaced_problem && _reinit_displaced_face)
    _displaced_problem->reinitNodeFace(&_displaced_mesh->nodeRef(node->id()), bnd_id, tid);

  for (const auto i : index_range(_nl))
  {
    _assembly[tid][i]->reinit(node);
    _nl[i]->reinitNodeFace(node, bnd_id, tid);
  }
  _aux->reinitNodeFace(node, bnd_id, tid);
}

void
FEProblemBase::reinitNodes(const std::vector<dof_id_type> & nodes, const THREAD_ID tid)
{
  if (_displaced_problem && _reinit_displaced_elem)
    _displaced_problem->reinitNodes(nodes, tid);

  for (auto & nl : _nl)
    nl->reinitNodes(nodes, tid);
  _aux->reinitNodes(nodes, tid);
}

void
FEProblemBase::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, const THREAD_ID tid)
{
  if (_displaced_problem && _reinit_displaced_elem)
    _displaced_problem->reinitNodesNeighbor(nodes, tid);

  for (auto & nl : _nl)
    nl->reinitNodesNeighbor(nodes, tid);
  _aux->reinitNodesNeighbor(nodes, tid);
}

void
FEProblemBase::reinitScalars(const THREAD_ID tid, bool reinit_for_derivative_reordering /*=false*/)
{
  TIME_SECTION("reinitScalars", 3, "Reinitializing Scalar Variables");

  if (_displaced_problem && _reinit_displaced_elem)
    _displaced_problem->reinitScalars(tid, reinit_for_derivative_reordering);

  for (auto & nl : _nl)
    nl->reinitScalars(tid, reinit_for_derivative_reordering);
  _aux->reinitScalars(tid, reinit_for_derivative_reordering);

  // This is called outside of residual/Jacobian call-backs
  for (auto & assembly : _assembly[tid])
    assembly->prepareScalar();
}

void
FEProblemBase::reinitOffDiagScalars(const THREAD_ID tid)
{
  _assembly[tid][_current_nl_sys->number()]->prepareOffDiagScalar();
  if (_displaced_problem)
    _displaced_problem->reinitOffDiagScalars(tid);
}

void
FEProblemBase::reinitNeighbor(const Elem * elem, unsigned int side, const THREAD_ID tid)
{
  setNeighborSubdomainID(elem, side, tid);

  const Elem * neighbor = elem->neighbor_ptr(side);
  unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);

  for (const auto i : index_range(_nl))
  {
    _assembly[tid][i]->reinitElemAndNeighbor(elem, side, neighbor, neighbor_side);
    _nl[i]->prepareNeighbor(tid);
    // Called during stateful material property evaluation outside of solve
    _assembly[tid][i]->prepareNeighbor();
  }
  _aux->prepareNeighbor(tid);

  for (auto & nl : _nl)
  {
    nl->reinitElemFace(elem, side, tid);
    nl->reinitNeighborFace(neighbor, neighbor_side, tid);
  }
  _aux->reinitElemFace(elem, side, tid);
  _aux->reinitNeighborFace(neighbor, neighbor_side, tid);

  if (_displaced_problem && _reinit_displaced_neighbor)
  {
    // There are cases like for cohesive zone modeling without significant sliding where we cannot
    // use FEInterface::inverse_map in Assembly::reinitElemAndNeighbor in the displaced problem
    // because the physical points coming from the element don't actually lie on the neighbor.
    // Moreover, what's the point of doing another physical point inversion in other cases? We only
    // care about the reference points which we can just take from the undisplaced computation
    const auto & displaced_ref_pts = _assembly[tid][0]->qRuleNeighbor()->get_points();

    _displaced_problem->reinitNeighbor(
        _displaced_mesh->elemPtr(elem->id()), side, tid, &displaced_ref_pts);
  }
}

void
FEProblemBase::reinitElemNeighborAndLowerD(const Elem * elem,
                                           unsigned int side,
                                           const THREAD_ID tid)
{
  reinitNeighbor(elem, side, tid);

  const Elem * lower_d_elem = _mesh.getLowerDElem(elem, side);
  if (lower_d_elem && _mesh.interiorLowerDBlocks().count(lower_d_elem->subdomain_id()) > 0)
    reinitLowerDElem(lower_d_elem, tid);
  else
  {
    // with mesh refinement, lower-dimensional element might be defined on neighbor side
    auto & neighbor = _assembly[tid][0]->neighbor();
    auto & neighbor_side = _assembly[tid][0]->neighborSide();
    const Elem * lower_d_elem_neighbor = _mesh.getLowerDElem(neighbor, neighbor_side);
    if (lower_d_elem_neighbor &&
        _mesh.interiorLowerDBlocks().count(lower_d_elem_neighbor->subdomain_id()) > 0)
    {
      auto qps = _assembly[tid][0]->qPointsFaceNeighbor().stdVector();
      std::vector<Point> reference_points;
      FEInterface::inverse_map(
          lower_d_elem_neighbor->dim(), FEType(), lower_d_elem_neighbor, qps, reference_points);
      reinitLowerDElem(lower_d_elem_neighbor, tid, &reference_points);
    }
  }

  if (_displaced_problem &&
      (_reinit_displaced_elem || _reinit_displaced_face || _reinit_displaced_neighbor))
    _displaced_problem->reinitElemNeighborAndLowerD(
        _displaced_mesh->elemPtr(elem->id()), side, tid);
}

void
FEProblemBase::reinitNeighborPhys(const Elem * neighbor,
                                  unsigned int neighbor_side,
                                  const std::vector<Point> & physical_points,
                                  const THREAD_ID tid)
{
  mooseAssert(_mesh.queryElemPtr(neighbor->id()) == neighbor,
              "Are you calling this method with a displaced mesh element?");

  for (const auto i : index_range(_nl))
  {
    // Reinits shape the functions at the physical points
    _assembly[tid][i]->reinitNeighborAtPhysical(neighbor, neighbor_side, physical_points);

    // Sets the neighbor dof indices
    _nl[i]->prepareNeighbor(tid);
  }
  _aux->prepareNeighbor(tid);

  // Resizes Re and Ke
  _assembly[tid][_current_nl_sys->number()]->prepareNeighbor();

  // Compute the values of each variable at the points
  for (auto & nl : _nl)
    nl->reinitNeighborFace(neighbor, neighbor_side, tid);
  _aux->reinitNeighborFace(neighbor, neighbor_side, tid);
}

void
FEProblemBase::reinitNeighborPhys(const Elem * neighbor,
                                  const std::vector<Point> & physical_points,
                                  const THREAD_ID tid)
{
  mooseAssert(_mesh.queryElemPtr(neighbor->id()) == neighbor,
              "Are you calling this method with a displaced mesh element?");

  for (const auto i : index_range(_nl))
  {
    // Reinits shape the functions at the physical points
    _assembly[tid][i]->reinitNeighborAtPhysical(neighbor, physical_points);

    // Sets the neighbor dof indices
    _nl[i]->prepareNeighbor(tid);
  }
  _aux->prepareNeighbor(tid);

  // Resizes Re and Ke
  _assembly[tid][_current_nl_sys->number()]->prepareNeighbor();

  // Compute the values of each variable at the points
  for (auto & nl : _nl)
    nl->reinitNeighbor(neighbor, tid);
  _aux->reinitNeighbor(neighbor, tid);
}

void
FEProblemBase::getDiracElements(std::set<const Elem *> & elems)
{
  // First add in the undisplaced elements
  elems = _dirac_kernel_info.getElements();

  if (_displaced_problem)
  {
    std::set<const Elem *> displaced_elements;
    _displaced_problem->getDiracElements(displaced_elements);

    { // Use the ids from the displaced elements to get the undisplaced elements
      // and add them to the list
      for (const auto & elem : displaced_elements)
        elems.insert(_mesh.elemPtr(elem->id()));
    }
  }
}

void
FEProblemBase::clearDiracInfo()
{
  _dirac_kernel_info.clearPoints();

  if (_displaced_problem)
    _displaced_problem->clearDiracInfo();
}

void
FEProblemBase::subdomainSetup(SubdomainID subdomain, const THREAD_ID tid)
{
  _all_materials.subdomainSetup(subdomain, tid);

  // Call the subdomain methods of the output system, these are not threaded so only call it once
  if (tid == 0)
    _app.getOutputWarehouse().subdomainSetup();

  for (auto & nl : _nl)
    nl->subdomainSetup(subdomain, tid);

  // FIXME: call displaced_problem->subdomainSetup() ?
  //        When adding possibility with materials being evaluated on displaced mesh
}

void
FEProblemBase::neighborSubdomainSetup(SubdomainID subdomain, const THREAD_ID tid)
{
  _all_materials.neighborSubdomainSetup(subdomain, tid);
}

void
FEProblemBase::addFunction(const std::string & type,
                           const std::string & name,
                           InputParameters & parameters)
{
  parallel_object_only();

  parameters.set<SubProblem *>("_subproblem") = this;

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<Function> func = _factory.create<Function>(type, name, parameters, tid);
    logAdd("Function", name, type, parameters);
    _functions.addObject(func, tid);

    if (auto * const functor = dynamic_cast<Moose::FunctorBase<Real> *>(func.get()))
    {
      this->addFunctor(name, *functor, tid);
      if (_displaced_problem)
        _displaced_problem->addFunctor(name, *functor, tid);
    }
    else
      mooseError("Unrecognized function functor type");
  }
}

void
FEProblemBase::addConvergence(const std::string & type,
                              const std::string & name,
                              InputParameters & parameters)
{
  parallel_object_only();

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<Convergence> conv = _factory.create<Convergence>(type, name, parameters, tid);
    _convergences.addObject(conv, tid);
  }
}

void
FEProblemBase::addDefaultNonlinearConvergence(const InputParameters & params_to_apply)
{
  const std::string class_name = "DefaultNonlinearConvergence";
  InputParameters params = _factory.getValidParams(class_name);
  params.applyParameters(params_to_apply);
  params.applyParameters(parameters());
  params.set<bool>("added_as_default") = true;
  for (const auto & conv_name : getNonlinearConvergenceNames())
    addConvergence(class_name, conv_name, params);
}

bool
FEProblemBase::hasFunction(const std::string & name, const THREAD_ID tid)
{
  return _functions.hasActiveObject(name, tid);
}

Function &
FEProblemBase::getFunction(const std::string & name, const THREAD_ID tid)
{
  // This thread lock is necessary since this method will create functions
  // for all threads if one is missing.
  Threads::spin_mutex::scoped_lock lock(get_function_mutex);

  if (!hasFunction(name, tid))
  {
    // If we didn't find a function, it might be a default function, attempt to construct one now
    std::istringstream ss(name);
    Real real_value;

    // First see if it's just a constant. If it is, build a ConstantFunction
    if (ss >> real_value && ss.eof())
    {
      InputParameters params = _factory.getValidParams("ConstantFunction");
      params.set<Real>("value") = real_value;
      addFunction("ConstantFunction", ss.str(), params);
    }
    else
    {
      FunctionParserBase<Real> fp;
      std::string vars = "x,y,z,t,NaN,pi,e";
      if (fp.Parse(name, vars) == -1) // -1 for success
      {
        // It parsed ok, so build a MooseParsedFunction
        InputParameters params = _factory.getValidParams("ParsedFunction");
        params.set<std::string>("expression") = name;
        addFunction("ParsedFunction", name, params);
      }
    }

    // Try once more
    if (!hasFunction(name, tid))
      mooseError("Unable to find function " + name);
  }

  auto * const ret = dynamic_cast<Function *>(_functions.getActiveObject(name, tid).get());
  if (!ret)
    mooseError("No function named ", name, " of appropriate type");

  return *ret;
}

bool
FEProblemBase::hasConvergence(const std::string & name, const THREAD_ID tid) const
{
  return _convergences.hasActiveObject(name, tid);
}

Convergence &
FEProblemBase::getConvergence(const std::string & name, const THREAD_ID tid) const
{
  auto * const ret = dynamic_cast<Convergence *>(_convergences.getActiveObject(name, tid).get());
  if (!ret)
    mooseError("The Convergence object '", name, "' does not exist.");

  return *ret;
}

const std::vector<std::shared_ptr<Convergence>> &
FEProblemBase::getConvergenceObjects(const THREAD_ID tid) const
{
  return _convergences.getActiveObjects(tid);
}

void
FEProblemBase::addMeshDivision(const std::string & type,
                               const std::string & name,
                               InputParameters & parameters)
{
  parallel_object_only();
  parameters.set<FEProblemBase *>("_fe_problem_base") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<MeshDivision> func = _factory.create<MeshDivision>(type, name, parameters, tid);
    _mesh_divisions.addObject(func, tid);
  }
}

MeshDivision &
FEProblemBase::getMeshDivision(const std::string & name, const THREAD_ID tid) const
{
  auto * const ret = dynamic_cast<MeshDivision *>(_mesh_divisions.getActiveObject(name, tid).get());
  if (!ret)
    mooseError("No MeshDivision object named ", name, " of appropriate type");
  return *ret;
}

void
FEProblemBase::lineSearch()
{
  _line_search->lineSearch();
}

NonlinearSystem &
FEProblemBase::getNonlinearSystem(const unsigned int sys_num)
{
  mooseDeprecated("FEProblemBase::getNonlinearSystem() is deprecated, please use "
                  "FEProblemBase::getNonlinearSystemBase() \n");

  mooseAssert(sys_num < _nl.size(), "System number greater than the number of nonlinear systems");
  auto nl_sys = std::dynamic_pointer_cast<NonlinearSystem>(_nl[sys_num]);

  if (!nl_sys)
    mooseError("This is not a NonlinearSystem");

  return *nl_sys;
}

void
FEProblemBase::addDistribution(const std::string & type,
                               const std::string & name,
                               InputParameters & parameters)
{
  parameters.set<std::string>("type") = type;
  addObject<Distribution>(type, name, parameters, /* threaded = */ false);
}

Distribution &
FEProblemBase::getDistribution(const std::string & name)
{
  std::vector<Distribution *> objs;
  theWarehouse()
      .query()
      .condition<AttribSystem>("Distribution")
      .condition<AttribName>(name)
      .queryInto(objs);
  if (objs.empty())
    mooseError("Unable to find Distribution with name '" + name + "'");
  return *(objs[0]);
}

void
FEProblemBase::addSampler(const std::string & type,
                          const std::string & name,
                          InputParameters & parameters)
{
  const auto samplers = addObject<Sampler>(type, name, parameters);
  for (auto & sampler : samplers)
    sampler->init();
}

Sampler &
FEProblemBase::getSampler(const std::string & name, const THREAD_ID tid)
{
  std::vector<Sampler *> objs;
  theWarehouse()
      .query()
      .condition<AttribSystem>("Sampler")
      .condition<AttribThread>(tid)
      .condition<AttribName>(name)
      .queryInto(objs);
  if (objs.empty())
    mooseError(
        "Unable to find Sampler with name '" + name +
        "', if you are attempting to access this object in the constructor of another object then "
        "the object being retrieved must occur prior to the caller within the input file.");
  return *(objs[0]);
}

bool
FEProblemBase::duplicateVariableCheck(const std::string & var_name,
                                      const FEType & type,
                                      bool is_aux,
                                      const std::set<SubdomainID> * const active_subdomains)
{

  std::set<SubdomainID> subdomainIDs;
  if (active_subdomains->size() == 0)
  {
    const auto subdomains = _mesh.meshSubdomains();
    subdomainIDs.insert(subdomains.begin(), subdomains.end());
  }
  else
    subdomainIDs.insert(active_subdomains->begin(), active_subdomains->end());

  for (auto & sys : _solver_systems)
  {
    SystemBase * curr_sys_ptr = sys.get();
    SystemBase * other_sys_ptr = _aux.get();
    std::string error_prefix = "";
    if (is_aux)
    {
      curr_sys_ptr = _aux.get();
      other_sys_ptr = sys.get();
      error_prefix = "aux";
    }

    if (other_sys_ptr->hasVariable(var_name))
      mooseError("Cannot have an auxiliary variable and a solver variable with the same name: ",
                 var_name);

    if (curr_sys_ptr->hasVariable(var_name))
    {
      const Variable & var =
          curr_sys_ptr->system().variable(curr_sys_ptr->system().variable_number(var_name));

      // variable type
      if (var.type() != type)
      {
        const auto stringifyType = [](FEType t)
        { return Moose::stringify(t.family) + " of order " + Moose::stringify(t.order); };

        mooseError("Mismatching types are specified for ",
                   error_prefix,
                   "variable with name '",
                   var_name,
                   "': '",
                   stringifyType(var.type()),
                   "' and '",
                   stringifyType(type),
                   "'");
      }

      // block-restriction
      if (!(active_subdomains->size() == 0 && var.active_subdomains().size() == 0))
      {
        const auto varActiveSubdomains = var.active_subdomains();
        std::set<SubdomainID> varSubdomainIDs;
        if (varActiveSubdomains.size() == 0)
        {
          const auto subdomains = _mesh.meshSubdomains();
          varSubdomainIDs.insert(subdomains.begin(), subdomains.end());
        }
        else
          varSubdomainIDs.insert(varActiveSubdomains.begin(), varActiveSubdomains.end());

        // Is subdomainIDs a subset of varSubdomainIDs? With this we allow the case that the newly
        // requested block restriction is only a subset of the existing one.
        const auto isSubset = std::includes(varSubdomainIDs.begin(),
                                            varSubdomainIDs.end(),
                                            subdomainIDs.begin(),
                                            subdomainIDs.end());

        if (!isSubset)
        {
          // helper function: make a string from a set of subdomain ids
          const auto stringifySubdomains = [this](std::set<SubdomainID> subdomainIDs)
          {
            std::stringstream s;
            for (auto const i : subdomainIDs)
            {
              // do we need to insert a comma?
              if (s.tellp() != 0)
                s << ", ";

              // insert subdomain name and id -or- only the id (if no name is given)
              const auto subdomainName = _mesh.getSubdomainName(i);
              if (subdomainName.empty())
                s << i;
              else
                s << subdomainName << " (" << i << ")";
            }
            return s.str();
          };

          const std::string msg = "Mismatching block-restrictions are specified for " +
                                  error_prefix + "variable with name '" + var_name + "': {" +
                                  stringifySubdomains(varSubdomainIDs) + "} and {" +
                                  stringifySubdomains(subdomainIDs) + "}";

          mooseError(msg);
        }
      }

      return true;
    }
  }

  return false;
}

void
FEProblemBase::addVariable(const std::string & var_type,
                           const std::string & var_name,
                           InputParameters & params)
{
  parallel_object_only();

  auto fe_type = FEType(Utility::string_to_enum<Order>(params.get<MooseEnum>("order")),
                        Utility::string_to_enum<FEFamily>(params.get<MooseEnum>("family")));

  const auto active_subdomains_vector =
      _mesh.getSubdomainIDs(params.get<std::vector<SubdomainName>>("block"));
  const std::set<SubdomainID> active_subdomains(active_subdomains_vector.begin(),
                                                active_subdomains_vector.end());

  if (duplicateVariableCheck(var_name, fe_type, /* is_aux = */ false, &active_subdomains))
    return;

  params.set<FEProblemBase *>("_fe_problem_base") = this;
  params.set<Moose::VarKindType>("_var_kind") = Moose::VarKindType::VAR_SOLVER;
  SolverSystemName sys_name = params.get<SolverSystemName>("solver_sys");

  const auto solver_system_number = solverSysNum(sys_name);
  logAdd("Variable", var_name, var_type, params);
  _solver_systems[solver_system_number]->addVariable(var_type, var_name, params);
  if (_displaced_problem)
    // MooseObjects need to be unique so change the name here
    _displaced_problem->addVariable(var_type, var_name, params, solver_system_number);

  _solver_var_to_sys_num[var_name] = solver_system_number;
}

std::pair<bool, unsigned int>
FEProblemBase::determineSolverSystem(const std::string & var_name,
                                     const bool error_if_not_found) const
{
  auto map_it = _solver_var_to_sys_num.find(var_name);
  const bool var_in_sys = map_it != _solver_var_to_sys_num.end();
  if (var_in_sys)
    mooseAssert(_solver_systems[map_it->second]->hasVariable(var_name) ||
                    _solver_systems[map_it->second]->hasScalarVariable(var_name),
                "If the variable is in our FEProblem solver system map, then it must be in the "
                "solver system we expect");
  else if (error_if_not_found)
  {
    if (_aux->hasVariable(var_name) || _aux->hasScalarVariable(var_name))
      mooseError("No solver variable named ",
                 var_name,
                 " found. Did you specify an auxiliary variable when you meant to specify a "
                 "solver variable?");
    else
      mooseError("Unknown variable '",
                 var_name,
                 "'. It does not exist in the solver system(s) or auxiliary system");
  }

  return std::make_pair(var_in_sys, var_in_sys ? map_it->second : libMesh::invalid_uint);
}

void
FEProblemBase::setResidualObjectParamsAndLog(const std::string & ro_name,
                                             const std::string & name,
                                             InputParameters & parameters,
                                             const unsigned int nl_sys_num,
                                             const std::string & base_name,
                                             bool & reinit_displaced)
{
  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->solverSys(nl_sys_num);
    reinit_displaced = true;
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Kernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Kernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl[nl_sys_num].get();
  }

  logAdd(base_name, name, ro_name, parameters);
}

void
FEProblemBase::addKernel(const std::string & kernel_name,
                         const std::string & name,
                         InputParameters & parameters)
{
  parallel_object_only();
  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;
  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError("You are trying to add a Kernel to a linear variable/system, which is not "
               "supported at the moment!");
  setResidualObjectParamsAndLog(
      kernel_name, name, parameters, nl_sys_num, "Kernel", _reinit_displaced_elem);

  _nl[nl_sys_num]->addKernel(kernel_name, name, parameters);
}

void
FEProblemBase::addHDGKernel(const std::string & kernel_name,
                            const std::string & name,
                            InputParameters & parameters)
{
  parallel_object_only();
  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;
  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError("You are trying to add a HDGKernel to a linear variable/system, which is not "
               "supported at the moment!");
  setResidualObjectParamsAndLog(
      kernel_name, name, parameters, nl_sys_num, "HDGKernel", _reinit_displaced_elem);

  _nl[nl_sys_num]->addHDGKernel(kernel_name, name, parameters);
}

void
FEProblemBase::addNodalKernel(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters & parameters)
{
  parallel_object_only();

  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;
  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->solverSys(nl_sys_num);
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow NodalKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this NodalKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl[nl_sys_num].get();
  }
  logAdd("NodalKernel", name, kernel_name, parameters);
  _nl[nl_sys_num]->addNodalKernel(kernel_name, name, parameters);
}

void
FEProblemBase::addScalarKernel(const std::string & kernel_name,
                               const std::string & name,
                               InputParameters & parameters)
{
  parallel_object_only();

  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;
  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError("You are trying to add a ScalarKernel to a linear variable/system, which is not "
               "supported at the moment!");

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->solverSys(nl_sys_num);
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow ScalarKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this ScalarKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl[nl_sys_num].get();
  }

  logAdd("ScalarKernel", name, kernel_name, parameters);
  _nl[nl_sys_num]->addScalarKernel(kernel_name, name, parameters);
}

void
FEProblemBase::addBoundaryCondition(const std::string & bc_name,
                                    const std::string & name,
                                    InputParameters & parameters)
{
  parallel_object_only();

  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;
  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError(
        "You are trying to add a BoundaryCondition to a linear variable/system, which is not "
        "supported at the moment!");

  setResidualObjectParamsAndLog(
      bc_name, name, parameters, nl_sys_num, "BoundaryCondition", _reinit_displaced_face);
  _nl[nl_sys_num]->addBoundaryCondition(bc_name, name, parameters);
}

void
FEProblemBase::addHDGIntegratedBC(const std::string & bc_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  parallel_object_only();
  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;
  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError("You are trying to add a HDGIntegratedBC to a linear variable/system, which is not "
               "supported at the moment!");
  setResidualObjectParamsAndLog(
      bc_name, name, parameters, nl_sys_num, "BoundaryCondition", _reinit_displaced_face);

  _nl[nl_sys_num]->addHDGIntegratedBC(bc_name, name, parameters);
}

void
FEProblemBase::addConstraint(const std::string & c_name,
                             const std::string & name,
                             InputParameters & parameters)
{
  parallel_object_only();

  _has_constraints = true;

  auto determine_var_param_name = [&parameters, this]()
  {
    if (parameters.isParamValid("variable"))
      return "variable";
    else
    {
      // must be a mortar constraint
      const bool has_secondary_var = parameters.isParamValid("secondary_variable");
      const bool has_primary_var = parameters.isParamValid("primary_variable");
      if (!has_secondary_var && !has_primary_var)
        mooseError(
            "Either a 'secondary_variable' or 'primary_variable' parameter must be supplied for '",
            parameters.get<std::string>("_object_name"),
            "'");
      return has_secondary_var ? "secondary_variable" : "primary_variable";
    }
  };

  const auto nl_sys_num =
      determineSolverSystem(parameters.varName(determine_var_param_name(), name), true).second;
  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError("You are trying to add a Constraint to a linear variable/system, which is not "
               "supported at the moment!");

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->solverSys(nl_sys_num);
    _reinit_displaced_face = true;
  }
  else
  {
    // It might _want_ to use a displaced mesh... but we're not so set it to false
    if (parameters.have_parameter<bool>("use_displaced_mesh"))
      parameters.set<bool>("use_displaced_mesh") = false;

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl[nl_sys_num].get();
  }

  logAdd("Constraint", name, c_name, parameters);
  _nl[nl_sys_num]->addConstraint(c_name, name, parameters);
}

void
FEProblemBase::addAuxVariable(const std::string & var_type,
                              const std::string & var_name,
                              InputParameters & params)
{
  parallel_object_only();

  auto fe_type = FEType(Utility::string_to_enum<Order>(params.get<MooseEnum>("order")),
                        Utility::string_to_enum<FEFamily>(params.get<MooseEnum>("family")));

  const auto active_subdomains_vector =
      _mesh.getSubdomainIDs(params.get<std::vector<SubdomainName>>("block"));
  const std::set<SubdomainID> active_subdomains(active_subdomains_vector.begin(),
                                                active_subdomains_vector.end());

  if (duplicateVariableCheck(var_name, fe_type, /* is_aux = */ true, &active_subdomains))
    return;

  params.set<FEProblemBase *>("_fe_problem_base") = this;
  params.set<Moose::VarKindType>("_var_kind") = Moose::VarKindType::VAR_AUXILIARY;

  logAdd("AuxVariable", var_name, var_type, params);
  _aux->addVariable(var_type, var_name, params);
  if (_displaced_problem)
    // MooseObjects need to be unique so change the name here
    _displaced_problem->addAuxVariable(var_type, var_name, params);
}

void
FEProblemBase::addAuxVariable(const std::string & var_name,
                              const FEType & type,
                              const std::set<SubdomainID> * const active_subdomains)
{
  parallel_object_only();

  mooseDeprecated("Please use the addAuxVariable(var_type, var_name, params) API instead");

  if (duplicateVariableCheck(var_name, type, /* is_aux = */ true, active_subdomains))
    return;

  std::string var_type;
  if (type == FEType(0, MONOMIAL))
    var_type = "MooseVariableConstMonomial";
  else if (type.family == SCALAR)
    var_type = "MooseVariableScalar";
  else if (FEInterface::field_type(type) == TYPE_VECTOR)
    var_type = "VectorMooseVariable";
  else
    var_type = "MooseVariable";

  InputParameters params = _factory.getValidParams(var_type);
  params.set<FEProblemBase *>("_fe_problem_base") = this;
  params.set<Moose::VarKindType>("_var_kind") = Moose::VarKindType::VAR_AUXILIARY;
  params.set<MooseEnum>("order") = type.order.get_order();
  params.set<MooseEnum>("family") = Moose::stringify(type.family);

  if (active_subdomains)
    for (const SubdomainID & id : *active_subdomains)
      params.set<std::vector<SubdomainName>>("block").push_back(Moose::stringify(id));

  logAdd("AuxVariable", var_name, var_type, params);
  _aux->addVariable(var_type, var_name, params);
  if (_displaced_problem)
    _displaced_problem->addAuxVariable("MooseVariable", var_name, params);
}

void
FEProblemBase::addAuxArrayVariable(const std::string & var_name,
                                   const FEType & type,
                                   unsigned int components,
                                   const std::set<SubdomainID> * const active_subdomains)
{
  parallel_object_only();

  mooseDeprecated("Please use the addAuxVariable(var_type, var_name, params) API instead");

  if (duplicateVariableCheck(var_name, type, /* is_aux = */ true, active_subdomains))
    return;

  InputParameters params = _factory.getValidParams("ArrayMooseVariable");
  params.set<FEProblemBase *>("_fe_problem_base") = this;
  params.set<Moose::VarKindType>("_var_kind") = Moose::VarKindType::VAR_AUXILIARY;
  params.set<MooseEnum>("order") = type.order.get_order();
  params.set<MooseEnum>("family") = Moose::stringify(type.family);
  params.set<unsigned int>("components") = components;

  if (active_subdomains)
    for (const SubdomainID & id : *active_subdomains)
      params.set<std::vector<SubdomainName>>("block").push_back(Moose::stringify(id));

  logAdd("Variable", var_name, "ArrayMooseVariable", params);
  _aux->addVariable("ArrayMooseVariable", var_name, params);
  if (_displaced_problem)
    _displaced_problem->addAuxVariable("ArrayMooseVariable", var_name, params);
}

void
FEProblemBase::addAuxScalarVariable(const std::string & var_name,
                                    Order order,
                                    Real /*scale_factor*/,
                                    const std::set<SubdomainID> * const active_subdomains)
{
  parallel_object_only();

  mooseDeprecated("Please use the addAuxVariable(var_type, var_name, params) API instead");

  if (order > _max_scalar_order)
    _max_scalar_order = order;

  FEType type(order, SCALAR);
  if (duplicateVariableCheck(var_name, type, /* is_aux = */ true, active_subdomains))
    return;

  InputParameters params = _factory.getValidParams("MooseVariableScalar");
  params.set<FEProblemBase *>("_fe_problem_base") = this;
  params.set<Moose::VarKindType>("_var_kind") = Moose::VarKindType::VAR_AUXILIARY;

  params.set<MooseEnum>("order") = type.order.get_order();
  params.set<MooseEnum>("family") = "SCALAR";
  params.set<std::vector<Real>>("scaling") = {1};
  if (active_subdomains)
    for (const SubdomainID & id : *active_subdomains)
      params.set<std::vector<SubdomainName>>("block").push_back(Moose::stringify(id));

  logAdd("ScalarVariable", var_name, "MooseVariableScalar", params);
  _aux->addVariable("MooseVariableScalar", var_name, params);
  if (_displaced_problem)
    _displaced_problem->addAuxVariable("MooseVariableScalar", var_name, params);
}

void
FEProblemBase::addAuxKernel(const std::string & kernel_name,
                            const std::string & name,
                            InputParameters & parameters)
{
  parallel_object_only();

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    parameters.set<SystemBase *>("_nl_sys") = &_displaced_problem->solverSys(0);
    if (!parameters.get<std::vector<BoundaryName>>("boundary").empty())
      _reinit_displaced_face = true;
    else
      _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow AuxKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this AuxKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
    parameters.set<SystemBase *>("_nl_sys") = _solver_systems[0].get();
  }

  logAdd("AuxKernel", name, kernel_name, parameters);
  _aux->addKernel(kernel_name, name, parameters);
}

void
FEProblemBase::addAuxScalarKernel(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  parallel_object_only();

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow AuxScalarKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this AuxScalarKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
  }

  logAdd("AuxScalarKernel", name, kernel_name, parameters);
  _aux->addScalarKernel(kernel_name, name, parameters);
}

void
FEProblemBase::addDiracKernel(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters & parameters)
{
  parallel_object_only();

  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;
  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError("You are trying to add a DiracKernel to a linear variable/system, which is not "
               "supported at the moment!");

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->solverSys(nl_sys_num);
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow DiracKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this DiracKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl[nl_sys_num].get();
  }

  logAdd("DiracKernel", name, kernel_name, parameters);
  _nl[nl_sys_num]->addDiracKernel(kernel_name, name, parameters);
}

// DGKernels ////

void
FEProblemBase::addDGKernel(const std::string & dg_kernel_name,
                           const std::string & name,
                           InputParameters & parameters)
{
  parallel_object_only();

  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;
  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError("You are trying to add a DGKernel to a linear variable/system, which is not "
               "supported at the moment!");

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->solverSys(nl_sys_num);
    _reinit_displaced_neighbor = true;
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow DGKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this DGKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl[nl_sys_num].get();
  }

  logAdd("DGKernel", name, dg_kernel_name, parameters);
  _nl[nl_sys_num]->addDGKernel(dg_kernel_name, name, parameters);

  _has_internal_edge_residual_objects = true;
}

void
FEProblemBase::addFVKernel(const std::string & fv_kernel_name,
                           const std::string & name,
                           InputParameters & parameters)
{
  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
    // FVElementalKernels are computed in the historically finite element threaded loops. They rely
    // on Assembly data like _current_elem. When we call reinit on the FEProblemBase we will only
    // reinit the DisplacedProblem and its associated Assembly objects if we mark this boolean as
    // true
    _reinit_displaced_elem = true;
  addObject<FVKernel>(fv_kernel_name, name, parameters);
}

void
FEProblemBase::addFVBC(const std::string & fv_bc_name,
                       const std::string & name,
                       InputParameters & parameters)
{
  addObject<FVBoundaryCondition>(fv_bc_name, name, parameters);
}

void
FEProblemBase::addFVInterfaceKernel(const std::string & fv_ik_name,
                                    const std::string & name,
                                    InputParameters & parameters)
{
  /// We assume that variable1 and variable2 can live on different systems, in this case
  /// the user needs to create two interface kernels with flipped variables and parameters
  addObject<FVInterfaceKernel>(
      fv_ik_name, name, parameters, /*threaded=*/true, /*variable_param_name=*/"variable1");
}

void
FEProblemBase::addLinearFVKernel(const std::string & kernel_name,
                                 const std::string & name,
                                 InputParameters & parameters)
{
  addObject<LinearFVKernel>(kernel_name, name, parameters);
}

void
FEProblemBase::addLinearFVBC(const std::string & bc_name,
                             const std::string & name,
                             InputParameters & parameters)
{
  addObject<LinearFVBoundaryCondition>(bc_name, name, parameters);
}

// InterfaceKernels ////

void
FEProblemBase::addInterfaceKernel(const std::string & interface_kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  parallel_object_only();

  const auto nl_sys_num = determineSolverSystem(parameters.varName("variable", name), true).second;
  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError("You are trying to add a InterfaceKernel to a linear variable/system, which is not "
               "supported at the moment!");

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->solverSys(nl_sys_num);
    _reinit_displaced_neighbor = true;
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow InterfaceKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this InterfaceKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl[nl_sys_num].get();
  }

  logAdd("InterfaceKernel", name, interface_kernel_name, parameters);
  _nl[nl_sys_num]->addInterfaceKernel(interface_kernel_name, name, parameters);

  _has_internal_edge_residual_objects = true;
}

void
FEProblemBase::checkICRestartError(const std::string & ic_name,
                                   const std::string & name,
                                   const VariableName & var_name)
{
  if (!_allow_ics_during_restart)
  {
    std::string restart_method = "";
    if (_app.isRestarting())
      restart_method =
          "a checkpoint restart, by IC object '" + ic_name + "' for variable '" + name + "'";
    else if (_app.getExReaderForRestart())
    {
      std::vector<std::string> restarted_vars = _app.getExReaderForRestart()->get_elem_var_names();
      const auto nodal_vars = _app.getExReaderForRestart()->get_nodal_var_names();
      const auto global_vars = _app.getExReaderForRestart()->get_global_var_names();
      restarted_vars.insert(restarted_vars.end(), nodal_vars.begin(), nodal_vars.end());
      restarted_vars.insert(restarted_vars.end(), global_vars.begin(), global_vars.end());

      if (std::find(restarted_vars.begin(), restarted_vars.end(), var_name) != restarted_vars.end())
        restart_method = "an Exodus restart, by IC object '" + ic_name + "' for variable '" + name +
                         "' that is also being restarted";
    }
    if (!restart_method.empty())
      mooseError(
          "Initial conditions have been specified during ",
          restart_method,
          ".\nThis is only allowed if you specify 'allow_initial_conditions_with_restart' to "
          "the [Problem], as initial conditions can override restarted fields");
  }
}

void
FEProblemBase::addInitialCondition(const std::string & ic_name,
                                   const std::string & name,
                                   InputParameters & parameters)
{
  parallel_object_only();

  // before we start to mess with the initial condition, we need to check parameters for errors.
  parameters.checkParams(name);
  const std::string & var_name = parameters.get<VariableName>("variable");

  // Forbid initial conditions on a restarted problem, as they would override the restart
  checkICRestartError(ic_name, name, var_name);

  parameters.set<SubProblem *>("_subproblem") = this;

  // field IC
  if (hasVariable(var_name))
  {
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      MooseVariableFEBase & var = getVariable(
          tid, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
      parameters.set<SystemBase *>("_sys") = &var.sys();
      std::shared_ptr<InitialConditionBase> ic;
      if (dynamic_cast<MooseVariable *>(&var))
        ic = _factory.create<InitialCondition>(ic_name, name, parameters, tid);
      else if (dynamic_cast<VectorMooseVariable *>(&var))
        ic = _factory.create<VectorInitialCondition>(ic_name, name, parameters, tid);
      else if (dynamic_cast<ArrayMooseVariable *>(&var))
        ic = _factory.create<ArrayInitialCondition>(ic_name, name, parameters, tid);
      else if (dynamic_cast<MooseVariableFVReal *>(&var))
        ic = _factory.create<InitialCondition>(ic_name, name, parameters, tid);
      else if (dynamic_cast<MooseLinearVariableFVReal *>(&var))
        ic = _factory.create<InitialCondition>(ic_name, name, parameters, tid);
      else
        mooseError("Your FE variable in initial condition ",
                   name,
                   " must be either of scalar or vector type");
      logAdd("IC", name, ic_name, parameters);
      _ics.addObject(ic, tid);
    }
  }

  // scalar IC
  else if (hasScalarVariable(var_name))
  {
    MooseVariableScalar & var = getScalarVariable(0, var_name);
    parameters.set<SystemBase *>("_sys") = &var.sys();
    std::shared_ptr<ScalarInitialCondition> ic =
        _factory.create<ScalarInitialCondition>(ic_name, name, parameters);
    logAdd("ScalarIC", name, ic_name, parameters);
    _scalar_ics.addObject(ic);
  }

  else
    mooseError(
        "Variable '", var_name, "' requested in initial condition '", name, "' does not exist.");
}

void
FEProblemBase::addFVInitialCondition(const std::string & ic_name,
                                     const std::string & name,
                                     InputParameters & parameters)
{
  parallel_object_only();

  // before we start to mess with the initial condition, we need to check parameters for errors.
  parameters.checkParams(name);
  const std::string & var_name = parameters.get<VariableName>("variable");

  // Forbid initial conditions on a restarted problem, as they would override the restart
  checkICRestartError(ic_name, name, var_name);

  parameters.set<SubProblem *>("_subproblem") = this;

  // field IC
  if (hasVariable(var_name))
  {
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      auto & var = getVariable(
          tid, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
      parameters.set<SystemBase *>("_sys") = &var.sys();
      std::shared_ptr<FVInitialConditionBase> ic;
      if (var.isFV())
        ic = _factory.create<FVInitialCondition>(ic_name, name, parameters, tid);
      else
        mooseError(
            "Your variable for an FVInitialCondition needs to be an a finite volume variable!");
      _fv_ics.addObject(ic, tid);
    }
  }
  else
    mooseError("Variable '",
               var_name,
               "' requested in finite volume initial condition '",
               name,
               "' does not exist.");
}

void
FEProblemBase::projectSolution()
{
  TIME_SECTION("projectSolution", 2, "Projecting Initial Solutions")

  FloatingPointExceptionGuard fpe_guard(_app);

  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  ComputeInitialConditionThread cic(*this);
  Threads::parallel_reduce(elem_range, cic);

  if (haveFV())
  {
    using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
    ElemInfoRange elem_info_range(_mesh.ownedElemInfoBegin(), _mesh.ownedElemInfoEnd());

    ComputeFVInitialConditionThread cfvic(*this);
    Threads::parallel_reduce(elem_info_range, cfvic);
  }

  // Need to close the solution vector here so that boundary ICs take precendence
  for (auto & nl : _nl)
    nl->solution().close();
  _aux->solution().close();

  // now run boundary-restricted initial conditions
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  ComputeBoundaryInitialConditionThread cbic(*this);
  Threads::parallel_reduce(bnd_nodes, cbic);

  for (auto & nl : _nl)
    nl->solution().close();
  _aux->solution().close();

  // Also, load values into the SCALAR dofs
  // Note: We assume that all SCALAR dofs are on the
  // processor with highest ID
  if (processor_id() == (n_processors() - 1) && _scalar_ics.hasActiveObjects())
  {
    const auto & ics = _scalar_ics.getActiveObjects();
    for (const auto & ic : ics)
    {
      MooseVariableScalar & var = ic->variable();
      var.reinit();

      DenseVector<Number> vals(var.order());
      ic->compute(vals);

      const unsigned int n_SCALAR_dofs = var.dofIndices().size();
      for (unsigned int i = 0; i < n_SCALAR_dofs; i++)
      {
        const dof_id_type global_index = var.dofIndices()[i];
        var.sys().solution().set(global_index, vals(i));
        var.setValue(i, vals(i));
      }
    }
  }

  for (auto & sys : _solver_systems)
  {
    sys->solution().close();
    sys->solution().localize(*sys->system().current_local_solution, sys->dofMap().get_send_list());
  }

  _aux->solution().close();
  _aux->solution().localize(*_aux->sys().current_local_solution, _aux->dofMap().get_send_list());
}

void
FEProblemBase::projectInitialConditionOnCustomRange(ConstElemRange & elem_range,
                                                    ConstBndNodeRange & bnd_nodes)
{
  ComputeInitialConditionThread cic(*this);
  Threads::parallel_reduce(elem_range, cic);

  // Need to close the solution vector here so that boundary ICs take precendence
  for (auto & nl : _nl)
    nl->solution().close();
  _aux->solution().close();

  ComputeBoundaryInitialConditionThread cbic(*this);
  Threads::parallel_reduce(bnd_nodes, cbic);

  for (auto & nl : _nl)
    nl->solution().close();
  _aux->solution().close();

  // Also, load values into the SCALAR dofs
  // Note: We assume that all SCALAR dofs are on the
  // processor with highest ID
  if (processor_id() == (n_processors() - 1) && _scalar_ics.hasActiveObjects())
  {
    const auto & ics = _scalar_ics.getActiveObjects();
    for (const auto & ic : ics)
    {
      MooseVariableScalar & var = ic->variable();
      var.reinit();

      DenseVector<Number> vals(var.order());
      ic->compute(vals);

      const unsigned int n_SCALAR_dofs = var.dofIndices().size();
      for (unsigned int i = 0; i < n_SCALAR_dofs; i++)
      {
        const dof_id_type global_index = var.dofIndices()[i];
        var.sys().solution().set(global_index, vals(i));
        var.setValue(i, vals(i));
      }
    }
  }

  for (auto & nl : _nl)
  {
    nl->solution().close();
    nl->solution().localize(*nl->system().current_local_solution, nl->dofMap().get_send_list());
  }

  _aux->solution().close();
  _aux->solution().localize(*_aux->sys().current_local_solution, _aux->dofMap().get_send_list());
}

std::shared_ptr<MaterialBase>
FEProblemBase::getMaterial(std::string name,
                           Moose::MaterialDataType type,
                           const THREAD_ID tid,
                           bool no_warn)
{
  switch (type)
  {
    case Moose::NEIGHBOR_MATERIAL_DATA:
      name += "_neighbor";
      break;
    case Moose::FACE_MATERIAL_DATA:
      name += "_face";
      break;
    default:
      break;
  }

  std::shared_ptr<MaterialBase> material = _all_materials[type].getActiveObject(name, tid);
  if (!no_warn && material->getParam<bool>("compute") && type == Moose::BLOCK_MATERIAL_DATA)
    mooseWarning("You are retrieving a Material object (",
                 material->name(),
                 "), but its compute flag is set to true. This indicates that MOOSE is "
                 "computing this property which may not be desired and produce un-expected "
                 "results.");

  return material;
}

MaterialData &
FEProblemBase::getMaterialData(Moose::MaterialDataType type, const THREAD_ID tid)
{
  switch (type)
  {
    case Moose::BLOCK_MATERIAL_DATA:
      return _material_props.getMaterialData(tid);
    case Moose::NEIGHBOR_MATERIAL_DATA:
      return _neighbor_material_props.getMaterialData(tid);
    case Moose::BOUNDARY_MATERIAL_DATA:
    case Moose::FACE_MATERIAL_DATA:
    case Moose::INTERFACE_MATERIAL_DATA:
      return _bnd_material_props.getMaterialData(tid);
  }

  mooseError("FEProblemBase::getMaterialData(): Invalid MaterialDataType ", type);
}

void
FEProblemBase::setPreserveMatrixSparsityPattern(bool preserve)
{
  if (_ignore_zeros_in_jacobian && preserve)
    paramWarning(
        "ignore_zeros_in_jacobian",
        "We likely cannot preserve the sparsity pattern if ignoring zeros in the Jacobian, which "
        "leads to removing those entries from the Jacobian sparsity pattern");
  _preserve_matrix_sparsity_pattern = preserve;
}

bool
FEProblemBase::acceptInvalidSolution() const
{
  return allowInvalidSolution() || // invalid solutions are always allowed
         !_app.solutionInvalidity().hasInvalidSolutionError(); // if not allowed, check for errors
}

void
FEProblemBase::addFunctorMaterial(const std::string & functor_material_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  parallel_object_only();

  auto add_functor_materials = [&](const auto & parameters, const auto & name)
  {
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      // Create the general Block/Boundary MaterialBase object
      std::shared_ptr<MaterialBase> material =
          _factory.create<MaterialBase>(functor_material_name, name, parameters, tid);
      logAdd("FunctorMaterial", name, functor_material_name, parameters);
      _all_materials.addObject(material, tid);
      _materials.addObject(material, tid);
    }
  };

  parameters.set<SubProblem *>("_subproblem") = this;
  add_functor_materials(parameters, name);
  if (_displaced_problem)
  {
    auto disp_params = parameters;
    disp_params.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    add_functor_materials(disp_params, name + "_displaced");
  }
}

void
FEProblemBase::addMaterial(const std::string & mat_name,
                           const std::string & name,
                           InputParameters & parameters)
{
  addMaterialHelper({&_materials}, mat_name, name, parameters);
}

void
FEProblemBase::addInterfaceMaterial(const std::string & mat_name,
                                    const std::string & name,
                                    InputParameters & parameters)
{
  addMaterialHelper({&_interface_materials}, mat_name, name, parameters);
}

void
FEProblemBase::addMaterialHelper(std::vector<MaterialWarehouse *> warehouses,
                                 const std::string & mat_name,
                                 const std::string & name,
                                 InputParameters & parameters)
{
  parallel_object_only();

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    _reinit_displaced_elem = _reinit_displaced_face = _reinit_displaced_neighbor = true;
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Materials to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Material.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the general Block/Boundary MaterialBase object
    std::shared_ptr<MaterialBase> material =
        _factory.create<MaterialBase>(mat_name, name, parameters, tid);
    logAdd("Material", name, mat_name, parameters);
    bool discrete = !material->getParam<bool>("compute");

    // If the object is boundary restricted or if it is a functor material we do not create the
    // neighbor and face objects
    if (material->boundaryRestricted() || dynamic_cast<FunctorMaterial *>(material.get()))
    {
      _all_materials.addObject(material, tid);
      if (discrete)
        _discrete_materials.addObject(material, tid);
      else
        for (auto && warehouse : warehouses)
          warehouse->addObject(material, tid);
    }

    // Non-boundary restricted require face and neighbor objects
    else
    {
      // TODO: we only need to do this if we have needs for face materials (e.g.
      // FV, DG, etc.) - but currently we always do it.  Figure out how to fix
      // this.

      // The name of the object being created, this is changed multiple times as objects are
      // created below
      std::string object_name;

      // Create a copy of the supplied parameters to the setting for "_material_data_type" isn't
      // used from a previous tid loop
      InputParameters current_parameters = parameters;

      // face material
      current_parameters.set<Moose::MaterialDataType>("_material_data_type") =
          Moose::FACE_MATERIAL_DATA;
      object_name = name + "_face";
      std::shared_ptr<MaterialBase> face_material =
          _factory.create<MaterialBase>(mat_name, object_name, current_parameters, tid);

      // neighbor material
      current_parameters.set<Moose::MaterialDataType>("_material_data_type") =
          Moose::NEIGHBOR_MATERIAL_DATA;
      current_parameters.set<bool>("_neighbor") = true;
      object_name = name + "_neighbor";
      std::shared_ptr<MaterialBase> neighbor_material =
          _factory.create<MaterialBase>(mat_name, object_name, current_parameters, tid);

      // Store the material objects
      _all_materials.addObjects(material, neighbor_material, face_material, tid);

      if (discrete)
        _discrete_materials.addObjects(material, neighbor_material, face_material, tid);
      else
        for (auto && warehouse : warehouses)
          warehouse->addObjects(material, neighbor_material, face_material, tid);

      // Names of all controllable parameters for this Material object
      const std::string & base = parameters.get<std::string>("_moose_base");
      MooseObjectParameterName name(MooseObjectName(base, material->name()), "*");
      const auto param_names =
          _app.getInputParameterWarehouse().getControllableParameterNames(name);

      // Connect parameters of the primary Material object to those on the face and neighbor
      // objects
      for (const auto & p_name : param_names)
      {
        MooseObjectParameterName primary_name(MooseObjectName(base, material->name()),
                                              p_name.parameter());
        MooseObjectParameterName face_name(MooseObjectName(base, face_material->name()),
                                           p_name.parameter());
        MooseObjectParameterName neighbor_name(MooseObjectName(base, neighbor_material->name()),
                                               p_name.parameter());
        _app.getInputParameterWarehouse().addControllableParameterConnection(
            primary_name, face_name, false);
        _app.getInputParameterWarehouse().addControllableParameterConnection(
            primary_name, neighbor_name, false);
      }
    }
  }
}

void
FEProblemBase::prepareMaterials(const std::unordered_set<unsigned int> & consumer_needed_mat_props,
                                const SubdomainID blk_id,
                                const THREAD_ID tid)
{
  std::set<MooseVariableFEBase *> needed_moose_vars;
  std::unordered_set<unsigned int> needed_mat_props;

  if (_all_materials.hasActiveBlockObjects(blk_id, tid))
  {
    _all_materials.updateVariableDependency(needed_moose_vars, tid);
    _all_materials.updateBlockMatPropDependency(blk_id, needed_mat_props, tid);
  }

  const auto & ids = _mesh.getSubdomainBoundaryIds(blk_id);
  for (const auto id : ids)
  {
    _materials.updateBoundaryVariableDependency(id, needed_moose_vars, tid);
    _materials.updateBoundaryMatPropDependency(id, needed_mat_props, tid);
  }

  const auto & current_active_elemental_moose_variables = getActiveElementalMooseVariables(tid);
  needed_moose_vars.insert(current_active_elemental_moose_variables.begin(),
                           current_active_elemental_moose_variables.end());

  needed_mat_props.insert(consumer_needed_mat_props.begin(), consumer_needed_mat_props.end());

  setActiveElementalMooseVariables(needed_moose_vars, tid);
  setActiveMaterialProperties(needed_mat_props, tid);
}

void
FEProblemBase::reinitMaterials(SubdomainID blk_id, const THREAD_ID tid, bool swap_stateful)
{
  if (hasActiveMaterialProperties(tid))
  {
    auto && elem = _assembly[tid][0]->elem();
    unsigned int n_points = _assembly[tid][0]->qRule()->n_points();

    auto & material_data = _material_props.getMaterialData(tid);
    material_data.resize(n_points);

    // Only swap if requested
    if (swap_stateful)
      material_data.swap(*elem);

    if (_discrete_materials.hasActiveBlockObjects(blk_id, tid))
      material_data.reset(_discrete_materials.getActiveBlockObjects(blk_id, tid));

    if (_materials.hasActiveBlockObjects(blk_id, tid))
      material_data.reinit(_materials.getActiveBlockObjects(blk_id, tid));
  }
}

void
FEProblemBase::reinitMaterialsFace(const SubdomainID blk_id,
                                   const THREAD_ID tid,
                                   const bool swap_stateful,
                                   const std::deque<MaterialBase *> * const reinit_mats)
{
  if (hasActiveMaterialProperties(tid))
  {
    auto && elem = _assembly[tid][0]->elem();
    unsigned int side = _assembly[tid][0]->side();
    unsigned int n_points = _assembly[tid][0]->qRuleFace()->n_points();

    auto & bnd_material_data = _bnd_material_props.getMaterialData(tid);
    bnd_material_data.resize(n_points);

    if (swap_stateful && !bnd_material_data.isSwapped())
      bnd_material_data.swap(*elem, side);

    if (_discrete_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      bnd_material_data.reset(
          _discrete_materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));

    if (reinit_mats)
      bnd_material_data.reinit(*reinit_mats);
    else if (_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      bnd_material_data.reinit(
          _materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));
  }
}

void
FEProblemBase::reinitMaterialsNeighbor(const SubdomainID blk_id,
                                       const THREAD_ID tid,
                                       const bool swap_stateful,
                                       const std::deque<MaterialBase *> * const reinit_mats)
{
  if (hasActiveMaterialProperties(tid))
  {
    // NOTE: this will not work with h-adaptivity
    // lindsayad: why not?

    const Elem * neighbor = _assembly[tid][0]->neighbor();
    unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[tid][0]->elem());

    mooseAssert(neighbor, "neighbor should be non-null");
    mooseAssert(blk_id == neighbor->subdomain_id(),
                "The provided blk_id " << blk_id << " and neighbor subdomain ID "
                                       << neighbor->subdomain_id() << " do not match.");

    unsigned int n_points = _assembly[tid][0]->qRuleNeighbor()->n_points();

    auto & neighbor_material_data = _neighbor_material_props.getMaterialData(tid);
    neighbor_material_data.resize(n_points);

    // Only swap if requested
    if (swap_stateful)
      neighbor_material_data.swap(*neighbor, neighbor_side);

    if (_discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      neighbor_material_data.reset(
          _discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));

    if (reinit_mats)
      neighbor_material_data.reinit(*reinit_mats);
    else if (_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      neighbor_material_data.reinit(
          _materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));
  }
}

void
FEProblemBase::reinitMaterialsBoundary(const BoundaryID boundary_id,
                                       const THREAD_ID tid,
                                       const bool swap_stateful,
                                       const std::deque<MaterialBase *> * const reinit_mats)
{
  if (hasActiveMaterialProperties(tid))
  {
    auto && elem = _assembly[tid][0]->elem();
    unsigned int side = _assembly[tid][0]->side();
    unsigned int n_points = _assembly[tid][0]->qRuleFace()->n_points();

    auto & bnd_material_data = _bnd_material_props.getMaterialData(tid);
    bnd_material_data.resize(n_points);

    if (swap_stateful && !bnd_material_data.isSwapped())
      bnd_material_data.swap(*elem, side);

    if (_discrete_materials.hasActiveBoundaryObjects(boundary_id, tid))
      bnd_material_data.reset(_discrete_materials.getActiveBoundaryObjects(boundary_id, tid));

    if (reinit_mats)
      bnd_material_data.reinit(*reinit_mats);
    else if (_materials.hasActiveBoundaryObjects(boundary_id, tid))
      bnd_material_data.reinit(_materials.getActiveBoundaryObjects(boundary_id, tid));
  }
}

void
FEProblemBase::reinitMaterialsInterface(BoundaryID boundary_id,
                                        const THREAD_ID tid,
                                        bool swap_stateful)
{
  if (hasActiveMaterialProperties(tid))
  {
    const Elem * const & elem = _assembly[tid][0]->elem();
    unsigned int side = _assembly[tid][0]->side();
    unsigned int n_points = _assembly[tid][0]->qRuleFace()->n_points();

    auto & bnd_material_data = _bnd_material_props.getMaterialData(tid);
    bnd_material_data.resize(n_points);

    if (swap_stateful && !bnd_material_data.isSwapped())
      bnd_material_data.swap(*elem, side);

    if (_interface_materials.hasActiveBoundaryObjects(boundary_id, tid))
      bnd_material_data.reinit(_interface_materials.getActiveBoundaryObjects(boundary_id, tid));
  }
}

void
FEProblemBase::swapBackMaterials(const THREAD_ID tid)
{
  auto && elem = _assembly[tid][0]->elem();
  _material_props.getMaterialData(tid).swapBack(*elem);
}

void
FEProblemBase::swapBackMaterialsFace(const THREAD_ID tid)
{
  auto && elem = _assembly[tid][0]->elem();
  unsigned int side = _assembly[tid][0]->side();
  _bnd_material_props.getMaterialData(tid).swapBack(*elem, side);
}

void
FEProblemBase::swapBackMaterialsNeighbor(const THREAD_ID tid)
{
  // NOTE: this will not work with h-adaptivity
  const Elem * neighbor = _assembly[tid][0]->neighbor();
  unsigned int neighbor_side =
      neighbor ? neighbor->which_neighbor_am_i(_assembly[tid][0]->elem()) : libMesh::invalid_uint;

  if (!neighbor)
  {
    if (haveFV())
    {
      // If neighbor is null, then we're on the neighbor side of a mesh boundary, e.g. we're off
      // the mesh in ghost-land. If we're using the finite volume method, then variable values and
      // consequently material properties have well-defined values in this ghost region outside of
      // the mesh and we really do want to reinit our neighbor materials in this case. Since we're
      // off in ghost land it's safe to do swaps with `MaterialPropertyStorage` using the elem and
      // elem_side keys
      neighbor = _assembly[tid][0]->elem();
      neighbor_side = _assembly[tid][0]->side();
      mooseAssert(neighbor, "We should have an appropriate value for elem coming from Assembly");
    }
    else
      mooseError("neighbor is null in Assembly!");
  }

  _neighbor_material_props.getMaterialData(tid).swapBack(*neighbor, neighbor_side);
}

void
FEProblemBase::logAdd(const std::string & system,
                      const std::string & name,
                      const std::string & type,
                      const InputParameters & params) const
{
  if (_verbose_setup != "false")
    _console << "[DBG] Adding " << system << " '" << name << "' of type " << type << std::endl;
  if (_verbose_setup == "extra")
    _console << params << std::endl;
}

void
FEProblemBase::addObjectParamsHelper(InputParameters & parameters,
                                     const std::string & object_name,
                                     const std::string & var_param_name)
{
  const auto solver_sys_num =
      parameters.isParamValid(var_param_name) &&
              determineSolverSystem(parameters.varName(var_param_name, object_name)).first
          ? determineSolverSystem(parameters.varName(var_param_name, object_name)).second
          : (unsigned int)0;

  if (_displaced_problem && parameters.have_parameter<bool>("use_displaced_mesh") &&
      parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->solverSys(solver_sys_num);
  }
  else
  {
    // The object requested use_displaced_mesh, but it was overridden
    // due to there being no displacements variables in the [Mesh] block.
    // If that happened, update the value of use_displaced_mesh appropriately.
    if (!_displaced_problem && parameters.have_parameter<bool>("use_displaced_mesh") &&
        parameters.get<bool>("use_displaced_mesh"))
      parameters.set<bool>("use_displaced_mesh") = false;

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _solver_systems[solver_sys_num].get();
  }
}

void
FEProblemBase::addPostprocessor(const std::string & pp_name,
                                const std::string & name,
                                InputParameters & parameters)
{
  // Check for name collision
  if (hasUserObject(name))
    mooseError("A UserObject with the name \"",
               name,
               "\" already exists.  You may not add a Postprocessor by the same name.");

  addUserObject(pp_name, name, parameters);
}

void
FEProblemBase::addVectorPostprocessor(const std::string & pp_name,
                                      const std::string & name,
                                      InputParameters & parameters)
{
  // Check for name collision
  if (hasUserObject(name))
    mooseError("A UserObject with the name \"",
               name,
               "\" already exists.  You may not add a VectorPostprocessor by the same name.");

  addUserObject(pp_name, name, parameters);
}

void
FEProblemBase::addReporter(const std::string & type,
                           const std::string & name,
                           InputParameters & parameters)
{
  // Check for name collision
  if (hasUserObject(name))
    mooseError(std::string("A UserObject with the name \"") + name +
               "\" already exists.  You may not add a Reporter by the same name.");

  addUserObject(type, name, parameters);
}

std::vector<std::shared_ptr<UserObject>>
FEProblemBase::addUserObject(const std::string & user_object_name,
                             const std::string & name,
                             InputParameters & parameters)
{
  parallel_object_only();

  std::vector<std::shared_ptr<UserObject>> uos;

  // Add the _subproblem and _sys parameters depending on use_displaced_mesh
  addObjectParamsHelper(parameters, name);

  for (const auto tid : make_range(libMesh::n_threads()))
  {
    // Create the UserObject
    std::shared_ptr<UserObject> user_object =
        _factory.create<UserObject>(user_object_name, name, parameters, tid);
    logAdd("UserObject", name, user_object_name, parameters);
    uos.push_back(user_object);

    if (tid != 0)
      user_object->setPrimaryThreadCopy(uos[0].get());

    // TODO: delete this line after apps have been updated to not call getUserObjects
    _all_user_objects.addObject(user_object, tid);

    theWarehouse().add(user_object);

    // Attempt to create all the possible UserObject types
    auto euo = std::dynamic_pointer_cast<ElementUserObject>(user_object);
    auto suo = std::dynamic_pointer_cast<SideUserObject>(user_object);
    auto isuo = std::dynamic_pointer_cast<InternalSideUserObject>(user_object);
    auto iuob = std::dynamic_pointer_cast<InterfaceUserObjectBase>(user_object);
    auto nuo = std::dynamic_pointer_cast<NodalUserObject>(user_object);
    auto duo = std::dynamic_pointer_cast<DomainUserObject>(user_object);
    auto guo = std::dynamic_pointer_cast<GeneralUserObject>(user_object);
    auto tguo = std::dynamic_pointer_cast<ThreadedGeneralUserObject>(user_object);
    auto muo = std::dynamic_pointer_cast<MortarUserObject>(user_object);

    // Account for displaced mesh use
    if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
    {
      if (euo || nuo || duo)
        _reinit_displaced_elem = true;
      else if (suo)
        // shouldn't we add isuo
        _reinit_displaced_face = true;
      else if (iuob)
        _reinit_displaced_neighbor = true;
    }

    // These objects only require one thread
    if ((guo && !tguo) || muo)
      break;
  }

  // Add as a Functor if it is one
  // At the timing of adding this, this is only Postprocessors... but technically it
  // should enable any UO that is a Real Functor to be used as one
  // The ternary operator used in getting the functor is there because some UOs
  // are threaded and some are not. When a UO is not threaded, we need to add
  // the functor from thread 0 as the registered functor for all threads
  for (const auto tid : make_range(libMesh::n_threads()))
    if (const auto functor =
            dynamic_cast<Moose::FunctorBase<Real> *>(uos[uos.size() == 1 ? 0 : tid].get()))
    {
      this->addFunctor(name, *functor, tid);
      if (_displaced_problem)
        _displaced_problem->addFunctor(name, *functor, tid);
    }

  return uos;
}

const UserObject &
FEProblemBase::getUserObjectBase(const std::string & name, const THREAD_ID tid /* = 0 */) const
{
  std::vector<UserObject *> objs;
  theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(tid)
      .condition<AttribName>(name)
      .queryInto(objs);
  if (objs.empty())
    mooseError("Unable to find user object with name '" + name + "'");
  mooseAssert(objs.size() == 1, "Should only find one UO");
  return *(objs[0]);
}

const Positions &
FEProblemBase::getPositionsObject(const std::string & name) const
{
  std::vector<Positions *> objs;
  theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribName>(name)
      .queryInto(objs);
  if (objs.empty())
    mooseError("Unable to find Positions object with name '" + name + "'");
  mooseAssert(objs.size() == 1, "Should only find one Positions");
  return *(objs[0]);
}

bool
FEProblemBase::hasUserObject(const std::string & name) const
{
  std::vector<UserObject *> objs;
  theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .condition<AttribName>(name)
      .queryInto(objs);
  return !objs.empty();
}

bool
FEProblemBase::hasPostprocessorValueByName(const PostprocessorName & name) const
{
  return _reporter_data.hasReporterValue<PostprocessorValue>(PostprocessorReporterName(name));
}

const PostprocessorValue &
FEProblemBase::getPostprocessorValueByName(const PostprocessorName & name,
                                           std::size_t t_index) const
{
  return _reporter_data.getReporterValue<PostprocessorValue>(PostprocessorReporterName(name),
                                                             t_index);
}

void
FEProblemBase::setPostprocessorValueByName(const PostprocessorName & name,
                                           const PostprocessorValue & value,
                                           std::size_t t_index)
{
  _reporter_data.setReporterValue<PostprocessorValue>(
      PostprocessorReporterName(name), value, t_index);
}

bool
FEProblemBase::hasPostprocessor(const std::string & name) const
{
  mooseDeprecated("FEProblemBase::hasPostprocssor is being removed; use "
                  "hasPostprocessorValueByName instead.");
  return hasPostprocessorValueByName(name);
}

const VectorPostprocessorValue &
FEProblemBase::getVectorPostprocessorValueByName(const std::string & object_name,
                                                 const std::string & vector_name,
                                                 std::size_t t_index) const
{
  return _reporter_data.getReporterValue<VectorPostprocessorValue>(
      VectorPostprocessorReporterName(object_name, vector_name), t_index);
}

void
FEProblemBase::setVectorPostprocessorValueByName(const std::string & object_name,
                                                 const std::string & vector_name,
                                                 const VectorPostprocessorValue & value,
                                                 std::size_t t_index)
{
  _reporter_data.setReporterValue<VectorPostprocessorValue>(
      VectorPostprocessorReporterName(object_name, vector_name), value, t_index);
}

const VectorPostprocessor &
FEProblemBase::getVectorPostprocessorObjectByName(const std::string & object_name,
                                                  const THREAD_ID tid) const
{
  return getUserObject<VectorPostprocessor>(object_name, tid);
}

void
FEProblemBase::parentOutputPositionChanged()
{
  for (const auto & it : _multi_apps)
  {
    const auto & objects = it.second.getActiveObjects();
    for (const auto & obj : objects)
      obj->parentOutputPositionChanged();
  }
}

void
FEProblemBase::computeIndicatorsAndMarkers()
{
  computeIndicators();
  computeMarkers();
}

void
FEProblemBase::computeIndicators()
{
  // Initialize indicator aux variable fields
  if (_indicators.hasActiveObjects() || _internal_side_indicators.hasActiveObjects())
  {
    TIME_SECTION("computeIndicators", 1, "Computing Indicators");

    // Internal side indicators may lead to creating a much larger sparsity pattern than dictated by
    // the actual finite element scheme (e.g. CFEM)
    const auto old_do_derivatives = ADReal::do_derivatives;
    ADReal::do_derivatives = false;

    std::vector<std::string> fields;

    // Indicator Fields
    const auto & indicators = _indicators.getActiveObjects();
    for (const auto & indicator : indicators)
      fields.push_back(indicator->name());

    // InternalSideIndicator Fields
    const auto & internal_indicators = _internal_side_indicators.getActiveObjects();
    for (const auto & internal_indicator : internal_indicators)
      fields.push_back(internal_indicator->name());

    _aux->zeroVariables(fields);

    // compute Indicators
    ComputeIndicatorThread cit(*this);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cit);
    _aux->solution().close();
    _aux->update();

    ComputeIndicatorThread finalize_cit(*this, true);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), finalize_cit);
    _aux->solution().close();
    _aux->update();

    ADReal::do_derivatives = old_do_derivatives;
  }
}

void
FEProblemBase::computeMarkers()
{
  if (_markers.hasActiveObjects())
  {
    TIME_SECTION("computeMarkers", 1, "Computing Markers");

    std::vector<std::string> fields;

    // Marker Fields
    const auto & markers = _markers.getActiveObjects();
    for (const auto & marker : markers)
      fields.push_back(marker->name());

    _aux->zeroVariables(fields);

    _adaptivity.updateErrorVectors();

    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      const auto & markers = _markers.getActiveObjects(tid);
      for (const auto & marker : markers)
        marker->markerSetup();
    }

    ComputeMarkerThread cmt(*this);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cmt);

    _aux->solution().close();
    _aux->update();
  }
}

const ExecFlagType &
FEProblemBase::getCurrentExecuteOnFlag() const
{
  return _current_execute_on_flag;
}

void
FEProblemBase::setCurrentExecuteOnFlag(const ExecFlagType & flag)
{
  _current_execute_on_flag = flag;
}

void
FEProblemBase::executeAllObjects(const ExecFlagType & /*exec_type*/)
{
}

void
FEProblemBase::customSetup(const ExecFlagType & exec_type)
{
  SubProblem::customSetup(exec_type);

  if (_line_search)
    _line_search->customSetup(exec_type);

  unsigned int n_threads = libMesh::n_threads();
  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _all_materials.customSetup(exec_type, tid);
    _functions.customSetup(exec_type, tid);
  }

  _aux->customSetup(exec_type);
  for (auto & nl : _nl)
    nl->customSetup(exec_type);

  if (_displaced_problem)
    _displaced_problem->customSetup(exec_type);

  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _internal_side_indicators.customSetup(exec_type, tid);
    _indicators.customSetup(exec_type, tid);
    _markers.customSetup(exec_type, tid);
  }

  std::vector<UserObject *> userobjs;
  theWarehouse().query().condition<AttribSystem>("UserObject").queryIntoUnsorted(userobjs);
  for (auto obj : userobjs)
    obj->customSetup(exec_type);

  _app.getOutputWarehouse().customSetup(exec_type);
}

void
FEProblemBase::execute(const ExecFlagType & exec_type)
{
  // Set the current flag
  setCurrentExecuteOnFlag(exec_type);

  if (exec_type != EXEC_INITIAL)
    executeControls(exec_type);

  // intentially call this after executing controls because the setups may rely on the controls
  // FIXME: we skip the following flags because they have dedicated setup functions in
  //        SetupInterface and it may not be appropriate to call them here.
  if (!(exec_type == EXEC_INITIAL || exec_type == EXEC_TIMESTEP_BEGIN ||
        exec_type == EXEC_SUBDOMAIN || exec_type == EXEC_NONLINEAR || exec_type == EXEC_LINEAR))
    customSetup(exec_type);

  // Samplers; EXEC_INITIAL is not called because the Sampler::init() method that is called after
  // construction makes the first Sampler::execute() call. This ensures that the random number
  // generator object is the correct state prior to any other object (e.g., Transfers) attempts to
  // extract data from the Sampler. That is, if the Sampler::execute() call is delayed to here
  // then it is not in the correct state for other objects.
  if (exec_type != EXEC_INITIAL)
    executeSamplers(exec_type);

  // Pre-aux UserObjects
  computeUserObjects(exec_type, Moose::PRE_AUX);

  // Systems (includes system time derivative and aux kernel calculations)
  computeSystems(exec_type);

  // Post-aux UserObjects
  computeUserObjects(exec_type, Moose::POST_AUX);

  // Return the current flag to None
  setCurrentExecuteOnFlag(EXEC_NONE);

  if (_uo_aux_state_check && !_checking_uo_aux_state)
  {
    // we will only check aux variables and postprocessors
    // checking more reporter data can be added in the future if needed
    std::unique_ptr<NumericVector<Number>> x = _aux->currentSolution()->clone();
    DenseVector<Real> pp_values = getReporterData().getAllRealReporterValues();

    // call THIS execute one more time for checking the possible states
    _checking_uo_aux_state = true;
    FEProblemBase::execute(exec_type);
    _checking_uo_aux_state = false;

    const Real check_tol = 1e-8;

    const Real xnorm = x->l2_norm();
    *x -= *_aux->currentSolution();
    if (x->l2_norm() > check_tol * xnorm)
    {
      const auto & sys = _aux->system();
      const unsigned int n_vars = sys.n_vars();
      std::multimap<Real, std::string, std::greater<Real>> ordered_map;
      for (const auto i : make_range(n_vars))
      {
        const Real vnorm = sys.calculate_norm(*x, i, DISCRETE_L2);
        ordered_map.emplace(vnorm, sys.variable_name(i));
      }

      std::ostringstream oss;
      for (const auto & [error_norm, var_name] : ordered_map)
        oss << "  {" << var_name << ", " << error_norm << "},\n";

      mooseError("Aux kernels, user objects appear to have states for aux variables on ",
                 exec_type,
                 ".\nVariable error norms in descending order:\n",
                 oss.str());
    }

    const DenseVector<Real> new_pp_values = getReporterData().getAllRealReporterValues();
    if (pp_values.size() != new_pp_values.size())
      mooseError("Second execution for uo/aux state check should not change the number of "
                 "real reporter values");

    const Real ppnorm = pp_values.l2_norm();
    pp_values -= new_pp_values;
    if (pp_values.l2_norm() > check_tol * ppnorm)
    {
      const auto pp_names = getReporterData().getAllRealReporterFullNames();
      std::multimap<Real, std::string, std::greater<Real>> ordered_map;
      for (const auto i : index_range(pp_names))
        ordered_map.emplace(std::abs(pp_values(i)), pp_names[i]);

      std::ostringstream oss;
      for (const auto & [error_norm, pp_name] : ordered_map)
        oss << "  {" << pp_name << ", " << error_norm << "},\n";

      mooseError("Aux kernels, user objects appear to have states for real reporter values on ",
                 exec_type,
                 ".\nErrors of real reporter values in descending order:\n",
                 oss.str());
    }
  }
}

// Finalize, threadJoin, and update PP values of Elemental/Nodal/Side/InternalSideUserObjects
void
FEProblemBase::joinAndFinalize(TheWarehouse::Query query, bool isgen)
{
  std::vector<UserObject *> objs;
  query.queryInto(objs);
  if (!isgen)
  {
    // join all threaded user objects (i.e. not regular general user objects) to the primary
    // thread
    for (auto obj : objs)
      if (obj->primaryThreadCopy())
        obj->primaryThreadCopy()->threadJoin(*obj);
  }

  query.condition<AttribThread>(0).queryInto(objs);

  // finalize objects and retrieve/store any postprocessor values
  for (auto obj : objs)
  {
    if (isgen && dynamic_cast<ThreadedGeneralUserObject *>(obj))
      continue;
    if (isgen)
    {
      // general user objects are not run in their own threaded loop object - so run them here
      if (shouldPrintExecution(0))
        _console << "[DBG] Initializing, executing & finalizing general UO '" << obj->name()
                 << "' on " << _current_execute_on_flag.name() << std::endl;
      obj->initialize();
      obj->execute();
    }

    obj->finalize();

    // These have to be stored piecemeal (with every call to this function) because general
    // postprocessors (which run last after other userobjects have been completed) might depend on
    // them being stored.  This wouldn't be a problem if all userobjects satisfied the dependency
    // resolver interface and could be sorted appropriately with the general userobjects, but they
    // don't.
    auto pp = dynamic_cast<const Postprocessor *>(obj);
    if (pp)
    {
      _reporter_data.finalize(obj->name());
      setPostprocessorValueByName(obj->name(), pp->getValue());
    }

    auto vpp = dynamic_cast<VectorPostprocessor *>(obj);
    if (vpp)
      _reporter_data.finalize(obj->name());

    // Update Reporter data
    auto reporter = dynamic_cast<Reporter *>(obj);
    if (reporter)
      _reporter_data.finalize(obj->name());
  }
}

void
FEProblemBase::computeUserObjectByName(const ExecFlagType & type,
                                       const Moose::AuxGroup & group,
                                       const std::string & name)
{
  const auto old_exec_flag = _current_execute_on_flag;
  _current_execute_on_flag = type;
  TheWarehouse::Query query = theWarehouse()
                                  .query()
                                  .condition<AttribSystem>("UserObject")
                                  .condition<AttribExecOns>(type)
                                  .condition<AttribName>(name);
  computeUserObjectsInternal(type, group, query);
  _current_execute_on_flag = old_exec_flag;
}

void
FEProblemBase::computeUserObjects(const ExecFlagType & type, const Moose::AuxGroup & group)
{
  TheWarehouse::Query query =
      theWarehouse().query().condition<AttribSystem>("UserObject").condition<AttribExecOns>(type);
  computeUserObjectsInternal(type, group, query);
}

void
FEProblemBase::computeUserObjectsInternal(const ExecFlagType & type,
                                          const Moose::AuxGroup & group,
                                          TheWarehouse::Query & primary_query)
{
  try
  {
    TIME_SECTION("computeUserObjects", 1, "Computing User Objects");

    // Add group to query
    if (group == Moose::PRE_IC)
      primary_query.condition<AttribPreIC>(true);
    else if (group == Moose::PRE_AUX)
      primary_query.condition<AttribPreAux>(type);
    else if (group == Moose::POST_AUX)
      primary_query.condition<AttribPostAux>(type);

    // query everything first to obtain a list of execution groups
    std::vector<UserObject *> uos;
    primary_query.clone().queryIntoUnsorted(uos);
    std::set<int> execution_groups;
    for (const auto & uo : uos)
      execution_groups.insert(uo->getParam<int>("execution_order_group"));

    // iterate over execution order groups
    for (const auto execution_group : execution_groups)
    {
      auto query = primary_query.clone().condition<AttribExecutionOrderGroup>(execution_group);

      std::vector<GeneralUserObject *> genobjs;
      query.clone().condition<AttribInterfaces>(Interfaces::GeneralUserObject).queryInto(genobjs);

      std::vector<UserObject *> userobjs;
      query.clone()
          .condition<AttribInterfaces>(Interfaces::ElementUserObject | Interfaces::SideUserObject |
                                       Interfaces::InternalSideUserObject |
                                       Interfaces::InterfaceUserObject |
                                       Interfaces::DomainUserObject)
          .queryInto(userobjs);

      std::vector<UserObject *> tgobjs;
      query.clone()
          .condition<AttribInterfaces>(Interfaces::ThreadedGeneralUserObject)
          .queryInto(tgobjs);

      std::vector<UserObject *> nodal;
      query.clone().condition<AttribInterfaces>(Interfaces::NodalUserObject).queryInto(nodal);

      std::vector<MortarUserObject *> mortar;
      query.clone().condition<AttribInterfaces>(Interfaces::MortarUserObject).queryInto(mortar);

      if (userobjs.empty() && genobjs.empty() && tgobjs.empty() && nodal.empty() && mortar.empty())
        continue;

      // Start the timer here since we have at least one active user object
      std::string compute_uo_tag = "computeUserObjects(" + Moose::stringify(type) + ")";

      // Perform Residual/Jacobian setups
      if (type == EXEC_LINEAR)
      {
        for (auto obj : userobjs)
          obj->residualSetup();
        for (auto obj : nodal)
          obj->residualSetup();
        for (auto obj : mortar)
          obj->residualSetup();
        for (auto obj : tgobjs)
          obj->residualSetup();
        for (auto obj : genobjs)
          obj->residualSetup();
      }
      else if (type == EXEC_NONLINEAR)
      {
        for (auto obj : userobjs)
          obj->jacobianSetup();
        for (auto obj : nodal)
          obj->jacobianSetup();
        for (auto obj : mortar)
          obj->jacobianSetup();
        for (auto obj : tgobjs)
          obj->jacobianSetup();
        for (auto obj : genobjs)
          obj->jacobianSetup();
      }

      for (auto obj : userobjs)
        obj->initialize();

      // Execute Side/InternalSide/Interface/Elemental/DomainUserObjects
      if (!userobjs.empty())
      {
        // non-nodal user objects have to be run separately before the nodal user objects run
        // because some nodal user objects (NodalNormal related) depend on elemental user objects
        // :-(
        ComputeUserObjectsThread cppt(*this, query);
        Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cppt);

        // There is one instance in rattlesnake where an elemental user object's finalize depends
        // on a side user object having been finalized first :-(
        joinAndFinalize(query.clone().condition<AttribInterfaces>(Interfaces::SideUserObject));
        joinAndFinalize(
            query.clone().condition<AttribInterfaces>(Interfaces::InternalSideUserObject));
        joinAndFinalize(query.clone().condition<AttribInterfaces>(Interfaces::InterfaceUserObject));
        joinAndFinalize(query.clone().condition<AttribInterfaces>(Interfaces::ElementUserObject));
        joinAndFinalize(query.clone().condition<AttribInterfaces>(Interfaces::DomainUserObject));
      }

      // if any userobject may have written to variables we need to close the aux solution
      for (const auto & uo : userobjs)
        if (auto euo = dynamic_cast<const ElementUserObject *>(uo);
            euo && euo->hasWritableCoupledVariables())
        {
          _aux->solution().close();
          _aux->system().update();
          break;
        }

      // Execute NodalUserObjects
      // BISON has an axial reloc elemental user object that has a finalize func that depends on a
      // nodal user object's prev value. So we can't initialize this until after elemental objects
      // have been finalized :-(
      for (auto obj : nodal)
        obj->initialize();
      if (query.clone().condition<AttribInterfaces>(Interfaces::NodalUserObject).count() > 0)
      {
        ComputeNodalUserObjectsThread cnppt(*this, query);
        Threads::parallel_reduce(*_mesh.getLocalNodeRange(), cnppt);
        joinAndFinalize(query.clone().condition<AttribInterfaces>(Interfaces::NodalUserObject));
      }

      // if any userobject may have written to variables we need to close the aux solution
      for (const auto & uo : nodal)
        if (auto nuo = dynamic_cast<const NodalUserObject *>(uo);
            nuo && nuo->hasWritableCoupledVariables())
        {
          _aux->solution().close();
          _aux->system().update();
          break;
        }

      // Execute MortarUserObjects
      {
        for (auto obj : mortar)
          obj->initialize();
        if (!mortar.empty())
        {
          auto create_and_run_mortar_functors = [this, type, &mortar](const bool displaced)
          {
            // go over mortar interfaces and construct functors
            const auto & mortar_interfaces = getMortarInterfaces(displaced);
            for (const auto & mortar_interface : mortar_interfaces)
            {
              const auto primary_secondary_boundary_pair = mortar_interface.first;
              auto mortar_uos_to_execute =
                  getMortarUserObjects(primary_secondary_boundary_pair.first,
                                       primary_secondary_boundary_pair.second,
                                       displaced,
                                       mortar);
              const auto & mortar_generation_object = mortar_interface.second;

              auto * const subproblem = displaced
                                            ? static_cast<SubProblem *>(_displaced_problem.get())
                                            : static_cast<SubProblem *>(this);
              MortarUserObjectThread muot(mortar_uos_to_execute,
                                          mortar_generation_object,
                                          *subproblem,
                                          *this,
                                          displaced,
                                          subproblem->assembly(0, 0));

              muot();
            }
          };

          create_and_run_mortar_functors(false);
          if (_displaced_problem)
            create_and_run_mortar_functors(true);
        }
        for (auto obj : mortar)
          obj->finalize();
      }

      // Execute threaded general user objects
      for (auto obj : tgobjs)
        obj->initialize();
      std::vector<GeneralUserObject *> tguos_zero;
      query.clone()
          .condition<AttribThread>(0)
          .condition<AttribInterfaces>(Interfaces::ThreadedGeneralUserObject)
          .queryInto(tguos_zero);
      for (auto obj : tguos_zero)
      {
        std::vector<GeneralUserObject *> tguos;
        auto q = query.clone()
                     .condition<AttribName>(obj->name())
                     .condition<AttribInterfaces>(Interfaces::ThreadedGeneralUserObject);
        q.queryInto(tguos);

        ComputeThreadedGeneralUserObjectsThread ctguot(*this);
        Threads::parallel_reduce(GeneralUserObjectRange(tguos.begin(), tguos.end()), ctguot);
        joinAndFinalize(q);
      }

      // Execute general user objects
      joinAndFinalize(query.clone().condition<AttribInterfaces>(Interfaces::GeneralUserObject),
                      true);
    }
  }
  catch (...)
  {
    handleException("computeUserObjectsInternal");
  }
}

void
FEProblemBase::executeControls(const ExecFlagType & exec_type)
{
  if (_control_warehouse[exec_type].hasActiveObjects())
  {
    TIME_SECTION("executeControls", 1, "Executing Controls");

    DependencyResolver<std::shared_ptr<Control>> resolver;

    auto controls_wh = _control_warehouse[exec_type];
    // Add all of the dependencies into the resolver and sort them
    for (const auto & it : controls_wh.getActiveObjects())
    {
      // Make sure an item with no dependencies comes out too!
      resolver.addItem(it);

      std::vector<std::string> & dependent_controls = it->getDependencies();
      for (const auto & depend_name : dependent_controls)
      {
        if (controls_wh.hasActiveObject(depend_name))
        {
          auto dep_control = controls_wh.getActiveObject(depend_name);
          resolver.addEdge(dep_control, it);
        }
        else
          mooseError("The Control \"",
                     depend_name,
                     "\" was not created, did you make a "
                     "spelling mistake or forget to include it "
                     "in your input file?");
      }
    }

    const auto & ordered_controls = resolver.getSortedValues();

    if (!ordered_controls.empty())
    {
      _control_warehouse.setup(exec_type);
      // Run the controls in the proper order
      for (const auto & control : ordered_controls)
        control->execute();
    }
  }
}

void
FEProblemBase::executeSamplers(const ExecFlagType & exec_type)
{
  // TODO: This should be done in a threaded loop, but this should be super quick so for now
  // do a serial loop.
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    std::vector<Sampler *> objects;
    theWarehouse()
        .query()
        .condition<AttribSystem>("Sampler")
        .condition<AttribThread>(tid)
        .condition<AttribExecOns>(exec_type)
        .queryInto(objects);

    if (!objects.empty())
    {
      TIME_SECTION("executeSamplers", 1, "Executing Samplers");
      FEProblemBase::objectSetupHelper<Sampler>(objects, exec_type);
      FEProblemBase::objectExecuteHelper<Sampler>(objects);
    }
  }
}

void
FEProblemBase::updateActiveObjects()
{
  TIME_SECTION("updateActiveObjects", 5, "Updating Active Objects");

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    for (auto & nl : _nl)
      nl->updateActive(tid);
    _aux->updateActive(tid);
    _indicators.updateActive(tid);
    _internal_side_indicators.updateActive(tid);
    _markers.updateActive(tid);
    _all_materials.updateActive(tid);
    _materials.updateActive(tid);
    _discrete_materials.updateActive(tid);
  }

  _control_warehouse.updateActive();
  _multi_apps.updateActive();
  _transient_multi_apps.updateActive();
  _transfers.updateActive();
  _to_multi_app_transfers.updateActive();
  _from_multi_app_transfers.updateActive();
  _between_multi_app_transfers.updateActive();
}

void
FEProblemBase::reportMooseObjectDependency(MooseObject * /*a*/, MooseObject * /*b*/)
{
  //<< "Object " << a->name() << " -> " << b->name() << std::endl;
}

void
FEProblemBase::reinitBecauseOfGhostingOrNewGeomObjects(const bool mortar_changed)
{
  TIME_SECTION("reinitBecauseOfGhostingOrNewGeomObjects",
               3,
               "Reinitializing Because of Geometric Search Objects");

  // Need to see if _any_ processor has ghosted elems or geometry objects.
  bool needs_reinit = !_ghosted_elems.empty();
  needs_reinit = needs_reinit || !_geometric_search_data._nearest_node_locators.empty() ||
                 (_mortar_data.hasObjects() && mortar_changed);
  needs_reinit =
      needs_reinit || (_displaced_problem &&
                       (!_displaced_problem->geomSearchData()._nearest_node_locators.empty() ||
                        (_mortar_data.hasDisplacedObjects() && mortar_changed)));
  _communicator.max(needs_reinit);

  if (needs_reinit)
  {
    // Call reinit to get the ghosted vectors correct now that some geometric search has been done
    es().reinit();

    if (_displaced_mesh)
      _displaced_problem->es().reinit();
  }
}

void
FEProblemBase::addDamper(const std::string & damper_name,
                         const std::string & name,
                         InputParameters & parameters)
{
  parallel_object_only();

  const auto nl_sys_num =
      parameters.isParamValid("variable")
          ? determineSolverSystem(parameters.varName("variable", name), true).second
          : (unsigned int)0;

  if (!isSolverSystemNonlinear(nl_sys_num))
    mooseError("You are trying to add a DGKernel to a linear variable/system, which is not "
               "supported at the moment!");

  parameters.set<SubProblem *>("_subproblem") = this;
  parameters.set<SystemBase *>("_sys") = _nl[nl_sys_num].get();

  _has_dampers = true;
  logAdd("Damper", name, damper_name, parameters);
  _nl[nl_sys_num]->addDamper(damper_name, name, parameters);
}

void
FEProblemBase::setupDampers()
{
  for (auto & nl : _nl)
    nl->setupDampers();
}

void
FEProblemBase::addIndicator(const std::string & indicator_name,
                            const std::string & name,
                            InputParameters & parameters)
{
  parallel_object_only();

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Indicators to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Indicator.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<Indicator> indicator =
        _factory.create<Indicator>(indicator_name, name, parameters, tid);
    logAdd("Indicator", name, indicator_name, parameters);
    std::shared_ptr<InternalSideIndicator> isi =
        std::dynamic_pointer_cast<InternalSideIndicator>(indicator);
    if (isi)
      _internal_side_indicators.addObject(isi, tid);
    else
      _indicators.addObject(indicator, tid);
  }
}

void
FEProblemBase::addMarker(const std::string & marker_name,
                         const std::string & name,
                         InputParameters & parameters)
{
  parallel_object_only();

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Markers to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Marker.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<Marker> marker = _factory.create<Marker>(marker_name, name, parameters, tid);
    logAdd("Marker", name, marker_name, parameters);
    _markers.addObject(marker, tid);
  }
}

void
FEProblemBase::addMultiApp(const std::string & multi_app_name,
                           const std::string & name,
                           InputParameters & parameters)
{
  parallel_object_only();

  parameters.set<MPI_Comm>("_mpi_comm") = _communicator.get();
  parameters.set<std::shared_ptr<CommandLine>>("_command_line") = _app.commandLine();

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow MultiApps to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this MultiApp.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
  }

  std::shared_ptr<MultiApp> multi_app = _factory.create<MultiApp>(multi_app_name, name, parameters);
  logAdd("MultiApp", name, multi_app_name, parameters);
  multi_app->setupPositions();

  _multi_apps.addObject(multi_app);

  // Store TransientMultiApp objects in another container, this is needed for calling computeDT
  std::shared_ptr<TransientMultiApp> trans_multi_app =
      std::dynamic_pointer_cast<TransientMultiApp>(multi_app);
  if (trans_multi_app)
    _transient_multi_apps.addObject(trans_multi_app);
}

bool
FEProblemBase::hasMultiApps(ExecFlagType type) const
{
  return _multi_apps[type].hasActiveObjects();
}

bool
FEProblemBase::hasMultiApp(const std::string & multi_app_name) const
{
  return _multi_apps.hasActiveObject(multi_app_name);
}

std::shared_ptr<MultiApp>
FEProblemBase::getMultiApp(const std::string & multi_app_name) const
{
  return _multi_apps.getObject(multi_app_name);
}

void
FEProblemBase::execMultiAppTransfers(ExecFlagType type, Transfer::DIRECTION direction)
{
  bool to_multiapp = direction == MultiAppTransfer::TO_MULTIAPP;
  bool from_multiapp = direction == MultiAppTransfer::FROM_MULTIAPP;
  std::string string_direction;
  if (to_multiapp)
    string_direction = " To ";
  else if (from_multiapp)
    string_direction = " From ";
  else
    string_direction = " Between ";

  const MooseObjectWarehouse<Transfer> & wh = to_multiapp     ? _to_multi_app_transfers[type]
                                              : from_multiapp ? _from_multi_app_transfers[type]
                                                              : _between_multi_app_transfers;

  if (wh.hasActiveObjects())
  {
    TIME_SECTION("execMultiAppTransfers", 1, "Executing Transfers");

    const auto & transfers = wh.getActiveObjects();

    if (_verbose_multiapps)
    {
      _console << COLOR_CYAN << "\nTransfers on " << Moose::stringify(type) << string_direction
               << "MultiApps" << COLOR_DEFAULT << ":" << std::endl;

      VariadicTable<std::string, std::string, std::string, std::string> table(
          {"Name", "Type", "From", "To"});

      // Build Table of Transfer Info
      for (const auto & transfer : transfers)
      {
        auto multiapp_transfer = dynamic_cast<MultiAppTransfer *>(transfer.get());

        table.addRow(multiapp_transfer->name(),
                     multiapp_transfer->type(),
                     multiapp_transfer->getFromName(),
                     multiapp_transfer->getToName());
      }

      // Print it
      table.print(_console);
    }

    for (const auto & transfer : transfers)
    {
      transfer->setCurrentDirection(direction);
      transfer->execute();
    }

    MooseUtils::parallelBarrierNotify(_communicator, _parallel_barrier_messaging);

    if (_verbose_multiapps)
      _console << COLOR_CYAN << "Transfers on " << Moose::stringify(type) << " Are Finished\n"
               << COLOR_DEFAULT << std::endl;
  }
  else if (_multi_apps[type].getActiveObjects().size())
  {
    if (_verbose_multiapps)
      _console << COLOR_CYAN << "\nNo Transfers on " << Moose::stringify(type) << string_direction
               << "MultiApps\n"
               << COLOR_DEFAULT << std::endl;
  }
}

std::vector<std::shared_ptr<Transfer>>
FEProblemBase::getTransfers(ExecFlagType type, Transfer::DIRECTION direction) const
{
  if (direction == MultiAppTransfer::TO_MULTIAPP)
    return _to_multi_app_transfers[type].getActiveObjects();
  else if (direction == MultiAppTransfer::FROM_MULTIAPP)
    return _from_multi_app_transfers[type].getActiveObjects();
  else
    return _between_multi_app_transfers[type].getActiveObjects();
}

std::vector<std::shared_ptr<Transfer>>
FEProblemBase::getTransfers(Transfer::DIRECTION direction) const
{
  if (direction == MultiAppTransfer::TO_MULTIAPP)
    return _to_multi_app_transfers.getActiveObjects();
  else if (direction == MultiAppTransfer::FROM_MULTIAPP)
    return _from_multi_app_transfers.getActiveObjects();
  else
    return _between_multi_app_transfers.getActiveObjects();
}

const ExecuteMooseObjectWarehouse<Transfer> &
FEProblemBase::getMultiAppTransferWarehouse(Transfer::DIRECTION direction) const
{
  if (direction == MultiAppTransfer::TO_MULTIAPP)
    return _to_multi_app_transfers;
  else if (direction == MultiAppTransfer::FROM_MULTIAPP)
    return _from_multi_app_transfers;
  else
    return _between_multi_app_transfers;
}

bool
FEProblemBase::execMultiApps(ExecFlagType type, bool auto_advance)
{
  // Active MultiApps
  const std::vector<MooseSharedPointer<MultiApp>> & multi_apps =
      _multi_apps[type].getActiveObjects();

  // Do anything that needs to be done to Apps before transfers
  for (const auto & multi_app : multi_apps)
    multi_app->preTransfer(_dt, _time);

  // Execute Transfers _to_ MultiApps
  execMultiAppTransfers(type, MultiAppTransfer::TO_MULTIAPP);

  // Execute Transfers _between_ Multiapps
  execMultiAppTransfers(type, MultiAppTransfer::BETWEEN_MULTIAPP);

  // Execute MultiApps
  if (multi_apps.size())
  {
    TIME_SECTION("execMultiApps", 1, "Executing MultiApps", false);

    if (_verbose_multiapps)
      _console << COLOR_CYAN << "\nExecuting MultiApps on " << Moose::stringify(type)
               << COLOR_DEFAULT << std::endl;

    bool success = true;

    for (const auto & multi_app : multi_apps)
    {
      success = multi_app->solveStep(_dt, _time, auto_advance);
      // no need to finish executing the subapps if one fails
      if (!success)
        break;
    }

    MooseUtils::parallelBarrierNotify(_communicator, _parallel_barrier_messaging);

    _communicator.min(success);

    if (!success)
      return false;

    if (_verbose_multiapps)
      _console << COLOR_CYAN << "Finished Executing MultiApps on " << Moose::stringify(type) << "\n"
               << COLOR_DEFAULT << std::endl;
  }

  // Execute Transfers _from_ MultiApps
  execMultiAppTransfers(type, MultiAppTransfer::FROM_MULTIAPP);

  // If we made it here then everything passed
  return true;
}

void
FEProblemBase::finalizeMultiApps()
{
  const auto & multi_apps = _multi_apps.getActiveObjects();

  for (const auto & multi_app : multi_apps)
    multi_app->finalize();
}

void
FEProblemBase::postExecute()
{
  const auto & multi_apps = _multi_apps.getActiveObjects();

  for (const auto & multi_app : multi_apps)
    multi_app->postExecute();
}

void
FEProblemBase::incrementMultiAppTStep(ExecFlagType type)
{
  const auto & multi_apps = _multi_apps[type].getActiveObjects();

  if (multi_apps.size())
    for (const auto & multi_app : multi_apps)
      multi_app->incrementTStep(_time);
}

void
FEProblemBase::finishMultiAppStep(ExecFlagType type, bool recurse_through_multiapp_levels)
{
  const auto & multi_apps = _multi_apps[type].getActiveObjects();

  if (multi_apps.size())
  {
    if (_verbose_multiapps)
      _console << COLOR_CYAN << "\nAdvancing MultiApps on " << type.name() << COLOR_DEFAULT
               << std::endl;

    for (const auto & multi_app : multi_apps)
      multi_app->finishStep(recurse_through_multiapp_levels);

    MooseUtils::parallelBarrierNotify(_communicator, _parallel_barrier_messaging);

    if (_verbose_multiapps)
      _console << COLOR_CYAN << "Finished Advancing MultiApps on " << type.name() << "\n"
               << COLOR_DEFAULT << std::endl;
  }
}

void
FEProblemBase::backupMultiApps(ExecFlagType type)
{
  const auto & multi_apps = _multi_apps[type].getActiveObjects();

  if (multi_apps.size())
  {
    TIME_SECTION("backupMultiApps", 5, "Backing Up MultiApp");

    if (_verbose_multiapps)
      _console << COLOR_CYAN << "\nBacking Up MultiApps on " << type.name() << COLOR_DEFAULT
               << std::endl;

    for (const auto & multi_app : multi_apps)
      multi_app->backup();

    MooseUtils::parallelBarrierNotify(_communicator, _parallel_barrier_messaging);

    if (_verbose_multiapps)
      _console << COLOR_CYAN << "Finished Backing Up MultiApps on " << type.name() << "\n"
               << COLOR_DEFAULT << std::endl;
  }
}

void
FEProblemBase::restoreMultiApps(ExecFlagType type, bool force)
{
  const auto & multi_apps = _multi_apps[type].getActiveObjects();

  if (multi_apps.size())
  {
    if (_verbose_multiapps)
    {
      if (force)
        _console << COLOR_CYAN << "\nRestoring Multiapps on " << type.name()
                 << " because of solve failure!" << COLOR_DEFAULT << std::endl;
      else
        _console << COLOR_CYAN << "\nRestoring MultiApps on " << type.name() << COLOR_DEFAULT
                 << std::endl;
    }

    for (const auto & multi_app : multi_apps)
      multi_app->restore(force);

    MooseUtils::parallelBarrierNotify(_communicator, _parallel_barrier_messaging);

    if (_verbose_multiapps)
      _console << COLOR_CYAN << "Finished Restoring MultiApps on " << type.name() << "\n"
               << COLOR_DEFAULT << std::endl;
  }
}

Real
FEProblemBase::computeMultiAppsDT(ExecFlagType type)
{
  const auto & multi_apps = _transient_multi_apps[type].getActiveObjects();

  Real smallest_dt = std::numeric_limits<Real>::max();

  for (const auto & multi_app : multi_apps)
    smallest_dt = std::min(smallest_dt, multi_app->computeDT());

  return smallest_dt;
}

void
FEProblemBase::execTransfers(ExecFlagType type)
{
  if (_transfers[type].hasActiveObjects())
  {
    TIME_SECTION("execTransfers", 3, "Executing Transfers");

    const auto & transfers = _transfers[type].getActiveObjects();

    for (const auto & transfer : transfers)
      transfer->execute();
  }
}

void
FEProblemBase::addTransfer(const std::string & transfer_name,
                           const std::string & name,
                           InputParameters & parameters)
{
  parallel_object_only();

  if (_displaced_problem && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == nullptr && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Transfers to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Transfer.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
  }

  // Handle the "SAME_AS_MULTIAPP" execute option. The get method is used to test for the
  // flag so the set by user flag is not reset, calling set with the true flag causes the set
  // by user status to be reset, which should only be done if the EXEC_SAME_AS_MULTIAPP is
  // being applied to the object.
  if (parameters.get<ExecFlagEnum>("execute_on").isValueSet(EXEC_SAME_AS_MULTIAPP))
  {
    ExecFlagEnum & exec_enum = parameters.set<ExecFlagEnum>("execute_on", true);
    std::shared_ptr<MultiApp> multiapp;
    if (parameters.isParamValid("multi_app"))
      multiapp = getMultiApp(parameters.get<MultiAppName>("multi_app"));
    else if (parameters.isParamValid("from_multi_app"))
      multiapp = getMultiApp(parameters.get<MultiAppName>("from_multi_app"));
    else if (parameters.isParamValid("to_multi_app"))
      multiapp = getMultiApp(parameters.get<MultiAppName>("to_multi_app"));
    // else do nothing because the user has provided invalid input. They should get a nice error
    // about this during transfer construction. This necessitates checking for null in this next
    // line, however
    if (multiapp)
      exec_enum = multiapp->getParam<ExecFlagEnum>("execute_on");
  }

  // Create the Transfer objects
  std::shared_ptr<Transfer> transfer = _factory.create<Transfer>(transfer_name, name, parameters);
  logAdd("Transfer", name, transfer_name, parameters);

  // Add MultiAppTransfer object
  std::shared_ptr<MultiAppTransfer> multi_app_transfer =
      std::dynamic_pointer_cast<MultiAppTransfer>(transfer);
  if (multi_app_transfer)
  {
    if (multi_app_transfer->directions().isValueSet(MultiAppTransfer::TO_MULTIAPP))
      _to_multi_app_transfers.addObject(multi_app_transfer);
    if (multi_app_transfer->directions().isValueSet(MultiAppTransfer::FROM_MULTIAPP))
      _from_multi_app_transfers.addObject(multi_app_transfer);
    if (multi_app_transfer->directions().isValueSet(MultiAppTransfer::BETWEEN_MULTIAPP))
      _between_multi_app_transfers.addObject(multi_app_transfer);
  }
  else
    _transfers.addObject(transfer);
}

bool
FEProblemBase::hasVariable(const std::string & var_name) const
{
  for (auto & sys : _solver_systems)
    if (sys->hasVariable(var_name))
      return true;
  if (_aux->hasVariable(var_name))
    return true;

  return false;
}

const MooseVariableFieldBase &
FEProblemBase::getVariable(const THREAD_ID tid,
                           const std::string & var_name,
                           Moose::VarKindType expected_var_type,
                           Moose::VarFieldType expected_var_field_type) const
{
  return getVariableHelper(
      tid, var_name, expected_var_type, expected_var_field_type, _solver_systems, *_aux);
}

MooseVariable &
FEProblemBase::getStandardVariable(const THREAD_ID tid, const std::string & var_name)
{
  for (auto & sys : _solver_systems)
    if (sys->hasVariable(var_name))
      return sys->getFieldVariable<Real>(tid, var_name);
  if (_aux->hasVariable(var_name))
    return _aux->getFieldVariable<Real>(tid, var_name);

  mooseError("Unknown variable " + var_name);
}

MooseVariableFieldBase &
FEProblemBase::getActualFieldVariable(const THREAD_ID tid, const std::string & var_name)
{
  for (auto & sys : _solver_systems)
    if (sys->hasVariable(var_name))
      return sys->getActualFieldVariable<Real>(tid, var_name);
  if (_aux->hasVariable(var_name))
    return _aux->getActualFieldVariable<Real>(tid, var_name);

  mooseError("Unknown variable " + var_name);
}

VectorMooseVariable &
FEProblemBase::getVectorVariable(const THREAD_ID tid, const std::string & var_name)
{
  for (auto & sys : _solver_systems)
    if (sys->hasVariable(var_name))
      return sys->getFieldVariable<RealVectorValue>(tid, var_name);
  if (_aux->hasVariable(var_name))
    return _aux->getFieldVariable<RealVectorValue>(tid, var_name);

  mooseError("Unknown variable " + var_name);
}

ArrayMooseVariable &
FEProblemBase::getArrayVariable(const THREAD_ID tid, const std::string & var_name)
{
  for (auto & sys : _solver_systems)
    if (sys->hasVariable(var_name))
      return sys->getFieldVariable<RealEigenVector>(tid, var_name);
  if (_aux->hasVariable(var_name))
    return _aux->getFieldVariable<RealEigenVector>(tid, var_name);

  mooseError("Unknown variable " + var_name);
}

bool
FEProblemBase::hasScalarVariable(const std::string & var_name) const
{
  for (auto & sys : _solver_systems)
    if (sys->hasScalarVariable(var_name))
      return true;
  if (_aux->hasScalarVariable(var_name))
    return true;

  return false;
}

MooseVariableScalar &
FEProblemBase::getScalarVariable(const THREAD_ID tid, const std::string & var_name)
{
  for (auto & sys : _solver_systems)
    if (sys->hasScalarVariable(var_name))
      return sys->getScalarVariable(tid, var_name);
  if (_aux->hasScalarVariable(var_name))
    return _aux->getScalarVariable(tid, var_name);

  mooseError("Unknown variable " + var_name);
}

System &
FEProblemBase::getSystem(const std::string & var_name)
{
  const auto [var_in_sys, sys_num] = determineSolverSystem(var_name);
  if (var_in_sys)
    return _solver_systems[sys_num]->system();
  else if (_aux->hasVariable(var_name))
    return _aux->system();
  else
    mooseError("Unable to find a system containing the variable " + var_name);
}

void
FEProblemBase::setActiveFEVariableCoupleableMatrixTags(std::set<TagID> & mtags, const THREAD_ID tid)
{
  SubProblem::setActiveFEVariableCoupleableMatrixTags(mtags, tid);

  if (_displaced_problem)
    _displaced_problem->setActiveFEVariableCoupleableMatrixTags(mtags, tid);
}

void
FEProblemBase::setActiveFEVariableCoupleableVectorTags(std::set<TagID> & vtags, const THREAD_ID tid)
{
  SubProblem::setActiveFEVariableCoupleableVectorTags(vtags, tid);

  if (_displaced_problem)
    _displaced_problem->setActiveFEVariableCoupleableVectorTags(vtags, tid);
}

void
FEProblemBase::setActiveScalarVariableCoupleableMatrixTags(std::set<TagID> & mtags,
                                                           const THREAD_ID tid)
{
  SubProblem::setActiveScalarVariableCoupleableMatrixTags(mtags, tid);

  if (_displaced_problem)
    _displaced_problem->setActiveScalarVariableCoupleableMatrixTags(mtags, tid);
}

void
FEProblemBase::setActiveScalarVariableCoupleableVectorTags(std::set<TagID> & vtags,
                                                           const THREAD_ID tid)
{
  SubProblem::setActiveScalarVariableCoupleableVectorTags(vtags, tid);

  if (_displaced_problem)
    _displaced_problem->setActiveScalarVariableCoupleableVectorTags(vtags, tid);
}

void
FEProblemBase::setActiveElementalMooseVariables(const std::set<MooseVariableFEBase *> & moose_vars,
                                                const THREAD_ID tid)
{
  SubProblem::setActiveElementalMooseVariables(moose_vars, tid);

  if (_displaced_problem)
    _displaced_problem->setActiveElementalMooseVariables(moose_vars, tid);
}

void
FEProblemBase::clearActiveElementalMooseVariables(const THREAD_ID tid)
{
  SubProblem::clearActiveElementalMooseVariables(tid);

  if (_displaced_problem)
    _displaced_problem->clearActiveElementalMooseVariables(tid);
}

void
FEProblemBase::clearActiveFEVariableCoupleableMatrixTags(const THREAD_ID tid)
{
  SubProblem::clearActiveFEVariableCoupleableMatrixTags(tid);

  if (_displaced_problem)
    _displaced_problem->clearActiveFEVariableCoupleableMatrixTags(tid);
}

void
FEProblemBase::clearActiveFEVariableCoupleableVectorTags(const THREAD_ID tid)
{
  SubProblem::clearActiveFEVariableCoupleableVectorTags(tid);

  if (_displaced_problem)
    _displaced_problem->clearActiveFEVariableCoupleableVectorTags(tid);
}

void
FEProblemBase::clearActiveScalarVariableCoupleableMatrixTags(const THREAD_ID tid)
{
  SubProblem::clearActiveScalarVariableCoupleableMatrixTags(tid);

  if (_displaced_problem)
    _displaced_problem->clearActiveScalarVariableCoupleableMatrixTags(tid);
}

void
FEProblemBase::clearActiveScalarVariableCoupleableVectorTags(const THREAD_ID tid)
{
  SubProblem::clearActiveScalarVariableCoupleableVectorTags(tid);

  if (_displaced_problem)
    _displaced_problem->clearActiveScalarVariableCoupleableVectorTags(tid);
}

void
FEProblemBase::setActiveMaterialProperties(const std::unordered_set<unsigned int> & mat_prop_ids,
                                           const THREAD_ID tid)
{
  // mark active properties in every material
  for (auto & mat : _all_materials.getObjects(tid))
    mat->setActiveProperties(mat_prop_ids);
  for (auto & mat : _all_materials[Moose::FACE_MATERIAL_DATA].getObjects(tid))
    mat->setActiveProperties(mat_prop_ids);
  for (auto & mat : _all_materials[Moose::NEIGHBOR_MATERIAL_DATA].getObjects(tid))
    mat->setActiveProperties(mat_prop_ids);

  _has_active_material_properties[tid] = !mat_prop_ids.empty();
}

bool
FEProblemBase::hasActiveMaterialProperties(const THREAD_ID tid) const
{
  return _has_active_material_properties[tid];
}

void
FEProblemBase::clearActiveMaterialProperties(const THREAD_ID tid)
{
  _has_active_material_properties[tid] = 0;
}

void
FEProblemBase::addAnyRedistributers()
{
#ifdef LIBMESH_ENABLE_AMR
  if ((_adaptivity.isOn() || _num_grid_steps) &&
      (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties() ||
       _neighbor_material_props.hasStatefulProperties()))
  {
    // Even on a serialized Mesh, we don't keep our material
    // properties serialized, so we'll rely on the callback to
    // redistribute() to redistribute properties at the same time
    // libMesh is redistributing elements.
    auto add_redistributer = [this](MooseMesh & mesh,
                                    const std::string & redistributer_name,
                                    const bool use_displaced_mesh)
    {
      InputParameters redistribute_params = RedistributeProperties::validParams();
      redistribute_params.set<MooseApp *>("_moose_app") = &_app;
      redistribute_params.set<std::string>("for_whom") = this->name();
      redistribute_params.set<MooseMesh *>("mesh") = &mesh;
      redistribute_params.set<Moose::RelationshipManagerType>("rm_type") =
          Moose::RelationshipManagerType::GEOMETRIC;
      redistribute_params.set<bool>("use_displaced_mesh") = use_displaced_mesh;
      redistribute_params.setHitNode(*parameters().getHitNode(), {});

      std::shared_ptr<RedistributeProperties> redistributer =
          _factory.create<RedistributeProperties>(
              "RedistributeProperties", redistributer_name, redistribute_params);

      if (_material_props.hasStatefulProperties())
        redistributer->addMaterialPropertyStorage(_material_props);

      if (_bnd_material_props.hasStatefulProperties())
        redistributer->addMaterialPropertyStorage(_bnd_material_props);

      if (_neighbor_material_props.hasStatefulProperties())
        redistributer->addMaterialPropertyStorage(_neighbor_material_props);

      mesh.getMesh().add_ghosting_functor(redistributer);
    };

    add_redistributer(_mesh, "mesh_property_redistributer", false);
    if (_displaced_problem)
      add_redistributer(_displaced_problem->mesh(), "displaced_mesh_property_redistributer", true);
  }
#endif // LIBMESH_ENABLE_AMR
}

void
FEProblemBase::updateMaxQps()
{
  // Find the maximum number of quadrature points
  {
    MaxQpsThread mqt(*this);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), mqt);
    _max_qps = mqt.max();

    // If we have more shape functions or more quadrature points on
    // another processor, then we may need to handle those elements
    // ourselves later after repartitioning.
    _communicator.max(_max_qps);
  }

  unsigned int max_qpts = getMaxQps();
  if (max_qpts > Moose::constMaxQpsPerElem)
    mooseError("Max quadrature points per element assumptions made in some code (e.g.  Coupleable ",
               "and MaterialPropertyInterface classes) have been violated.\n",
               "Complain to Moose developers to have constMaxQpsPerElem increased from ",
               Moose::constMaxQpsPerElem,
               " to ",
               max_qpts);
  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    // the highest available order in libMesh is 43
    _scalar_zero[tid].resize(FORTYTHIRD, 0);
    _zero[tid].resize(max_qpts, 0);
    _ad_zero[tid].resize(max_qpts, 0);
    _grad_zero[tid].resize(max_qpts, RealGradient(0.));
    _ad_grad_zero[tid].resize(max_qpts, ADRealGradient(0));
    _second_zero[tid].resize(max_qpts, RealTensor(0.));
    _ad_second_zero[tid].resize(max_qpts, ADRealTensorValue(0));
    _vector_zero[tid].resize(max_qpts, RealGradient(0.));
    _vector_curl_zero[tid].resize(max_qpts, RealGradient(0.));
  }
}

void
FEProblemBase::bumpVolumeQRuleOrder(Order order, SubdomainID block)
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    for (const auto i : index_range(_nl))
      _assembly[tid][i]->bumpVolumeQRuleOrder(order, block);

  if (_displaced_problem)
    _displaced_problem->bumpVolumeQRuleOrder(order, block);

  updateMaxQps();
}

void
FEProblemBase::bumpAllQRuleOrder(Order order, SubdomainID block)
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    for (const auto i : index_range(_nl))
      _assembly[tid][i]->bumpAllQRuleOrder(order, block);

  if (_displaced_problem)
    _displaced_problem->bumpAllQRuleOrder(order, block);

  updateMaxQps();
}

void
FEProblemBase::createQRules(QuadratureType type,
                            Order order,
                            Order volume_order,
                            Order face_order,
                            SubdomainID block,
                            const bool allow_negative_qweights)
{
  if (order == INVALID_ORDER)
  {
    // automatically determine the integration order
    order = _solver_systems[0]->getMinQuadratureOrder();
    for (const auto i : make_range(std::size_t(1), _solver_systems.size()))
      if (order < _solver_systems[i]->getMinQuadratureOrder())
        order = _solver_systems[i]->getMinQuadratureOrder();
    if (order < _aux->getMinQuadratureOrder())
      order = _aux->getMinQuadratureOrder();
  }

  if (volume_order == INVALID_ORDER)
    volume_order = order;

  if (face_order == INVALID_ORDER)
    face_order = order;

  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    for (const auto i : index_range(_solver_systems))
      _assembly[tid][i]->createQRules(
          type, order, volume_order, face_order, block, allow_negative_qweights);

  if (_displaced_problem)
    _displaced_problem->createQRules(
        type, order, volume_order, face_order, block, allow_negative_qweights);

  updateMaxQps();
}

void
FEProblemBase::setCoupling(Moose::CouplingType type)
{
  if (_trust_user_coupling_matrix)
  {
    if (_coupling != Moose::COUPLING_CUSTOM)
      mooseError("Someone told us (the FEProblemBase) to trust the user coupling matrix, but we "
                 "haven't been provided a coupling matrix!");

    // We've been told to trust the user coupling matrix, so we're going to leave things alone
    return;
  }

  _coupling = type;
}

void
FEProblemBase::setCouplingMatrix(CouplingMatrix * cm, const unsigned int i)
{
  // TODO: Deprecate method
  setCoupling(Moose::COUPLING_CUSTOM);
  _cm[i].reset(cm);
}

void
FEProblemBase::setCouplingMatrix(std::unique_ptr<CouplingMatrix> cm, const unsigned int i)
{
  setCoupling(Moose::COUPLING_CUSTOM);
  _cm[i] = std::move(cm);
}

void
FEProblemBase::trustUserCouplingMatrix()
{
  if (_coupling != Moose::COUPLING_CUSTOM)
    mooseError("Someone told us (the FEProblemBase) to trust the user coupling matrix, but we "
               "haven't been provided a coupling matrix!");

  _trust_user_coupling_matrix = true;
}

void
FEProblemBase::setNonlocalCouplingMatrix()
{
  TIME_SECTION("setNonlocalCouplingMatrix", 5, "Setting Nonlocal Coupling Matrix");

  if (_nl.size() > 1)
    mooseError("Nonlocal kernels are weirdly stored on the FEProblem so we don't currently support "
               "multiple nonlinear systems with nonlocal kernels.");

  for (const auto nl_sys_num : index_range(_nl))
  {
    auto & nl = _nl[nl_sys_num];
    auto & nonlocal_cm = _nonlocal_cm[nl_sys_num];
    unsigned int n_vars = nl->nVariables();
    nonlocal_cm.resize(n_vars);
    const auto & vars = nl->getVariables(0);
    const auto & nonlocal_kernel = _nonlocal_kernels.getObjects();
    const auto & nonlocal_integrated_bc = _nonlocal_integrated_bcs.getObjects();
    for (const auto & ivar : vars)
    {
      for (const auto & kernel : nonlocal_kernel)
      {
        for (unsigned int i = ivar->number(); i < ivar->number() + ivar->count(); ++i)
          if (i == kernel->variable().number())
            for (const auto & jvar : vars)
            {
              const auto it = _var_dof_map.find(jvar->name());
              if (it != _var_dof_map.end())
              {
                unsigned int j = jvar->number();
                nonlocal_cm(i, j) = 1;
              }
            }
      }
      for (const auto & integrated_bc : nonlocal_integrated_bc)
      {
        for (unsigned int i = ivar->number(); i < ivar->number() + ivar->count(); ++i)
          if (i == integrated_bc->variable().number())
            for (const auto & jvar : vars)
            {
              const auto it = _var_dof_map.find(jvar->name());
              if (it != _var_dof_map.end())
              {
                unsigned int j = jvar->number();
                nonlocal_cm(i, j) = 1;
              }
            }
      }
    }
  }
}

bool
FEProblemBase::areCoupled(const unsigned int ivar,
                          const unsigned int jvar,
                          const unsigned int nl_sys) const
{
  return (*_cm[nl_sys])(ivar, jvar);
}

std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> &
FEProblemBase::couplingEntries(const THREAD_ID tid, const unsigned int nl_sys)
{
  return _assembly[tid][nl_sys]->couplingEntries();
}

std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> &
FEProblemBase::nonlocalCouplingEntries(const THREAD_ID tid, const unsigned int nl_sys)
{
  return _assembly[tid][nl_sys]->nonlocalCouplingEntries();
}

void
FEProblemBase::init()
{
  if (_initialized)
    return;

  TIME_SECTION("init", 2, "Initializing");

  // call executioner's preProblemInit so that it can do some setups before problem init
  _app.getExecutioner()->preProblemInit();

  // If we have AD and we are doing global AD indexing, then we should by default set the matrix
  // coupling to full. If the user has told us to trust their coupling matrix, then this call will
  // not do anything
  if (haveADObjects() && Moose::globalADIndexing())
    setCoupling(Moose::COUPLING_FULL);

  for (const auto i : index_range(_nl))
  {
    auto & nl = _nl[i];
    auto & cm = _cm[i];

    unsigned int n_vars = nl->nVariables();
    {
      TIME_SECTION("fillCouplingMatrix", 3, "Filling Coupling Matrix");

      switch (_coupling)
      {
        case Moose::COUPLING_DIAG:
          cm = std::make_unique<CouplingMatrix>(n_vars);
          for (unsigned int i = 0; i < n_vars; i++)
            (*cm)(i, i) = 1;
          break;

          // for full jacobian
        case Moose::COUPLING_FULL:
          cm = std::make_unique<CouplingMatrix>(n_vars);
          for (unsigned int i = 0; i < n_vars; i++)
            for (unsigned int j = 0; j < n_vars; j++)
              (*cm)(i, j) = 1;
          break;

        case Moose::COUPLING_CUSTOM:
          // do nothing, _cm was already set through couplingMatrix() call
          break;
      }
    }

    nl->dofMap()._dof_coupling = cm.get();

    // If there are no variables, make sure to pass a nullptr coupling
    // matrix, to avoid warnings about non-nullptr yet empty
    // CouplingMatrices.
    if (n_vars == 0)
      nl->dofMap()._dof_coupling = nullptr;

    nl->dofMap().attach_extra_sparsity_function(&extraSparsity, nl.get());
    nl->dofMap().attach_extra_send_list_function(&extraSendList, nl.get());
    _aux->dofMap().attach_extra_send_list_function(&extraSendList, _aux.get());

    if (!_skip_nl_system_check && _solve && n_vars == 0)
      mooseError("No variables specified in nonlinear system '", nl->name(), "'.");
  }

  ghostGhostedBoundaries(); // We do this again right here in case new boundaries have been added

  // We may have added element/nodes to the mesh in ghostGhostedBoundaries so we need to update
  // all of our mesh information. We need to make sure that mesh information is up-to-date before
  // EquationSystems::init because that will call through to updateGeomSearch (for sparsity
  // augmentation) and if we haven't added back boundary node information before that latter call,
  // then we're screwed. We'll get things like "Unable to find closest node!"
  _mesh.meshChanged();
  if (_displaced_problem)
    _displaced_mesh->meshChanged();

  // do not assemble system matrix for JFNK solve
  for (auto & nl : _nl)
    if (solverParams()._type == Moose::ST_JFNK)
      nl->turnOffJacobian();

  for (auto & sys : _solver_systems)
    sys->preInit();
  _aux->preInit();

  // Build the mortar segment meshes, if they haven't been already, for a couple reasons:
  // 1) Get the ghosting correct for both static and dynamic meshes
  // 2) Make sure the mortar mesh is built for mortar constraints that live on the static mesh
  //
  // It is worth-while to note that mortar meshes that live on a dynamic mesh will be built
  // during residual and Jacobian evaluation because when displacements are solution variables
  // the mortar mesh will move and change during the course of a non-linear solve. We DO NOT
  // redo ghosting during non-linear solve, so for purpose 1) the below call has to be made
  if (!_mortar_data.initialized())
    updateMortarMesh();

  {
    TIME_SECTION("EquationSystems::Init", 2, "Initializing Equation Systems");
    es().init();
  }

  for (auto & sys : _solver_systems)
    sys->postInit();
  _aux->postInit();

  // Now that the equation system and the dof distribution is done, we can generate the
  // finite volume-related parts if needed.
  if (haveFV())
    _mesh.setupFiniteVolumeMeshData();

  for (auto & sys : _solver_systems)
    sys->update();
  _aux->update();

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    for (const auto i : index_range(_nl))
    {
      mooseAssert(
          _cm[i],
          "Coupling matrix not set for system "
              << i
              << ". This should only happen if a preconditioner was not setup for this system");
      _assembly[tid][i]->init(_cm[i].get());
    }

  if (_displaced_problem)
    _displaced_problem->init();

  _initialized = true;
}

unsigned int
FEProblemBase::nlSysNum(const NonlinearSystemName & nl_sys_name) const
{
  std::istringstream ss(nl_sys_name);
  unsigned int nl_sys_num;
  if (!(ss >> nl_sys_num) || !ss.eof())
    nl_sys_num = libmesh_map_find(_nl_sys_name_to_num, nl_sys_name);

  return nl_sys_num;
}

unsigned int
FEProblemBase::linearSysNum(const LinearSystemName & linear_sys_name) const
{
  std::istringstream ss(linear_sys_name);
  unsigned int linear_sys_num;
  if (!(ss >> linear_sys_num) || !ss.eof())
    linear_sys_num = libmesh_map_find(_linear_sys_name_to_num, linear_sys_name);

  return linear_sys_num;
}

unsigned int
FEProblemBase::solverSysNum(const SolverSystemName & solver_sys_name) const
{
  std::istringstream ss(solver_sys_name);
  unsigned int solver_sys_num;
  if (!(ss >> solver_sys_num) || !ss.eof())
  {
    const auto & search = _solver_sys_name_to_num.find(solver_sys_name);
    if (search == _solver_sys_name_to_num.end())
      mooseError("The solver system number was requested for system '" + solver_sys_name,
                 "' but this system does not exist in the Problem. Systems can be added to the "
                 "problem using the 'nl_sys_names' parameter.\nSystems in the Problem: " +
                     Moose::stringify(_solver_sys_names));
    solver_sys_num = search->second;
  }

  return solver_sys_num;
}

unsigned int
FEProblemBase::systemNumForVariable(const VariableName & variable_name) const
{
  for (const auto & solver_sys : _solver_systems)
    if (solver_sys->hasVariable(variable_name))
      return solver_sys->number();
  mooseAssert(_aux, "Should have an auxiliary system");
  if (_aux->hasVariable(variable_name))
    return _aux->number();

  mooseError("Variable '",
             variable_name,
             "' was not found in any solver (nonlinear/linear) or auxiliary system");
}

void
FEProblemBase::solve(const unsigned int nl_sys_num)
{
  TIME_SECTION("solve", 1, "Solving", false);

  setCurrentNonlinearSystem(nl_sys_num);

  // This prevents stale dof indices from lingering around and possibly leading to invalid reads
  // and writes. Dof indices may be made stale through operations like mesh adaptivity
  clearAllDofIndices();
  if (_displaced_problem)
    _displaced_problem->clearAllDofIndices();

#if PETSC_RELEASE_LESS_THAN(3, 12, 0)
  Moose::PetscSupport::petscSetOptions(
      _petsc_options, _solver_params); // Make sure the PETSc options are setup for this app
#else
  // Now this database will be the default
  // Each app should have only one database
  if (!_app.isUltimateMaster())
    LibmeshPetscCall(PetscOptionsPush(_petsc_option_data_base));

  // We did not add PETSc options to database yet
  if (!_is_petsc_options_inserted)
  {
    Moose::PetscSupport::petscSetOptions(_petsc_options, _solver_params, this);
    _is_petsc_options_inserted = true;
  }
#endif

  // set up DM which is required if use a field split preconditioner
  // We need to setup DM every "solve()" because libMesh destroy SNES after solve()
  // Do not worry, DM setup is very cheap
  _current_nl_sys->setupDM();

  // Setup the output system for printing linear/nonlinear iteration information and some solver
  // settings
  initPetscOutputAndSomeSolverSettings();

  possiblyRebuildGeomSearchPatches();

  // reset flag so that residual evaluation does not get skipped
  // and the next non-linear iteration does not automatically fail with
  // "DIVERGED_NANORINF", when we throw  an exception and stop solve
  _fail_next_nonlinear_convergence_check = false;

  if (_solve)
  {
    _current_nl_sys->solve();
    _current_nl_sys->update();
  }

  // sync solutions in displaced problem
  if (_displaced_problem)
    _displaced_problem->syncSolutions();

#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  if (!_app.isUltimateMaster())
    LibmeshPetscCall(PetscOptionsPop());
#endif
}

void
FEProblemBase::setException(const std::string & message)
{
  _has_exception = true;
  _exception_message = message;
}

void
FEProblemBase::checkExceptionAndStopSolve(bool print_message)
{
  if (_skip_exception_check)
    return;

  TIME_SECTION("checkExceptionAndStopSolve", 5);

  // See if any processor had an exception.  If it did, get back the
  // processor that the exception occurred on.
  unsigned int processor_id;

  _communicator.maxloc(_has_exception, processor_id);

  if (_has_exception)
  {
    _communicator.broadcast(_exception_message, processor_id);

    if (_current_execute_on_flag == EXEC_LINEAR || _current_execute_on_flag == EXEC_NONLINEAR ||
        _current_execute_on_flag == EXEC_POSTCHECK)
    {
      // Print the message
      if (_communicator.rank() == 0 && print_message)
      {
        _console << "\n" << _exception_message << "\n";
        if (isTransient())
          _console
              << "To recover, the solution will fail and then be re-attempted with a reduced time "
                 "step.\n"
              << std::endl;
      }

      // Stop the solve -- this entails setting
      // SNESSetFunctionDomainError() or directly inserting NaNs in the
      // residual vector to let PETSc >= 3.6 return DIVERGED_NANORINF.
      if (_current_nl_sys)
        _current_nl_sys->stopSolve(_current_execute_on_flag, _fe_vector_tags);

      if (_current_linear_sys)
        _current_linear_sys->stopSolve(_current_execute_on_flag, _fe_vector_tags);

      // and close Aux system (we MUST do this here; see #11525)
      _aux->solution().close();

      // We've handled this exception, so we no longer have one.
      _has_exception = false;

      // Force the next non-linear convergence check to fail (and all further residual evaluation
      // to be skipped).
      _fail_next_nonlinear_convergence_check = true;

      // Repropagate the exception, so it can be caught at a higher level, typically
      // this is NonlinearSystem::computeResidual().
      throw MooseException(_exception_message);
    }
    else
      mooseError("The following parallel-communicated exception was detected during " +
                 Moose::stringify(_current_execute_on_flag) + " evaluation:\n" +
                 _exception_message +
                 "\nBecause this did not occur during residual evaluation, there"
                 " is no way to handle this, so the solution is aborting.\n");
  }
}

void
FEProblemBase::resetState()
{
  // Our default state is to allow computing derivatives
  ADReal::do_derivatives = true;
  _current_execute_on_flag = EXEC_NONE;

  // Clear the VectorTags and MatrixTags
  clearCurrentResidualVectorTags();
  clearCurrentJacobianMatrixTags();

  _safe_access_tagged_vectors = true;
  _safe_access_tagged_matrices = true;

  setCurrentlyComputingResidual(false);
  setCurrentlyComputingJacobian(false);
  setCurrentlyComputingResidualAndJacobian(false);
  if (_displaced_problem)
  {
    _displaced_problem->setCurrentlyComputingResidual(false);
    _displaced_problem->setCurrentlyComputingJacobian(false);
    _displaced_problem->setCurrentlyComputingResidualAndJacobian(false);
  }
}

void
FEProblemBase::solveLinearSystem(const unsigned int linear_sys_num,
                                 const Moose::PetscSupport::PetscOptions * po)
{
  TIME_SECTION("solve", 1, "Solving", false);

  setCurrentLinearSystem(linear_sys_num);

  const Moose::PetscSupport::PetscOptions & options = po ? *po : _petsc_options;
  SolverParams solver_params;
  solver_params._type = Moose::SolveType::ST_LINEAR;
  solver_params._line_search = Moose::LineSearchType::LS_NONE;

#if PETSC_RELEASE_LESS_THAN(3, 12, 0)
  LibmeshPetscCall(Moose::PetscSupport::petscSetOptions(
      options, solver_params)); // Make sure the PETSc options are setup for this app
#else
  // Now this database will be the default
  // Each app should have only one database
  if (!_app.isUltimateMaster())
    LibmeshPetscCall(PetscOptionsPush(_petsc_option_data_base));

  // We did not add PETSc options to database yet
  if (!_is_petsc_options_inserted)
  {
    Moose::PetscSupport::petscSetOptions(options, solver_params, this);
    _is_petsc_options_inserted = true;
  }
#endif

  if (_solve)
    _current_linear_sys->solve();

#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  if (!_app.isUltimateMaster())
    LibmeshPetscCall(PetscOptionsPop());
#endif
}

bool
FEProblemBase::solverSystemConverged(const unsigned int sys_num)
{
  if (_solve)
    return _solver_systems[sys_num]->converged();
  else
    return true;
}

unsigned int
FEProblemBase::nNonlinearIterations(const unsigned int nl_sys_num) const
{
  return _nl[nl_sys_num]->nNonlinearIterations();
}

unsigned int
FEProblemBase::nLinearIterations(const unsigned int nl_sys_num) const
{
  return _nl[nl_sys_num]->nLinearIterations();
}

Real
FEProblemBase::finalNonlinearResidual(const unsigned int nl_sys_num) const
{
  return _nl[nl_sys_num]->finalNonlinearResidual();
}

bool
FEProblemBase::computingPreSMOResidual(const unsigned int nl_sys_num) const
{
  return _nl[nl_sys_num]->computingPreSMOResidual();
}

void
FEProblemBase::copySolutionsBackwards()
{
  TIME_SECTION("copySolutionsBackwards", 3, "Copying Solutions Backward");

  for (auto & sys : _solver_systems)
    sys->copySolutionsBackwards();
  _aux->copySolutionsBackwards();
}

void
FEProblemBase::advanceState()
{
  TIME_SECTION("advanceState", 5, "Advancing State");

  for (auto & sys : _solver_systems)
    sys->copyOldSolutions();
  _aux->copyOldSolutions();

  if (_displaced_problem)
  {
    for (const auto i : index_range(_solver_systems))
      _displaced_problem->solverSys(i).copyOldSolutions();
    _displaced_problem->auxSys().copyOldSolutions();
  }

  _reporter_data.copyValuesBack();

  getMooseApp().getChainControlDataSystem().copyValuesBack();

  if (_material_props.hasStatefulProperties())
    _material_props.shift();

  if (_bnd_material_props.hasStatefulProperties())
    _bnd_material_props.shift();

  if (_neighbor_material_props.hasStatefulProperties())
    _neighbor_material_props.shift();
}

void
FEProblemBase::restoreSolutions()
{
  TIME_SECTION("restoreSolutions", 5, "Restoring Solutions");

  for (auto & sys : _solver_systems)
    sys->restoreSolutions();
  _aux->restoreSolutions();

  if (_displaced_problem)
    _displaced_problem->updateMesh();
}

void
FEProblemBase::saveOldSolutions()
{
  TIME_SECTION("saveOldSolutions", 5, "Saving Old Solutions");

  for (auto & sys : _solver_systems)
    sys->saveOldSolutions();
  _aux->saveOldSolutions();
}

void
FEProblemBase::restoreOldSolutions()
{
  TIME_SECTION("restoreOldSolutions", 5, "Restoring Old Solutions");

  for (auto & sys : _solver_systems)
    sys->restoreOldSolutions();
  _aux->restoreOldSolutions();
}

void
FEProblemBase::outputStep(ExecFlagType type)
{
  TIME_SECTION("outputStep", 1, "Outputting");

  setCurrentExecuteOnFlag(type);

  for (auto & sys : _solver_systems)
    sys->update();
  _aux->update();
  if (_displaced_problem)
    _displaced_problem->syncSolutions();
  _app.getOutputWarehouse().outputStep(type);

  setCurrentExecuteOnFlag(EXEC_NONE);
}

void
FEProblemBase::allowOutput(bool state)
{
  _app.getOutputWarehouse().allowOutput(state);
}

void
FEProblemBase::forceOutput()
{
  _app.getOutputWarehouse().forceOutput();
}

void
FEProblemBase::initPetscOutputAndSomeSolverSettings()
{
  _app.getOutputWarehouse().solveSetup();
  Moose::PetscSupport::petscSetDefaults(*this);
}

void
FEProblemBase::onTimestepBegin()
{
  TIME_SECTION("onTimestepBegin", 2);

  for (auto & nl : _nl)
    nl->onTimestepBegin();
}

void
FEProblemBase::onTimestepEnd()
{
}

Real
FEProblemBase::getTimeFromStateArg(const Moose::StateArg & state) const
{
  if (state.iteration_type != Moose::SolutionIterationType::Time)
    // If we are any iteration type other than time (e.g. nonlinear), then temporally we are still
    // in the present time
    return time();

  switch (state.state)
  {
    case 0:
      return time();

    case 1:
      return timeOld();

    default:
      mooseError("Unhandled state ", state.state, " in FEProblemBase::getTimeFromStateArg");
  }
}

void
FEProblemBase::addTimeIntegrator(const std::string & type,
                                 const std::string & name,
                                 InputParameters & parameters)
{
  parallel_object_only();

  parameters.set<SubProblem *>("_subproblem") = this;
  logAdd("TimeIntegrator", name, type, parameters);
  _aux->addTimeIntegrator(type, name + ":aux", parameters);
  for (auto & sys : _solver_systems)
    sys->addTimeIntegrator(type, name + ":" + sys->name(), parameters);
  _has_time_integrator = true;

  // add vectors to store u_dot, u_dotdot, udot_old, u_dotdot_old and
  // solution vectors older than 2 time steps, if requested by the time
  // integrator
  _aux->addDotVectors();
  for (auto & nl : _nl)
  {
    nl->addDotVectors();

    auto tag_udot = nl->getTimeIntegrators()[0]->uDotFactorTag();
    if (!nl->hasVector(tag_udot))
      nl->associateVectorToTag(*nl->solutionUDot(), tag_udot);
    auto tag_udotdot = nl->getTimeIntegrators()[0]->uDotDotFactorTag();
    if (!nl->hasVector(tag_udotdot) && uDotDotRequested())
      nl->associateVectorToTag(*nl->solutionUDotDot(), tag_udotdot);
  }

  if (_displaced_problem)
    // Time integrator does not exist when displaced problem is created.
    _displaced_problem->addTimeIntegrator();
}

void
FEProblemBase::addPredictor(const std::string & type,
                            const std::string & name,
                            InputParameters & parameters)
{
  parallel_object_only();

  if (!numNonlinearSystems() && numLinearSystems())
    mooseError("Vector bounds cannot be used with LinearSystems!");

  parameters.set<SubProblem *>("_subproblem") = this;
  std::shared_ptr<Predictor> predictor = _factory.create<Predictor>(type, name, parameters);
  logAdd("Predictor", name, type, parameters);

  for (auto & nl : _nl)
    nl->setPredictor(predictor);
}

Real
FEProblemBase::computeResidualL2Norm(NonlinearSystemBase & sys)
{
  _current_nl_sys = &sys;
  computeResidual(*sys.currentSolution(), sys.RHS(), sys.number());
  return sys.RHS().l2_norm();
}

Real
FEProblemBase::computeResidualL2Norm(LinearSystem & sys)
{
  _current_linear_sys = &sys;

  // We assemble the current system to check the current residual
  computeLinearSystemSys(sys.linearImplicitSystem(),
                         *sys.linearImplicitSystem().matrix,
                         *sys.linearImplicitSystem().rhs,
                         /*compute fresh gradients*/ true);

  // Unfortunate, but we have to allocate a new vector for the residual
  auto residual = sys.linearImplicitSystem().rhs->clone();
  residual->scale(-1.0);
  residual->add_vector(*sys.currentSolution(), *sys.linearImplicitSystem().matrix);
  return residual->l2_norm();
}

Real
FEProblemBase::computeResidualL2Norm()
{
  TIME_SECTION("computeResidualL2Norm", 2, "Computing L2 Norm of Residual");

  // We use sum the squared norms of the individual systems and then take the square root of it
  Real l2_norm = 0.0;
  for (auto sys : _nl)
  {
    const auto norm = computeResidualL2Norm(*sys);
    l2_norm += norm * norm;
  }

  for (auto sys : _linear_systems)
  {
    const auto norm = computeResidualL2Norm(*sys);
    l2_norm += norm * norm;
  }

  return std::sqrt(l2_norm);
}

void
FEProblemBase::computeResidualSys(NonlinearImplicitSystem & sys,
                                  const NumericVector<Number> & soln,
                                  NumericVector<Number> & residual)
{
  parallel_object_only();

  TIME_SECTION("computeResidualSys", 5);

  computeResidual(soln, residual, sys.number());
}

void
FEProblemBase::computeResidual(NonlinearImplicitSystem & sys,
                               const NumericVector<Number> & soln,
                               NumericVector<Number> & residual)
{
  mooseDeprecated("Please use computeResidualSys");

  computeResidualSys(sys, soln, residual);
}

void
FEProblemBase::computeResidual(const NumericVector<Number> & soln,
                               NumericVector<Number> & residual,
                               const unsigned int nl_sys_num)
{
  setCurrentNonlinearSystem(nl_sys_num);

  // We associate the residual tag with the given residual vector to make sure we
  // don't filter it out below
  _current_nl_sys->associateVectorToTag(residual, _current_nl_sys->residualVectorTag());
  const auto & residual_vector_tags = getVectorTags(Moose::VECTOR_TAG_RESIDUAL);

  mooseAssert(_fe_vector_tags.empty(), "This should be empty indicating a clean starting state");
  // We filter out tags which do not have associated vectors in the current nonlinear
  // system. This is essential to be able to use system-dependent residual tags.
  selectVectorTagsFromSystem(*_current_nl_sys, residual_vector_tags, _fe_vector_tags);

  computeResidualInternal(soln, residual, _fe_vector_tags);
  _fe_vector_tags.clear();
}

void
FEProblemBase::computeResidualAndJacobian(const NumericVector<Number> & soln,
                                          NumericVector<Number> & residual,
                                          SparseMatrix<Number> & jacobian)
{
  try
  {
    try
    {
      // vector tags
      _current_nl_sys->associateVectorToTag(residual, _current_nl_sys->residualVectorTag());
      const auto & residual_vector_tags = getVectorTags(Moose::VECTOR_TAG_RESIDUAL);

      mooseAssert(_fe_vector_tags.empty(),
                  "This should be empty indicating a clean starting state");
      // We filter out tags which do not have associated vectors in the current nonlinear
      // system. This is essential to be able to use system-dependent residual tags.
      selectVectorTagsFromSystem(*_current_nl_sys, residual_vector_tags, _fe_vector_tags);

      setCurrentResidualVectorTags(_fe_vector_tags);

      // matrix tags
      {
        _fe_matrix_tags.clear();

        auto & tags = getMatrixTags();
        for (auto & tag : tags)
          _fe_matrix_tags.insert(tag.second);
      }

      _current_nl_sys->setSolution(soln);

      _current_nl_sys->associateVectorToTag(residual, _current_nl_sys->residualVectorTag());
      _current_nl_sys->associateMatrixToTag(jacobian, _current_nl_sys->systemMatrixTag());

      for (const auto tag : _fe_matrix_tags)
        if (_current_nl_sys->hasMatrix(tag))
        {
          auto & matrix = _current_nl_sys->getMatrix(tag);
          matrix.zero();
          if (haveADObjects())
            // PETSc algorithms require diagonal allocations regardless of whether there is non-zero
            // diagonal dependence. With global AD indexing we only add non-zero
            // dependence, so PETSc will scream at us unless we artificially add the diagonals.
            for (auto index : make_range(matrix.row_start(), matrix.row_stop()))
              matrix.add(index, index, 0);
        }

      _aux->zeroVariablesForResidual();

      unsigned int n_threads = libMesh::n_threads();

      _current_execute_on_flag = EXEC_LINEAR;

      // Random interface objects
      for (const auto & it : _random_data_objects)
        it.second->updateSeeds(EXEC_LINEAR);

      setCurrentlyComputingResidual(true);
      setCurrentlyComputingJacobian(true);
      setCurrentlyComputingResidualAndJacobian(true);
      if (_displaced_problem)
      {
        _displaced_problem->setCurrentlyComputingResidual(true);
        _displaced_problem->setCurrentlyComputingJacobian(true);
        _displaced_problem->setCurrentlyComputingResidualAndJacobian(true);
      }

      execTransfers(EXEC_LINEAR);

      execMultiApps(EXEC_LINEAR);

      for (unsigned int tid = 0; tid < n_threads; tid++)
        reinitScalars(tid);

      computeUserObjects(EXEC_LINEAR, Moose::PRE_AUX);

      _aux->residualSetup();

      if (_displaced_problem)
      {
        computeSystems(EXEC_PRE_DISPLACE);
        _displaced_problem->updateMesh();
        if (_mortar_data.hasDisplacedObjects())
          updateMortarMesh();
      }

      for (THREAD_ID tid = 0; tid < n_threads; tid++)
      {
        _all_materials.residualSetup(tid);
        _functions.residualSetup(tid);
      }

      computeSystems(EXEC_LINEAR);

      computeUserObjects(EXEC_LINEAR, Moose::POST_AUX);

      executeControls(EXEC_LINEAR);

      _app.getOutputWarehouse().residualSetup();

      _safe_access_tagged_vectors = false;
      _safe_access_tagged_matrices = false;

      _current_nl_sys->computeResidualAndJacobianTags(_fe_vector_tags, _fe_matrix_tags);

      _current_nl_sys->disassociateMatrixFromTag(jacobian, _current_nl_sys->systemMatrixTag());
      _current_nl_sys->disassociateVectorFromTag(residual, _current_nl_sys->residualVectorTag());
    }
    catch (...)
    {
      handleException("computeResidualAndJacobian");
    }
  }
  catch (const MooseException &)
  {
    // The buck stops here, we have already handled the exception by
    // calling the system's stopSolve() method, it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }
  catch (...)
  {
    mooseError("Unexpected exception type");
  }

  resetState();
  _fe_vector_tags.clear();
  _fe_matrix_tags.clear();
}

void
FEProblemBase::computeResidualTag(const NumericVector<Number> & soln,
                                  NumericVector<Number> & residual,
                                  TagID tag)
{
  try
  {
    _current_nl_sys->setSolution(soln);

    _current_nl_sys->associateVectorToTag(residual, tag);

    computeResidualTags({tag});

    _current_nl_sys->disassociateVectorFromTag(residual, tag);
  }
  catch (MooseException & e)
  {
    // If a MooseException propagates all the way to here, it means
    // that it was thrown from a MOOSE system where we do not
    // (currently) properly support the throwing of exceptions, and
    // therefore we have no choice but to error out.  It may be
    // *possible* to handle exceptions from other systems, but in the
    // meantime, we don't want to silently swallow any unhandled
    // exceptions here.
    mooseError("An unhandled MooseException was raised during residual computation.  Please "
               "contact the MOOSE team for assistance.");
  }
}

void
FEProblemBase::computeResidualInternal(const NumericVector<Number> & soln,
                                       NumericVector<Number> & residual,
                                       const std::set<TagID> & tags)
{
  parallel_object_only();

  TIME_SECTION("computeResidualInternal", 1);

  try
  {
    _current_nl_sys->setSolution(soln);

    _current_nl_sys->associateVectorToTag(residual, _current_nl_sys->residualVectorTag());

    computeResidualTags(tags);

    _current_nl_sys->disassociateVectorFromTag(residual, _current_nl_sys->residualVectorTag());
  }
  catch (MooseException & e)
  {
    // If a MooseException propagates all the way to here, it means
    // that it was thrown from a MOOSE system where we do not
    // (currently) properly support the throwing of exceptions, and
    // therefore we have no choice but to error out.  It may be
    // *possible* to handle exceptions from other systems, but in the
    // meantime, we don't want to silently swallow any unhandled
    // exceptions here.
    mooseError("An unhandled MooseException was raised during residual computation.  Please "
               "contact the MOOSE team for assistance.");
  }
}

void
FEProblemBase::computeResidualType(const NumericVector<Number> & soln,
                                   NumericVector<Number> & residual,
                                   TagID tag)
{
  TIME_SECTION("computeResidualType", 5);

  try
  {
    _current_nl_sys->setSolution(soln);

    _current_nl_sys->associateVectorToTag(residual, _current_nl_sys->residualVectorTag());

    computeResidualTags({tag, _current_nl_sys->residualVectorTag()});

    _current_nl_sys->disassociateVectorFromTag(residual, _current_nl_sys->residualVectorTag());
  }
  catch (MooseException & e)
  {
    // If a MooseException propagates all the way to here, it means
    // that it was thrown from a MOOSE system where we do not
    // (currently) properly support the throwing of exceptions, and
    // therefore we have no choice but to error out.  It may be
    // *possible* to handle exceptions from other systems, but in the
    // meantime, we don't want to silently swallow any unhandled
    // exceptions here.
    mooseError("An unhandled MooseException was raised during residual computation.  Please "
               "contact the MOOSE team for assistance.");
  }
}

void
FEProblemBase::handleException(const std::string & calling_method)
{
  auto create_exception_message =
      [&calling_method](const std::string & exception_type, const auto & exception)
  {
    return std::string("A " + exception_type + " was raised during FEProblemBase::" +
                       calling_method + "\n" + std::string(exception.what()));
  };

  try
  {
    throw;
  }
  catch (const libMesh::LogicError & e)
  {
    setException(create_exception_message("libMesh::LogicError", e));
  }
  catch (const MooseException & e)
  {
    setException(create_exception_message("MooseException", e));
  }
  catch (const MetaPhysicL::LogicError & e)
  {
    moose::translateMetaPhysicLError(e);
  }
  catch (const libMesh::PetscSolverException & e)
  {
    // One PETSc solver exception that we cannot currently recover from are new nonzero errors. In
    // particular I have observed the following scenario in a parallel test:
    // - Both processes throw because of a new nonzero during MOOSE's computeJacobianTags
    // - We potentially handle the exceptions nicely here
    // - When the matrix is closed in libMesh's libmesh_petsc_snes_solver, there is a new nonzero
    //   throw which we do not catch here in MOOSE and the simulation terminates. This only appears
    //   in parallel (and not all the time; a test I was examining threw with distributed mesh, but
    //   not with replicated). In serial there are no new throws from libmesh_petsc_snes_solver.
    // So for uniformity of behavior across serial/parallel, we will choose to abort here and always
    // produce a non-zero exit code
    mooseError(create_exception_message("libMesh::PetscSolverException", e));
  }
  catch (const std::exception & e)
  {
    const auto message = create_exception_message("std::exception", e);
    if (_regard_general_exceptions_as_errors)
      mooseError(message);
    else
      setException(message);
  }

  checkExceptionAndStopSolve();
}

void
FEProblemBase::computeResidualTags(const std::set<TagID> & tags)
{
  parallel_object_only();

  try
  {
    try
    {
      TIME_SECTION("computeResidualTags", 5, "Computing Residual");

      ADReal::do_derivatives = false;

      setCurrentResidualVectorTags(tags);

      _aux->zeroVariablesForResidual();

      unsigned int n_threads = libMesh::n_threads();

      _current_execute_on_flag = EXEC_LINEAR;

      // Random interface objects
      for (const auto & it : _random_data_objects)
        it.second->updateSeeds(EXEC_LINEAR);

      execTransfers(EXEC_LINEAR);

      execMultiApps(EXEC_LINEAR);

      for (unsigned int tid = 0; tid < n_threads; tid++)
        reinitScalars(tid);

      computeUserObjects(EXEC_LINEAR, Moose::PRE_AUX);

      _aux->residualSetup();

      if (_displaced_problem)
      {
        computeSystems(EXEC_PRE_DISPLACE);
        _displaced_problem->updateMesh();
        if (_mortar_data.hasDisplacedObjects())
          updateMortarMesh();
      }

      for (THREAD_ID tid = 0; tid < n_threads; tid++)
      {
        _all_materials.residualSetup(tid);
        _functions.residualSetup(tid);
      }

      computeSystems(EXEC_LINEAR);

      computeUserObjects(EXEC_LINEAR, Moose::POST_AUX);

      executeControls(EXEC_LINEAR);

      _app.getOutputWarehouse().residualSetup();

      _safe_access_tagged_vectors = false;
      _current_nl_sys->computeResidualTags(tags);
    }
    catch (...)
    {
      handleException("computeResidualTags");
    }
  }
  catch (const MooseException &)
  {
    // The buck stops here, we have already handled the exception by
    // calling the system's stopSolve() method, it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }
  catch (...)
  {
    mooseError("Unexpected exception type");
  }

  resetState();
}

void
FEProblemBase::computeJacobianSys(NonlinearImplicitSystem & sys,
                                  const NumericVector<Number> & soln,
                                  SparseMatrix<Number> & jacobian)
{
  computeJacobian(soln, jacobian, sys.number());
}

void
FEProblemBase::computeJacobianTag(const NumericVector<Number> & soln,
                                  SparseMatrix<Number> & jacobian,
                                  TagID tag)
{
  _current_nl_sys->setSolution(soln);

  _current_nl_sys->associateMatrixToTag(jacobian, tag);

  computeJacobianTags({tag});

  _current_nl_sys->disassociateMatrixFromTag(jacobian, tag);
}

void
FEProblemBase::computeJacobian(const NumericVector<Number> & soln,
                               SparseMatrix<Number> & jacobian,
                               const unsigned int nl_sys_num)
{
  setCurrentNonlinearSystem(nl_sys_num);

  _fe_matrix_tags.clear();

  auto & tags = getMatrixTags();
  for (auto & tag : tags)
    _fe_matrix_tags.insert(tag.second);

  computeJacobianInternal(soln, jacobian, _fe_matrix_tags);
}

void
FEProblemBase::computeJacobianInternal(const NumericVector<Number> & soln,
                                       SparseMatrix<Number> & jacobian,
                                       const std::set<TagID> & tags)
{
  TIME_SECTION("computeJacobianInternal", 1);

  _current_nl_sys->setSolution(soln);

  _current_nl_sys->associateMatrixToTag(jacobian, _current_nl_sys->systemMatrixTag());

  computeJacobianTags(tags);

  _current_nl_sys->disassociateMatrixFromTag(jacobian, _current_nl_sys->systemMatrixTag());
}

void
FEProblemBase::computeJacobianTags(const std::set<TagID> & tags)
{
  try
  {
    try
    {
      if (!_has_jacobian || !_const_jacobian)
      {
        TIME_SECTION("computeJacobianTags", 5, "Computing Jacobian");

        for (auto tag : tags)
          if (_current_nl_sys->hasMatrix(tag))
          {
            auto & matrix = _current_nl_sys->getMatrix(tag);
            matrix.zero();
            if (haveADObjects())
              // PETSc algorithms require diagonal allocations regardless of whether there is
              // non-zero diagonal dependence. With global AD indexing we only add non-zero
              // dependence, so PETSc will scream at us unless we artificially add the diagonals.
              for (auto index : make_range(matrix.row_start(), matrix.row_stop()))
                matrix.add(index, index, 0);
          }

        _aux->zeroVariablesForJacobian();

        unsigned int n_threads = libMesh::n_threads();

        // Random interface objects
        for (const auto & it : _random_data_objects)
          it.second->updateSeeds(EXEC_NONLINEAR);

        _current_execute_on_flag = EXEC_NONLINEAR;
        _currently_computing_jacobian = true;
        if (_displaced_problem)
          _displaced_problem->setCurrentlyComputingJacobian(true);

        execTransfers(EXEC_NONLINEAR);
        execMultiApps(EXEC_NONLINEAR);

        for (unsigned int tid = 0; tid < n_threads; tid++)
          reinitScalars(tid);

        computeUserObjects(EXEC_NONLINEAR, Moose::PRE_AUX);

        _aux->jacobianSetup();

        if (_displaced_problem)
        {
          computeSystems(EXEC_PRE_DISPLACE);
          _displaced_problem->updateMesh();
        }

        for (unsigned int tid = 0; tid < n_threads; tid++)
        {
          _all_materials.jacobianSetup(tid);
          _functions.jacobianSetup(tid);
        }

        computeSystems(EXEC_NONLINEAR);

        computeUserObjects(EXEC_NONLINEAR, Moose::POST_AUX);

        executeControls(EXEC_NONLINEAR);

        _app.getOutputWarehouse().jacobianSetup();

        _safe_access_tagged_matrices = false;

        _current_nl_sys->computeJacobianTags(tags);

        // For explicit Euler calculations for example we often compute the Jacobian one time and
        // then re-use it over and over. If we're performing automatic scaling, we don't want to
        // use that kernel, diagonal-block only Jacobian for our actual matrix when performing
        // solves!
        if (!_current_nl_sys->computingScalingJacobian())
          _has_jacobian = true;
      }
    }
    catch (...)
    {
      handleException("computeJacobianTags");
    }
  }
  catch (const MooseException &)
  {
    // The buck stops here, we have already handled the exception by
    // calling the system's stopSolve() method, it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }
  catch (...)
  {
    mooseError("Unexpected exception type");
  }

  resetState();
}

void
FEProblemBase::computeJacobianBlocks(std::vector<JacobianBlock *> & blocks,
                                     const unsigned int nl_sys_num)
{
  TIME_SECTION("computeTransientImplicitJacobian", 2);
  setCurrentNonlinearSystem(nl_sys_num);

  if (_displaced_problem)
  {
    computeSystems(EXEC_PRE_DISPLACE);
    _displaced_problem->updateMesh();
  }

  computeSystems(EXEC_NONLINEAR);

  _currently_computing_jacobian = true;
  _current_nl_sys->computeJacobianBlocks(blocks);
  _currently_computing_jacobian = false;
}

void
FEProblemBase::computeJacobianBlock(SparseMatrix<Number> & jacobian,
                                    libMesh::System & precond_system,
                                    unsigned int ivar,
                                    unsigned int jvar)
{
  JacobianBlock jac_block(precond_system, jacobian, ivar, jvar);
  std::vector<JacobianBlock *> blocks = {&jac_block};
  mooseAssert(_current_nl_sys, "This should be non-null");
  computeJacobianBlocks(blocks, _current_nl_sys->number());
}

void
FEProblemBase::computeBounds(NonlinearImplicitSystem & libmesh_dbg_var(sys),
                             NumericVector<Number> & lower,
                             NumericVector<Number> & upper)
{
  try
  {
    try
    {
      mooseAssert(_current_nl_sys && (sys.number() == _current_nl_sys->number()),
                  "I expect these system numbers to be the same");

      if (!_current_nl_sys->hasVector("lower_bound") || !_current_nl_sys->hasVector("upper_bound"))
        return;

      TIME_SECTION("computeBounds", 1, "Computing Bounds");

      NumericVector<Number> & _lower = _current_nl_sys->getVector("lower_bound");
      NumericVector<Number> & _upper = _current_nl_sys->getVector("upper_bound");
      _lower.swap(lower);
      _upper.swap(upper);
      for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
        _all_materials.residualSetup(tid);

      _aux->residualSetup();
      computeSystems(EXEC_LINEAR);
      _lower.swap(lower);
      _upper.swap(upper);
    }
    catch (...)
    {
      handleException("computeBounds");
    }
  }
  catch (MooseException & e)
  {
    mooseError("Irrecoverable exception: " + std::string(e.what()));
  }
  catch (...)
  {
    mooseError("Unexpected exception type");
  }
}

void
FEProblemBase::computeLinearSystemSys(LinearImplicitSystem & sys,
                                      SparseMatrix<Number> & system_matrix,
                                      NumericVector<Number> & rhs,
                                      const bool compute_gradients)
{
  TIME_SECTION("computeLinearSystemSys", 5);

  setCurrentLinearSystem(linearSysNum(sys.name()));

  // We are using the residual tag system for right hand sides so we fetch everything
  const auto & vector_tags = getVectorTags(Moose::VECTOR_TAG_RESIDUAL);

  // We filter out tags which do not have associated vectors in the current
  // system. This is essential to be able to use system-dependent vector tags.
  selectVectorTagsFromSystem(*_current_linear_sys, vector_tags, _linear_vector_tags);
  selectMatrixTagsFromSystem(*_current_linear_sys, getMatrixTags(), _linear_matrix_tags);

  computeLinearSystemTags(*(_current_linear_sys->currentSolution()),
                          system_matrix,
                          rhs,
                          _linear_vector_tags,
                          _linear_matrix_tags,
                          compute_gradients);
}

void
FEProblemBase::computeLinearSystemTags(const NumericVector<Number> & soln,
                                       SparseMatrix<Number> & system_matrix,
                                       NumericVector<Number> & rhs,
                                       const std::set<TagID> & vector_tags,
                                       const std::set<TagID> & matrix_tags,
                                       const bool compute_gradients)
{
  TIME_SECTION("computeLinearSystemTags", 5, "Computing Linear System");

  _current_linear_sys->setSolution(soln);

  _current_linear_sys->associateVectorToTag(rhs, _current_linear_sys->rightHandSideVectorTag());
  _current_linear_sys->associateMatrixToTag(system_matrix, _current_linear_sys->systemMatrixTag());

  for (const auto tag : matrix_tags)
  {
    auto & matrix = _current_linear_sys->getMatrix(tag);
    matrix.zero();
  }

  unsigned int n_threads = libMesh::n_threads();

  _current_execute_on_flag = EXEC_NONLINEAR;

  // Random interface objects
  for (const auto & it : _random_data_objects)
    it.second->updateSeeds(EXEC_NONLINEAR);

  execTransfers(EXEC_NONLINEAR);
  execMultiApps(EXEC_NONLINEAR);

  computeUserObjects(EXEC_NONLINEAR, Moose::PRE_AUX);

  _aux->jacobianSetup();

  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _functions.jacobianSetup(tid);
  }

  try
  {
    computeSystems(EXEC_NONLINEAR);
  }
  catch (MooseException & e)
  {
    _console << "\nA MooseException was raised during Auxiliary variable computation.\n"
             << "The next solve will fail, the timestep will be reduced, and we will try again.\n"
             << std::endl;

    // We know the next solve is going to fail, so there's no point in
    // computing anything else after this.  Plus, using incompletely
    // computed AuxVariables in subsequent calculations could lead to
    // other errors or unhandled exceptions being thrown.
    return;
  }

  computeUserObjects(EXEC_NONLINEAR, Moose::POST_AUX);
  executeControls(EXEC_NONLINEAR);

  _app.getOutputWarehouse().jacobianSetup();

  _safe_access_tagged_vectors = false;
  _safe_access_tagged_matrices = false;

  _current_linear_sys->computeLinearSystemTags(vector_tags, matrix_tags, compute_gradients);

  _safe_access_tagged_vectors = true;
  _safe_access_tagged_matrices = true;

  _current_linear_sys->disassociateMatrixFromTag(system_matrix,
                                                 _current_linear_sys->systemMatrixTag());
  _current_linear_sys->disassociateVectorFromTag(rhs,
                                                 _current_linear_sys->rightHandSideVectorTag());

  // Reset execution flag as after this point we are no longer on LINEAR
  _current_execute_on_flag = EXEC_NONE;
}

void
FEProblemBase::computeNearNullSpace(NonlinearImplicitSystem & libmesh_dbg_var(sys),
                                    std::vector<NumericVector<Number> *> & sp)
{
  mooseAssert(_current_nl_sys && (sys.number() == _current_nl_sys->number()),
              "I expect these system numbers to be the same");

  sp.clear();
  for (unsigned int i = 0; i < subspaceDim("NearNullSpace"); ++i)
  {
    std::stringstream postfix;
    postfix << "_" << i;
    std::string modename = "NearNullSpace" + postfix.str();
    sp.push_back(&_current_nl_sys->getVector(modename));
  }
}

void
FEProblemBase::computeNullSpace(NonlinearImplicitSystem & libmesh_dbg_var(sys),
                                std::vector<NumericVector<Number> *> & sp)
{
  mooseAssert(_current_nl_sys && (sys.number() == _current_nl_sys->number()),
              "I expect these system numbers to be the same");
  sp.clear();
  for (unsigned int i = 0; i < subspaceDim("NullSpace"); ++i)
  {
    std::stringstream postfix;
    postfix << "_" << i;
    sp.push_back(&_current_nl_sys->getVector("NullSpace" + postfix.str()));
  }
}

void
FEProblemBase::computeTransposeNullSpace(NonlinearImplicitSystem & libmesh_dbg_var(sys),
                                         std::vector<NumericVector<Number> *> & sp)
{
  mooseAssert(_current_nl_sys && (sys.number() == _current_nl_sys->number()),
              "I expect these system numbers to be the same");
  sp.clear();
  for (unsigned int i = 0; i < subspaceDim("TransposeNullSpace"); ++i)
  {
    std::stringstream postfix;
    postfix << "_" << i;
    sp.push_back(&_current_nl_sys->getVector("TransposeNullSpace" + postfix.str()));
  }
}

void
FEProblemBase::computePostCheck(NonlinearImplicitSystem & sys,
                                const NumericVector<Number> & old_soln,
                                NumericVector<Number> & search_direction,
                                NumericVector<Number> & new_soln,
                                bool & changed_search_direction,
                                bool & changed_new_soln)
{
  mooseAssert(_current_nl_sys && (sys.number() == _current_nl_sys->number()),
              "I expect these system numbers to be the same");

  // This function replaces the old PetscSupport::dampedCheck() function.
  //
  // 1.) Recreate code in PetscSupport::dampedCheck() for constructing
  //     ghosted "soln" and "update" vectors.
  // 2.) Call FEProblemBase::computeDamping() with these ghost vectors.
  // 3.) Recreate the code in PetscSupport::dampedCheck() to actually update
  //     the solution vector based on the damping, and set the "changed" flags
  //     appropriately.

  TIME_SECTION("computePostCheck", 2, "Computing Post Check");

  _current_execute_on_flag = EXEC_POSTCHECK;

  // MOOSE's FEProblemBase doesn't update the solution during the
  // postcheck, but FEProblemBase-derived classes might.
  if (_has_dampers || shouldUpdateSolution())
  {
    // We need ghosted versions of new_soln and search_direction (the
    // ones we get from libmesh/PETSc are PARALLEL vectors.  To make
    // our lives simpler, we use the same ghosting pattern as the
    // system's current_local_solution to create new ghosted vectors.

    // Construct zeroed-out clones with the same ghosted dofs as the
    // System's current_local_solution.
    std::unique_ptr<NumericVector<Number>> ghosted_solution =
                                               sys.current_local_solution->zero_clone(),
                                           ghosted_search_direction =
                                               sys.current_local_solution->zero_clone();

    // Copy values from input vectors into clones with ghosted values.
    *ghosted_solution = new_soln;
    *ghosted_search_direction = search_direction;

    if (_has_dampers)
    {
      // Compute the damping coefficient using the ghosted vectors
      Real damping = computeDamping(*ghosted_solution, *ghosted_search_direction);

      // If some non-trivial damping was computed, update the new_soln
      // vector accordingly.
      if (damping < 1.0)
      {
        new_soln = old_soln;
        new_soln.add(-damping, search_direction);
        changed_new_soln = true;
      }
    }

    if (shouldUpdateSolution())
    {
      // Update the ghosted copy of the new solution, if necessary.
      if (changed_new_soln)
        *ghosted_solution = new_soln;

      bool updated_solution = updateSolution(new_soln, *ghosted_solution);
      if (updated_solution)
        changed_new_soln = true;
    }
  }

  if (vectorTagExists(Moose::PREVIOUS_NL_SOLUTION_TAG))
  {
    _current_nl_sys->setPreviousNewtonSolution(old_soln);
    _aux->copyCurrentIntoPreviousNL();
  }

  // MOOSE doesn't change the search_direction
  changed_search_direction = false;

  _current_execute_on_flag = EXEC_NONE;
}

Real
FEProblemBase::computeDamping(const NumericVector<Number> & soln,
                              const NumericVector<Number> & update)
{
  // Default to no damping
  Real damping = 1.0;

  if (_has_dampers)
  {
    TIME_SECTION("computeDamping", 1, "Computing Damping");

    // Save pointer to the current solution
    const NumericVector<Number> * _saved_current_solution = _current_nl_sys->currentSolution();

    _current_nl_sys->setSolution(soln);
    // For now, do not re-compute auxiliary variables.  Doing so allows a wild solution increment
    //   to get to the material models, which may not be able to cope with drastically different
    //   values.  Once more complete dependency checking is in place, auxiliary variables (and
    //   material properties) will be computed as needed by dampers.
    //    _aux.compute();
    damping = _current_nl_sys->computeDamping(soln, update);

    // restore saved solution
    _current_nl_sys->setSolution(*_saved_current_solution);
  }

  return damping;
}

bool
FEProblemBase::shouldUpdateSolution()
{
  return false;
}

bool
FEProblemBase::updateSolution(NumericVector<Number> & /*vec_solution*/,
                              NumericVector<Number> & /*ghosted_solution*/)
{
  return false;
}

void
FEProblemBase::predictorCleanup(NumericVector<Number> & /*ghosted_solution*/)
{
}

void
FEProblemBase::addDisplacedProblem(std::shared_ptr<DisplacedProblem> displaced_problem)
{
  parallel_object_only();

  _displaced_mesh = &displaced_problem->mesh();
  _displaced_problem = displaced_problem;
}

void
FEProblemBase::updateGeomSearch(GeometricSearchData::GeometricSearchType type)
{
  TIME_SECTION("updateGeometricSearch", 3, "Updating Geometric Search");

  _geometric_search_data.update(type);

  if (_displaced_problem)
    _displaced_problem->updateGeomSearch(type);
}

void
FEProblemBase::updateMortarMesh()
{
  TIME_SECTION("updateMortarMesh", 5, "Updating Mortar Mesh");

  FloatingPointExceptionGuard fpe_guard(_app);

  _mortar_data.update();
}

void
FEProblemBase::createMortarInterface(
    const std::pair<BoundaryID, BoundaryID> & primary_secondary_boundary_pair,
    const std::pair<SubdomainID, SubdomainID> & primary_secondary_subdomain_pair,
    bool on_displaced,
    bool periodic,
    const bool debug,
    const bool correct_edge_dropping,
    const Real minimum_projection_angle)
{
  _has_mortar = true;

  if (on_displaced)
    return _mortar_data.createMortarInterface(primary_secondary_boundary_pair,
                                              primary_secondary_subdomain_pair,
                                              *_displaced_problem,
                                              on_displaced,
                                              periodic,
                                              debug,
                                              correct_edge_dropping,
                                              minimum_projection_angle);
  else
    return _mortar_data.createMortarInterface(primary_secondary_boundary_pair,
                                              primary_secondary_subdomain_pair,
                                              *this,
                                              on_displaced,
                                              periodic,
                                              debug,
                                              correct_edge_dropping,
                                              minimum_projection_angle);
}

const AutomaticMortarGeneration &
FEProblemBase::getMortarInterface(
    const std::pair<BoundaryID, BoundaryID> & primary_secondary_boundary_pair,
    const std::pair<SubdomainID, SubdomainID> & primary_secondary_subdomain_pair,
    bool on_displaced) const
{
  return _mortar_data.getMortarInterface(
      primary_secondary_boundary_pair, primary_secondary_subdomain_pair, on_displaced);
}

AutomaticMortarGeneration &
FEProblemBase::getMortarInterface(
    const std::pair<BoundaryID, BoundaryID> & primary_secondary_boundary_pair,
    const std::pair<SubdomainID, SubdomainID> & primary_secondary_subdomain_pair,
    bool on_displaced)
{
  return _mortar_data.getMortarInterface(
      primary_secondary_boundary_pair, primary_secondary_subdomain_pair, on_displaced);
}

const std::unordered_map<std::pair<BoundaryID, BoundaryID>, AutomaticMortarGeneration> &
FEProblemBase::getMortarInterfaces(bool on_displaced) const
{
  return _mortar_data.getMortarInterfaces(on_displaced);
}

void
FEProblemBase::possiblyRebuildGeomSearchPatches()
{
  if (_displaced_problem) // Only need to do this if things are moving...
  {
    TIME_SECTION("possiblyRebuildGeomSearchPatches", 5, "Rebuilding Geometric Search Patches");

    switch (_mesh.getPatchUpdateStrategy())
    {
      case Moose::Never:
        break;
      case Moose::Iteration:
        // Update the list of ghosted elements at the start of the time step
        _geometric_search_data.updateGhostedElems();
        _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);

        _displaced_problem->geomSearchData().updateGhostedElems();
        _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);

        // The commands below ensure that the sparsity of the Jacobian matrix is
        // augmented at the start of the time step using neighbor nodes from the end
        // of the previous time step.

        reinitBecauseOfGhostingOrNewGeomObjects();

        // This is needed to reinitialize PETSc output
        initPetscOutputAndSomeSolverSettings();

        break;

      case Moose::Auto:
      {
        Real max = _displaced_problem->geomSearchData().maxPatchPercentage();
        _communicator.max(max);

        // If we haven't moved very far through the patch
        if (max < 0.4)
          break;
      }
        libmesh_fallthrough();

      // Let this fall through if things do need to be updated...
      case Moose::Always:
        // Flush output here to see the message before the reinitialization, which could take a
        // while
        _console << "\n\nUpdating geometric search patches\n" << std::endl;

        _geometric_search_data.clearNearestNodeLocators();
        _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);

        _displaced_problem->geomSearchData().clearNearestNodeLocators();
        _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);

        reinitBecauseOfGhostingOrNewGeomObjects();

        // This is needed to reinitialize PETSc output
        initPetscOutputAndSomeSolverSettings();
    }
  }
}

#ifdef LIBMESH_ENABLE_AMR
void
FEProblemBase::initialAdaptMesh()
{
  unsigned int n = adaptivity().getInitialSteps();
  _cycles_completed = 0;
  if (n)
  {
    if (!_mesh.interiorLowerDBlocks().empty() || !_mesh.boundaryLowerDBlocks().empty())
      mooseError("HFEM does not support mesh adaptivity currently.");

    TIME_SECTION("initialAdaptMesh", 2, "Performing Initial Adaptivity");

    for (unsigned int i = 0; i < n; i++)
    {
      computeIndicators();
      computeMarkers();

      if (_adaptivity.initialAdaptMesh())
      {
        meshChanged();

        // reproject the initial condition
        projectSolution();

        _cycles_completed++;
      }
      else
      {
        _console << "Mesh unchanged, skipping remaining steps..." << std::endl;
        return;
      }
    }
  }
}

bool
FEProblemBase::adaptMesh()
{
  // reset cycle counter
  _cycles_completed = 0;

  if (!_adaptivity.isAdaptivityDue())
    return false;

  TIME_SECTION("adaptMesh", 3, "Adapting Mesh");

  unsigned int cycles_per_step = _adaptivity.getCyclesPerStep();

  bool mesh_changed = false;

  for (unsigned int i = 0; i < cycles_per_step; ++i)
  {
    if (!_mesh.interiorLowerDBlocks().empty() || !_mesh.boundaryLowerDBlocks().empty())
      mooseError("HFEM does not support mesh adaptivity currently.");

    // Markers were already computed once by Executioner
    if (_adaptivity.getRecomputeMarkersFlag() && i > 0)
      computeMarkers();

    bool mesh_changed_this_step;
    mesh_changed_this_step = _adaptivity.adaptMesh();

    if (mesh_changed_this_step)
    {
      mesh_changed = true;

      meshChangedHelper(true); // This may be an intermediate change
      _cycles_completed++;
    }
    else
    {
      _console << "Mesh unchanged, skipping remaining steps..." << std::endl;
      break;
    }

    // Show adaptivity progress
    _console << std::flush;
  }

  // We're done with all intermediate changes; now get systems ready
  // for real if necessary.
  if (mesh_changed)
    es().reinit_systems();

  return mesh_changed;
}
#endif // LIBMESH_ENABLE_AMR

void
FEProblemBase::initXFEM(std::shared_ptr<XFEMInterface> xfem)
{
  _xfem = xfem;
  _xfem->setMesh(&_mesh);
  if (_displaced_mesh)
    _xfem->setDisplacedMesh(_displaced_mesh);

  auto fill_data = [](auto & storage)
  {
    std::vector<MaterialData *> data(libMesh::n_threads());
    for (const auto tid : make_range(libMesh::n_threads()))
      data[tid] = &storage.getMaterialData(tid);
    return data;
  };
  _xfem->setMaterialData(fill_data(_material_props));
  _xfem->setBoundaryMaterialData(fill_data(_bnd_material_props));

  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; ++i)
    for (const auto nl_sys_num : index_range(_nl))
    {
      _assembly[i][nl_sys_num]->setXFEM(_xfem);
      if (_displaced_problem)
        _displaced_problem->assembly(i, nl_sys_num).setXFEM(_xfem);
    }
}

bool
FEProblemBase::updateMeshXFEM()
{
  TIME_SECTION("updateMeshXFEM", 5, "Updating XFEM");

  bool updated = false;
  if (haveXFEM())
  {
    if (_xfem->updateHeal())
      meshChanged();

    updated = _xfem->update(_time, _nl, *_aux);
    if (updated)
    {
      meshChanged();
      _xfem->initSolution(_nl, *_aux);
      restoreSolutions();
    }
  }
  return updated;
}

void
FEProblemBase::meshChanged()
{
  TIME_SECTION("meshChanged", 3, "Handling Mesh Changes");

  this->meshChangedHelper();
}

void
FEProblemBase::meshChangedHelper(bool intermediate_change)
{
  TIME_SECTION("meshChangedHelper", 5);

  if (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties() ||
      _neighbor_material_props.hasStatefulProperties())
    _mesh.cacheChangedLists(); // Currently only used with adaptivity and stateful material
                               // properties

  // Clear these out because they corresponded to the old mesh
  _ghosted_elems.clear();
  ghostGhostedBoundaries();

  // The mesh changed.  We notify the MooseMesh first, because
  // callbacks (e.g. for sparsity calculations) triggered by the
  // EquationSystems reinit may require up-to-date MooseMesh caches.
  _mesh.meshChanged();

  // If we're just going to alter the mesh again, all we need to
  // handle here is AMR and projections, not full system reinit
  if (intermediate_change)
    es().reinit_solutions();
  else
  {
    es().reinit();
    // Since the mesh has changed, we need to make sure that we update any of our
    // MOOSE-system specific data.
    for (auto & sys : _solver_systems)
      sys->reinit();
    _aux->reinit();
  }

  // Updating MooseMesh first breaks other adaptivity code, unless we
  // then *again* update the MooseMesh caches.  E.g. the definition of
  // "active" and "local" may have been *changed* by refinement and
  // repartitioning done in EquationSystems::reinit().
  _mesh.meshChanged();

  // If we have finite volume variables, we will need to recompute additional elemental/face
  // quantities
  if (haveFV() && _mesh.isFiniteVolumeInfoDirty())
    _mesh.setupFiniteVolumeMeshData();

  // Let the meshChangedInterface notify the mesh changed event before we update the active
  // semilocal nodes, because the set of ghosted elements may potentially be updated during a mesh
  // changed event.
  for (const auto & mci : _notify_when_mesh_changes)
    mci->meshChanged();

  // Since the Mesh changed, update the PointLocator object used by DiracKernels.
  _dirac_kernel_info.updatePointLocator(_mesh);

  // Need to redo ghosting
  _geometric_search_data.reinit();

  if (_displaced_problem)
  {
    _displaced_problem->meshChanged();
    _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);
  }

  _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);

  _evaluable_local_elem_range.reset();
  _nl_evaluable_local_elem_range.reset();

  // Just like we reinitialized our geometric search objects, we also need to reinitialize our
  // mortar meshes. Note that this needs to happen after DisplacedProblem::meshChanged because the
  // mortar mesh discretization will depend necessarily on the displaced mesh being re-displaced
  updateMortarMesh();

  reinitBecauseOfGhostingOrNewGeomObjects(/*mortar_changed=*/true);

  // We need to create new storage for newly active elements, and copy
  // stateful properties from the old elements.
  if (_has_initialized_stateful &&
      (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties()))
  {
    if (havePRefinement())
      _mesh.buildPRefinementAndCoarseningMaps(_assembly[0][0].get());

    // Prolong properties onto newly refined elements' children
    {
      ProjectMaterialProperties pmp(
          /* refine = */ true, *this, _material_props, _bnd_material_props, _assembly);
      const auto & range = *_mesh.refinedElementRange();
      Threads::parallel_reduce(range, pmp);

      // Concurrent erasure from the shared hash map is not safe while we are reading from it in
      // ProjectMaterialProperties, so we handle erasure here. Moreover, erasure based on key is
      // not thread safe in and of itself because it is a read-write operation. Note that we do not
      // do the erasure for p-refinement because the coarse level element is the same as our active
      // refined level element
      if (!doingPRefinement())
        for (const auto & elem : range)
        {
          _material_props.eraseProperty(elem);
          _bnd_material_props.eraseProperty(elem);
          _neighbor_material_props.eraseProperty(elem);
        }
    }

    // Restrict properties onto newly coarsened elements
    {
      ProjectMaterialProperties pmp(
          /* refine = */ false, *this, _material_props, _bnd_material_props, _assembly);
      const auto & range = *_mesh.coarsenedElementRange();
      Threads::parallel_reduce(range, pmp);
      // Note that we do not do the erasure for p-refinement because the coarse level element is the
      // same as our active refined level element
      if (!doingPRefinement())
        for (const auto & elem : range)
        {
          auto && coarsened_children = _mesh.coarsenedElementChildren(elem);
          for (auto && child : coarsened_children)
          {
            _material_props.eraseProperty(child);
            _bnd_material_props.eraseProperty(child);
            _neighbor_material_props.eraseProperty(child);
          }
        }
    }
  }

  if (_calculate_jacobian_in_uo)
    setVariableAllDoFMap(_uo_jacobian_moose_vars[0]);

  _has_jacobian = false; // we have to recompute jacobian when mesh changed
}

void
FEProblemBase::notifyWhenMeshChanges(MeshChangedInterface * mci)
{
  _notify_when_mesh_changes.push_back(mci);
}

void
FEProblemBase::initElementStatefulProps(const ConstElemRange & elem_range, const bool threaded)
{
  ComputeMaterialsObjectThread cmt(
      *this, _material_props, _bnd_material_props, _neighbor_material_props, _assembly);
  if (threaded)
    Threads::parallel_reduce(elem_range, cmt);
  else
    cmt(elem_range, true);
}

void
FEProblemBase::checkProblemIntegrity()
{
  TIME_SECTION("checkProblemIntegrity", 5);

  // Check for unsatisfied actions
  const std::set<SubdomainID> & mesh_subdomains = _mesh.meshSubdomains();

  // Check kernel coverage of subdomains (blocks) in the mesh
  if (!_skip_nl_system_check && _solve && _kernel_coverage_check != CoverageCheckMode::FALSE &&
      _kernel_coverage_check != CoverageCheckMode::OFF)
  {
    std::set<SubdomainID> blocks;
    if (_kernel_coverage_check == CoverageCheckMode::TRUE ||
        _kernel_coverage_check == CoverageCheckMode::ON)
      blocks = mesh_subdomains;
    else if (_kernel_coverage_check == CoverageCheckMode::SKIP_LIST)
    {
      blocks = mesh_subdomains;
      for (const auto & subdomain_name : _kernel_coverage_blocks)
      {
        const auto id = _mesh.getSubdomainID(subdomain_name);
        if (id == Moose::INVALID_BLOCK_ID)
          paramError("kernel_coverage_block_list",
                     "Subdomain \"" + subdomain_name + "\" not found in mesh.");
        blocks.erase(id);
      }
    }
    else if (_kernel_coverage_check == CoverageCheckMode::ONLY_LIST)
      for (const auto & subdomain_name : _kernel_coverage_blocks)
      {
        const auto id = _mesh.getSubdomainID(subdomain_name);
        if (id == Moose::INVALID_BLOCK_ID)
          paramError("kernel_coverage_block_list",
                     "Subdomain \"" + subdomain_name + "\" not found in mesh.");
        blocks.insert(id);
      }
    if (!blocks.empty())
      for (auto & nl : _nl)
        nl->checkKernelCoverage(blocks);
  }

  // Check materials
  {
#ifdef LIBMESH_ENABLE_AMR
    if ((_adaptivity.isOn() || _num_grid_steps) &&
        (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties() ||
         _neighbor_material_props.hasStatefulProperties()))
    {
      _console << "Using EXPERIMENTAL Stateful Material Property projection with Adaptivity!\n"
               << std::flush;
    }
#endif

    std::set<SubdomainID> local_mesh_subs(mesh_subdomains);

    if (_material_coverage_check != CoverageCheckMode::FALSE &&
        _material_coverage_check != CoverageCheckMode::OFF)
    {
      /**
       * If a material is specified for any block in the simulation, then all blocks must
       * have a material specified.
       */
      bool check_material_coverage = false;
      std::set<SubdomainID> ids = _all_materials.getActiveBlocks();
      for (const auto & id : ids)
      {
        local_mesh_subs.erase(id);
        check_material_coverage = true;
      }

      // did the user limit the subdomains to be checked?
      if (_material_coverage_check == CoverageCheckMode::SKIP_LIST)
      {
        for (const auto & subdomain_name : _material_coverage_blocks)
        {
          const auto id = _mesh.getSubdomainID(subdomain_name);
          if (id == Moose::INVALID_BLOCK_ID)
            paramError("material_coverage_block_list",
                       "Subdomain \"" + subdomain_name + "\" not found in mesh.");
          local_mesh_subs.erase(id);
        }
      }
      else if (_material_coverage_check == CoverageCheckMode::ONLY_LIST)
      {
        std::set<SubdomainID> blocks(local_mesh_subs);
        for (const auto & subdomain_name : _material_coverage_blocks)
        {
          const auto id = _mesh.getSubdomainID(subdomain_name);
          if (id == Moose::INVALID_BLOCK_ID)
            paramError("material_coverage_block_list",
                       "Subdomain \"" + subdomain_name + "\" not found in mesh.");
          blocks.erase(id);
        }
        for (const auto id : blocks)
          local_mesh_subs.erase(id);
      }

      // also exclude mortar spaces from the material check
      auto && mortar_subdomain_ids = _mortar_data.getMortarSubdomainIDs();
      for (auto subdomain_id : mortar_subdomain_ids)
        local_mesh_subs.erase(subdomain_id);

      // Check Material Coverage
      if (check_material_coverage && !local_mesh_subs.empty())
      {
        std::stringstream extra_subdomain_ids;
        /// unsigned int is necessary to print SubdomainIDs in the statement below
        std::copy(local_mesh_subs.begin(),
                  local_mesh_subs.end(),
                  std::ostream_iterator<unsigned int>(extra_subdomain_ids, " "));

        mooseError("The following blocks from your input mesh do not contain an active material: " +
                   extra_subdomain_ids.str() +
                   "\nWhen ANY mesh block contains a Material object, "
                   "all blocks must contain a Material object.\n");
      }
    }

    // Check material properties on blocks and boundaries
    checkBlockMatProps();
    checkBoundaryMatProps();

    // Check that material properties exist when requested by other properties on a given block
    const auto & materials = _all_materials.getActiveObjects();
    for (const auto & material : materials)
      material->checkStatefulSanity();

    // auto mats_to_check = _materials.getActiveBlockObjects();
    // const auto & discrete_materials = _discrete_materials.getActiveBlockObjects();
    // for (const auto & map_it : discrete_materials)
    //   for (const auto & container_element : map_it.second)
    //     mats_to_check[map_it.first].push_back(container_element);
    if (_material_dependency_check)
      checkDependMaterialsHelper(_all_materials.getActiveBlockObjects());
  }

  checkUserObjects();

  // Verify that we don't have any Element type/Coordinate Type conflicts
  checkCoordinateSystems();

  // If using displacements, verify that the order of the displacement
  // variables matches the order of the elements in the displaced
  // mesh.
  checkDisplacementOrders();

  // Check for postprocessor names with same name as a scalar variable
  checkDuplicatePostprocessorVariableNames();
}

void
FEProblemBase::checkDisplacementOrders()
{
  if (_displaced_problem)
  {
    bool mesh_has_second_order_elements = false;
    for (const auto & elem : as_range(_displaced_mesh->activeLocalElementsBegin(),
                                      _displaced_mesh->activeLocalElementsEnd()))
    {
      if (elem->default_order() == SECOND)
      {
        mesh_has_second_order_elements = true;
        break;
      }
    }

    // We checked our local elements, so take the max over all processors.
    _displaced_mesh->comm().max(mesh_has_second_order_elements);

    // If the Mesh has second order elements, make sure the
    // displacement variables are second-order.
    if (mesh_has_second_order_elements)
    {
      const std::vector<std::string> & displacement_variables =
          _displaced_problem->getDisplacementVarNames();

      for (const auto & var_name : displacement_variables)
      {
        MooseVariableFEBase & mv =
            _displaced_problem->getVariable(/*tid=*/0,
                                            var_name,
                                            Moose::VarKindType::VAR_ANY,
                                            Moose::VarFieldType::VAR_FIELD_STANDARD);
        if (mv.order() != SECOND)
          mooseError("Error: mesh has SECOND order elements, so all displacement variables must be "
                     "SECOND order.");
      }
    }
  }
}

void
FEProblemBase::checkUserObjects()
{
  // Check user_objects block coverage
  std::set<SubdomainID> mesh_subdomains = _mesh.meshSubdomains();
  std::set<SubdomainID> user_objects_blocks;

  // gather names of all user_objects that were defined in the input file
  // and the blocks that they are defined on
  std::set<std::string> names;

  std::vector<UserObject *> objects;
  theWarehouse().query().condition<AttribInterfaces>(Interfaces::UserObject).queryInto(objects);

  for (const auto & obj : objects)
    names.insert(obj->name());

  // See if all referenced blocks are covered
  std::set<SubdomainID> difference;
  std::set_difference(user_objects_blocks.begin(),
                      user_objects_blocks.end(),
                      mesh_subdomains.begin(),
                      mesh_subdomains.end(),
                      std::inserter(difference, difference.end()));

  if (!difference.empty())
  {
    std::ostringstream oss;
    oss << "One or more UserObjects is referencing a nonexistent block:\n";
    for (const auto & id : difference)
      oss << id << "\n";
    mooseError(oss.str());
  }
}

void
FEProblemBase::checkDependMaterialsHelper(
    const std::map<SubdomainID, std::vector<std::shared_ptr<MaterialBase>>> & materials_map)
{
  for (const auto & it : materials_map)
  {
    /// These two sets are used to make sure that all dependent props on a block are actually supplied
    std::set<std::string> block_depend_props, block_supplied_props;

    for (const auto & mat1 : it.second)
    {
      const std::set<std::string> & depend_props = mat1->getRequestedItems();
      block_depend_props.insert(depend_props.begin(), depend_props.end());

      auto & alldeps = mat1->getMatPropDependencies(); // includes requested stateful props
      for (auto & dep : alldeps)
        if (const auto name = _material_props.queryStatefulPropName(dep))
          block_depend_props.insert(*name);

      // See if any of the active materials supply this property
      for (const auto & mat2 : it.second)
      {
        const std::set<std::string> & supplied_props = mat2->MaterialBase::getSuppliedItems();
        block_supplied_props.insert(supplied_props.begin(), supplied_props.end());
      }
    }

    // Add zero material properties specific to this block and unrestricted
    block_supplied_props.insert(_zero_block_material_props[it.first].begin(),
                                _zero_block_material_props[it.first].end());

    // Error check to make sure all properties consumed by materials are supplied on this block
    std::set<std::string> difference;
    std::set_difference(block_depend_props.begin(),
                        block_depend_props.end(),
                        block_supplied_props.begin(),
                        block_supplied_props.end(),
                        std::inserter(difference, difference.end()));

    if (!difference.empty())
    {
      std::ostringstream oss;
      oss << "One or more Material Properties were not supplied on block ";
      const std::string & subdomain_name = _mesh.getSubdomainName(it.first);
      if (subdomain_name.length() > 0)
        oss << subdomain_name << " (" << it.first << ")";
      else
        oss << it.first;
      oss << ":\n";
      for (const auto & name : difference)
        oss << name << "\n";
      mooseError(oss.str());
    }
  }

  // This loop checks that materials are not supplied by multiple Material objects
  for (const auto & it : materials_map)
  {
    const auto & materials = it.second;
    std::set<std::string> inner_supplied, outer_supplied;

    for (const auto & outer_mat : materials)
    {
      // Storage for properties for this material (outer) and all other materials (inner)
      outer_supplied = outer_mat->getSuppliedItems();
      inner_supplied.clear();

      // Property to material map for error reporting
      std::map<std::string, std::set<std::string>> prop_to_mat;
      for (const auto & name : outer_supplied)
        prop_to_mat[name].insert(outer_mat->name());

      for (const auto & inner_mat : materials)
      {
        if (outer_mat == inner_mat)
          continue;

        // Check whether these materials are an AD pair
        auto outer_mat_type = outer_mat->type();
        auto inner_mat_type = inner_mat->type();
        removeSubstring(outer_mat_type, "<RESIDUAL>");
        removeSubstring(outer_mat_type, "<JACOBIAN>");
        removeSubstring(inner_mat_type, "<RESIDUAL>");
        removeSubstring(inner_mat_type, "<JACOBIAN>");
        if (outer_mat_type == inner_mat_type && outer_mat_type != outer_mat->type() &&
            inner_mat_type != inner_mat->type())
          continue;

        inner_supplied.insert(inner_mat->getSuppliedItems().begin(),
                              inner_mat->getSuppliedItems().end());

        for (const auto & inner_supplied_name : inner_supplied)
          prop_to_mat[inner_supplied_name].insert(inner_mat->name());
      }

      // Test that a property isn't supplied on multiple blocks
      std::set<std::string> intersection;
      std::set_intersection(outer_supplied.begin(),
                            outer_supplied.end(),
                            inner_supplied.begin(),
                            inner_supplied.end(),
                            std::inserter(intersection, intersection.end()));

      if (!intersection.empty())
      {
        std::ostringstream oss;
        oss << "The following material properties are declared on block " << it.first
            << " by multiple materials:\n";
        oss << ConsoleUtils::indent(2) << std::setw(30) << std::left << "Material Property"
            << "Material Objects\n";
        for (const auto & outer_name : intersection)
        {
          oss << ConsoleUtils::indent(2) << std::setw(30) << std::left << outer_name;
          for (const auto & inner_name : prop_to_mat[outer_name])
            oss << inner_name << " ";
          oss << '\n';
        }

        mooseError(oss.str());
        break;
      }
    }
  }
}

void
FEProblemBase::checkCoordinateSystems()
{
  _mesh.checkCoordinateSystems();
}

void
FEProblemBase::setRestartFile(const std::string & file_name)
{
  if (_app.isRecovering())
  {
    mooseInfo("Restart file ", file_name, " is NOT being used since we are performing recovery.");
  }
  else
  {
    _app.setRestart(true);
    _app.setRestartRecoverFileBase(file_name);
    mooseInfo("Using ", file_name, " for restart.");
  }
}

std::vector<VariableName>
FEProblemBase::getVariableNames()
{
  std::vector<VariableName> names;

  for (auto & sys : _solver_systems)
  {
    const std::vector<VariableName> & var_names = sys->getVariableNames();
    names.insert(names.end(), var_names.begin(), var_names.end());
  }

  const std::vector<VariableName> & aux_var_names = _aux->getVariableNames();
  names.insert(names.end(), aux_var_names.begin(), aux_var_names.end());

  return names;
}

SolverParams &
FEProblemBase::solverParams()
{
  return _solver_params;
}

const SolverParams &
FEProblemBase::solverParams() const
{
  return _solver_params;
}

void
FEProblemBase::registerRandomInterface(RandomInterface & random_interface, const std::string & name)
{
  auto insert_pair = moose_try_emplace(
      _random_data_objects, name, std::make_unique<RandomData>(*this, random_interface));

  auto random_data_ptr = insert_pair.first->second.get();
  random_interface.setRandomDataPointer(random_data_ptr);
}

bool
FEProblemBase::needBoundaryMaterialOnSide(BoundaryID bnd_id, const THREAD_ID tid)
{
  if (_bnd_mat_side_cache[tid].find(bnd_id) == _bnd_mat_side_cache[tid].end())
  {
    auto & bnd_mat_side_cache = _bnd_mat_side_cache[tid][bnd_id];
    bnd_mat_side_cache = false;

    if (_aux->needMaterialOnSide(bnd_id))
    {
      bnd_mat_side_cache = true;
      return true;
    }
    else
      for (auto & nl : _nl)
        if (nl->needBoundaryMaterialOnSide(bnd_id, tid))
        {
          bnd_mat_side_cache = true;
          return true;
        }

    if (theWarehouse()
            .query()
            .condition<AttribThread>(tid)
            .condition<AttribInterfaces>(Interfaces::SideUserObject)
            .condition<AttribBoundaries>(bnd_id)
            .count() > 0)
    {
      bnd_mat_side_cache = true;
      return true;
    }
  }

  return _bnd_mat_side_cache[tid][bnd_id];
}

bool
FEProblemBase::needInterfaceMaterialOnSide(BoundaryID bnd_id, const THREAD_ID tid)
{
  if (_interface_mat_side_cache[tid].find(bnd_id) == _interface_mat_side_cache[tid].end())
  {
    auto & interface_mat_side_cache = _interface_mat_side_cache[tid][bnd_id];
    interface_mat_side_cache = false;

    for (auto & nl : _nl)
      if (nl->needInterfaceMaterialOnSide(bnd_id, tid))
      {
        interface_mat_side_cache = true;
        return true;
      }

    if (theWarehouse()
            .query()
            .condition<AttribThread>(tid)
            .condition<AttribInterfaces>(Interfaces::InterfaceUserObject)
            .condition<AttribBoundaries>(bnd_id)
            .count() > 0)
    {
      interface_mat_side_cache = true;
      return true;
    }
    else if (_interface_materials.hasActiveBoundaryObjects(bnd_id, tid))
    {
      interface_mat_side_cache = true;
      return true;
    }
  }
  return _interface_mat_side_cache[tid][bnd_id];
}

bool
FEProblemBase::needSubdomainMaterialOnSide(SubdomainID subdomain_id, const THREAD_ID tid)
{
  if (_block_mat_side_cache[tid].find(subdomain_id) == _block_mat_side_cache[tid].end())
  {
    _block_mat_side_cache[tid][subdomain_id] = false;

    for (auto & nl : _nl)
      if (nl->needSubdomainMaterialOnSide(subdomain_id, tid))
      {
        _block_mat_side_cache[tid][subdomain_id] = true;
        return true;
      }

    if (theWarehouse()
            .query()
            .condition<AttribThread>(tid)
            .condition<AttribInterfaces>(Interfaces::InternalSideUserObject)
            .condition<AttribSubdomains>(subdomain_id)
            .count() > 0)
    {
      _block_mat_side_cache[tid][subdomain_id] = true;
      return true;
    }
  }

  return _block_mat_side_cache[tid][subdomain_id];
}

bool
FEProblemBase::needsPreviousNewtonIteration() const
{
  return vectorTagExists(Moose::PREVIOUS_NL_SOLUTION_TAG);
}

void
FEProblemBase::needsPreviousNewtonIteration(bool state)
{
  if (state && !vectorTagExists(Moose::PREVIOUS_NL_SOLUTION_TAG))
    mooseError("Previous nonlinear solution is required but not added through "
               "Problem/previous_nl_solution_required=true");
}

bool
FEProblemBase::hasJacobian() const
{
  return _has_jacobian;
}

bool
FEProblemBase::constJacobian() const
{
  return _const_jacobian;
}

void
FEProblemBase::addOutput(const std::string & object_type,
                         const std::string & object_name,
                         InputParameters & parameters)
{
  parallel_object_only();

  // Get a reference to the OutputWarehouse
  OutputWarehouse & output_warehouse = _app.getOutputWarehouse();

  // Reject the reserved names for objects not built by MOOSE
  if (!parameters.get<bool>("_built_by_moose") && output_warehouse.isReservedName(object_name))
    mooseError("The name '", object_name, "' is a reserved name for output objects");

  // Check that an object by the same name does not already exist; this must be done before the
  // object is created to avoid getting misleading errors from the Parser
  if (output_warehouse.hasOutput(object_name))
    mooseError("An output object named '", object_name, "' already exists");

  // Add a pointer to the FEProblemBase class
  parameters.addPrivateParam<FEProblemBase *>("_fe_problem_base", this);

  // Create common parameter exclude list
  std::vector<std::string> exclude;
  if (object_type == "Console")
  {
    exclude.push_back("execute_on");

    // --show-input should enable the display of the input file on the screen
    if (_app.getParam<bool>("show_input") && parameters.get<bool>("output_screen"))
      parameters.set<ExecFlagEnum>("execute_input_on") = EXEC_INITIAL;
  }
  // Need this because Checkpoint::validParams changes the default value of
  // execute_on
  else if (object_type == "Checkpoint")
    exclude.push_back("execute_on");

  // Apply the common parameters loaded with Outputs input syntax
  const InputParameters * common = output_warehouse.getCommonParameters();
  if (common)
    parameters.applyParameters(*common, exclude);
  if (common && std::find(exclude.begin(), exclude.end(), "execute_on") != exclude.end() &&
      common->isParamSetByUser("execute_on") && object_type != "Console")
    mooseInfoRepeated(
        "'execute_on' parameter specified in [Outputs] block is ignored for object '" +
        object_name +
        "'.\nDefine this object in its own sub-block of [Outputs] to modify its "
        "execution schedule.");

  // Set the correct value for the binary flag for XDA/XDR output
  if (object_type == "XDR")
    parameters.set<bool>("_binary") = true;
  else if (object_type == "XDA")
    parameters.set<bool>("_binary") = false;

  // Adjust the checkpoint suffix if auto recovery was enabled
  if (object_name == "auto_recovery_checkpoint")
    parameters.set<std::string>("suffix") = "auto_recovery";

  // Create the object and add it to the warehouse
  std::shared_ptr<Output> output = _factory.create<Output>(object_type, object_name, parameters);
  logAdd("Output", object_name, object_type, parameters);
  output_warehouse.addOutput(output);
}

void
FEProblemBase::haveADObjects(const bool have_ad_objects)
{
  _have_ad_objects = have_ad_objects;
  if (_displaced_problem)
    _displaced_problem->SubProblem::haveADObjects(have_ad_objects);
}

const SystemBase &
FEProblemBase::systemBaseNonlinear(const unsigned int sys_num) const
{
  mooseAssert(sys_num < _nl.size(), "System number greater than the number of nonlinear systems");
  return *_nl[sys_num];
}

SystemBase &
FEProblemBase::systemBaseNonlinear(const unsigned int sys_num)
{
  return *_nl[sys_num];
}

const SystemBase &
FEProblemBase::systemBaseLinear(const unsigned int sys_num) const
{
  mooseAssert(sys_num < _linear_systems.size(),
              "System number greater than the number of linear systems");
  return *_linear_systems[sys_num];
}

SystemBase &
FEProblemBase::systemBaseLinear(const unsigned int sys_num)
{
  mooseAssert(sys_num < _linear_systems.size(),
              "System number greater than the number of linear systems");
  return *_linear_systems[sys_num];
}

const SystemBase &
FEProblemBase::systemBaseAuxiliary() const
{
  return *_aux;
}

SystemBase &
FEProblemBase::systemBaseAuxiliary()
{
  return *_aux;
}

void
FEProblemBase::computingNonlinearResid(bool computing_nonlinear_residual)
{
  parallel_object_only();

  if (_displaced_problem)
    _displaced_problem->computingNonlinearResid(computing_nonlinear_residual);
  _computing_nonlinear_residual = computing_nonlinear_residual;
}

void
FEProblemBase::setCurrentlyComputingResidual(bool currently_computing_residual)
{
  if (_displaced_problem)
    _displaced_problem->setCurrentlyComputingResidual(currently_computing_residual);
  _currently_computing_residual = currently_computing_residual;
}

void
FEProblemBase::uniformRefine()
{
  // ResetDisplacedMeshThread::onNode looks up the reference mesh by ID, so we need to make sure
  // we undisplace before adapting the reference mesh
  if (_displaced_problem)
    _displaced_problem->undisplaceMesh();

  Adaptivity::uniformRefine(&_mesh, 1);
  if (_displaced_problem)
    Adaptivity::uniformRefine(&_displaced_problem->mesh(), 1);

  meshChangedHelper(/*intermediate_change=*/false);
}

void
FEProblemBase::automaticScaling(bool automatic_scaling)
{
  if (_displaced_problem)
    _displaced_problem->automaticScaling(automatic_scaling);

  SubProblem::automaticScaling(automatic_scaling);
}

void
FEProblemBase::reinitElemFaceRef(const Elem * elem,
                                 unsigned int side,
                                 Real tolerance,
                                 const std::vector<Point> * const pts,
                                 const std::vector<Real> * const weights,
                                 const THREAD_ID tid)
{
  SubProblem::reinitElemFaceRef(elem, side, tolerance, pts, weights, tid);

  if (_displaced_problem)
    _displaced_problem->reinitElemFaceRef(
        _displaced_mesh->elemPtr(elem->id()), side, tolerance, pts, weights, tid);
}

void
FEProblemBase::reinitNeighborFaceRef(const Elem * neighbor_elem,
                                     unsigned int neighbor_side,
                                     Real tolerance,
                                     const std::vector<Point> * const pts,
                                     const std::vector<Real> * const weights,
                                     const THREAD_ID tid)
{
  SubProblem::reinitNeighborFaceRef(neighbor_elem, neighbor_side, tolerance, pts, weights, tid);

  if (_displaced_problem)
    _displaced_problem->reinitNeighborFaceRef(
        _displaced_mesh->elemPtr(neighbor_elem->id()), neighbor_side, tolerance, pts, weights, tid);
}

void
FEProblemBase::getFVMatsAndDependencies(
    const SubdomainID blk_id,
    std::vector<std::shared_ptr<MaterialBase>> & face_materials,
    std::vector<std::shared_ptr<MaterialBase>> & neighbor_materials,
    std::set<MooseVariableFieldBase *> & variables,
    const THREAD_ID tid)
{
  if (_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
  {
    auto & this_face_mats =
        _materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid);
    for (std::shared_ptr<MaterialBase> face_mat : this_face_mats)
      if (face_mat->ghostable())
      {
        mooseAssert(!face_mat->hasStatefulProperties(),
                    "Finite volume materials do not currently support stateful properties.");
        face_materials.push_back(face_mat);
        auto & var_deps = face_mat->getMooseVariableDependencies();
        for (auto * var : var_deps)
        {
          mooseAssert(
              var->isFV(),
              "Ghostable materials should only have finite volume variables coupled into them.");
          variables.insert(var);
        }
      }
  }

  if (_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
  {
    auto & this_neighbor_mats =
        _materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid);
    for (std::shared_ptr<MaterialBase> neighbor_mat : this_neighbor_mats)
      if (neighbor_mat->ghostable())
      {
        mooseAssert(!neighbor_mat->hasStatefulProperties(),
                    "Finite volume materials do not currently support stateful properties.");
        neighbor_materials.push_back(neighbor_mat);
#ifndef NDEBUG
        auto & var_deps = neighbor_mat->getMooseVariableDependencies();
        for (auto * var : var_deps)
        {
          mooseAssert(
              var->isFV(),
              "Ghostable materials should only have finite volume variables coupled into them.");
          auto pr = variables.insert(var);
          mooseAssert(!pr.second,
                      "We should not have inserted any new variables dependencies from our "
                      "neighbor materials that didn't exist for our face materials");
        }
#endif
      }
  }
}

void
FEProblemBase::resizeMaterialData(const Moose::MaterialDataType data_type,
                                  const unsigned int nqp,
                                  const THREAD_ID tid)
{
  getMaterialData(data_type, tid).resize(nqp);
}

void
FEProblemBase::setNonlinearConvergenceNames(const std::vector<ConvergenceName> & convergence_names)
{
  if (convergence_names.size() != numNonlinearSystems())
    paramError("nonlinear_convergence",
               "There must be one convergence object per nonlinear system");
  _nonlinear_convergence_names = convergence_names;
  _set_nonlinear_convergence_names = true;
}

std::vector<ConvergenceName>
FEProblemBase::getNonlinearConvergenceNames() const
{
  if (_set_nonlinear_convergence_names)
    return _nonlinear_convergence_names;
  else
    mooseError("The nonlinear convergence name(s) have not been set.");
}

void
FEProblemBase::residualSetup()
{
  SubProblem::residualSetup();
  // We need to setup all the nonlinear systems other than our current one which actually called
  // this method (so we have to make sure we don't go in a circle)
  for (const auto i : make_range(numNonlinearSystems()))
    if (i != currentNlSysNum())
      _nl[i]->residualSetup();
  // We don't setup the aux sys because that's been done elsewhere
  if (_displaced_problem)
    _displaced_problem->residualSetup();
}

void
FEProblemBase::jacobianSetup()
{
  SubProblem::jacobianSetup();
  // We need to setup all the nonlinear systems other than our current one which actually called
  // this method (so we have to make sure we don't go in a circle)
  for (const auto i : make_range(numNonlinearSystems()))
    if (i != currentNlSysNum())
      _nl[i]->jacobianSetup();
  // We don't setup the aux sys because that's been done elsewhere
  if (_displaced_problem)
    _displaced_problem->jacobianSetup();
}

MooseAppCoordTransform &
FEProblemBase::coordTransform()
{
  return mesh().coordTransform();
}

unsigned int
FEProblemBase::currentNlSysNum() const
{
  // If we don't have nonlinear systems this should be an invalid number
  unsigned int current_nl_sys_num = libMesh::invalid_uint;
  if (_nl.size())
    current_nl_sys_num = currentNonlinearSystem().number();

  return current_nl_sys_num;
}

unsigned int
FEProblemBase::currentLinearSysNum() const
{
  // If we don't have linear systems this should be an invalid number
  unsigned int current_linear_sys_num = libMesh::invalid_uint;
  if (_linear_systems.size())
    current_linear_sys_num = currentLinearSystem().number();

  return current_linear_sys_num;
}

bool
FEProblemBase::shouldPrintExecution(const THREAD_ID tid) const
{
  // For now, only support printing from thread 0
  if (tid != 0)
    return false;

  if (_print_execution_on.isValueSet(_current_execute_on_flag) ||
      _print_execution_on.isValueSet(EXEC_ALWAYS))
    return true;
  else
    return false;
}

std::vector<MortarUserObject *>
FEProblemBase::getMortarUserObjects(const BoundaryID primary_boundary_id,
                                    const BoundaryID secondary_boundary_id,
                                    const bool displaced,
                                    const std::vector<MortarUserObject *> & mortar_uo_superset)
{
  std::vector<MortarUserObject *> mortar_uos;
  auto * const subproblem = displaced ? static_cast<SubProblem *>(_displaced_problem.get())
                                      : static_cast<SubProblem *>(this);
  for (auto * const obj : mortar_uo_superset)
    if (obj->onInterface(primary_boundary_id, secondary_boundary_id) &&
        (&obj->getSubProblem() == subproblem))
      mortar_uos.push_back(obj);

  return mortar_uos;
}

std::vector<MortarUserObject *>
FEProblemBase::getMortarUserObjects(const BoundaryID primary_boundary_id,
                                    const BoundaryID secondary_boundary_id,
                                    const bool displaced)
{
  std::vector<MortarUserObject *> mortar_uos;
  theWarehouse()
      .query()
      .condition<AttribInterfaces>(Interfaces::MortarUserObject)
      .queryInto(mortar_uos);
  return getMortarUserObjects(primary_boundary_id, secondary_boundary_id, displaced, mortar_uos);
}

void
FEProblemBase::reinitMortarUserObjects(const BoundaryID primary_boundary_id,
                                       const BoundaryID secondary_boundary_id,
                                       const bool displaced)
{
  const auto mortar_uos =
      getMortarUserObjects(primary_boundary_id, secondary_boundary_id, displaced);
  for (auto * const mortar_uo : mortar_uos)
  {
    mortar_uo->setNormals();
    mortar_uo->reinit();
  }
}

void
FEProblemBase::doingPRefinement(const bool doing_p_refinement,
                                const MultiMooseEnum & disable_p_refinement_for_families)
{
  SubProblem::doingPRefinement(doing_p_refinement, disable_p_refinement_for_families);
  if (_displaced_problem)
    _displaced_problem->doingPRefinement(doing_p_refinement, disable_p_refinement_for_families);
}

void
FEProblemBase::setVerboseProblem(bool verbose)
{
  _verbose_setup = verbose ? "true" : "false";
  _verbose_multiapps = verbose;
}

void
FEProblemBase::setCurrentLowerDElem(const Elem * const lower_d_elem, const THREAD_ID tid)
{
  SubProblem::setCurrentLowerDElem(lower_d_elem, tid);
  if (_displaced_problem)
    _displaced_problem->setCurrentLowerDElem(
        lower_d_elem ? _displaced_mesh->elemPtr(lower_d_elem->id()) : nullptr, tid);
}

void
FEProblemBase::setCurrentBoundaryID(BoundaryID bid, const THREAD_ID tid)
{
  SubProblem::setCurrentBoundaryID(bid, tid);
  if (_displaced_problem)
    _displaced_problem->setCurrentBoundaryID(bid, tid);
}

void
FEProblemBase::setCurrentNonlinearSystem(const unsigned int nl_sys_num)
{
  mooseAssert(nl_sys_num < _nl.size(),
              "System number greater than the number of nonlinear systems");
  _current_nl_sys = _nl[nl_sys_num].get();
  _current_solver_sys = _current_nl_sys;
}

void
FEProblemBase::setCurrentLinearSystem(const unsigned int sys_num)
{
  mooseAssert(sys_num < _linear_systems.size(),
              "System number greater than the number of linear systems");
  _current_linear_sys = _linear_systems[sys_num].get();
  _current_solver_sys = _current_linear_sys;
}

void
FEProblemBase::computeSystems(const ExecFlagType & type)
{
  // When performing an adjoint solve in the optimization module, the current solver system is the
  // adjoint. However, the adjoint solve requires having accurate time derivative calculations for
  // the forward system. The cleanest way to handle such uses is just to compute the time
  // derivatives for all solver systems instead of trying to guess which ones we need and don't need
  for (auto & solver_sys : _solver_systems)
    solver_sys->compute(type);

  _aux->compute(type);
}

const ConstElemRange &
FEProblemBase::getCurrentAlgebraicElementRange()
{
  if (!_current_algebraic_elem_range)
    return *_mesh.getActiveLocalElementRange();

  return *_current_algebraic_elem_range;
}
const ConstNodeRange &
FEProblemBase::getCurrentAlgebraicNodeRange()
{
  if (!_current_algebraic_node_range)
    return *_mesh.getLocalNodeRange();

  return *_current_algebraic_node_range;
}
const ConstBndNodeRange &
FEProblemBase::getCurrentAlgebraicBndNodeRange()
{
  if (!_current_algebraic_bnd_node_range)
    return *_mesh.getBoundaryNodeRange();

  return *_current_algebraic_bnd_node_range;
}

void
FEProblemBase::setCurrentAlgebraicElementRange(ConstElemRange * range)
{
  if (!range)
  {
    _current_algebraic_elem_range = nullptr;
    return;
  }

  _current_algebraic_elem_range = std::make_unique<ConstElemRange>(*range);
}
void
FEProblemBase::setCurrentAlgebraicNodeRange(ConstNodeRange * range)
{
  if (!range)
  {
    _current_algebraic_node_range = nullptr;
    return;
  }

  _current_algebraic_node_range = std::make_unique<ConstNodeRange>(*range);
}
void
FEProblemBase::setCurrentAlgebraicBndNodeRange(ConstBndNodeRange * range)
{
  if (!range)
  {
    _current_algebraic_bnd_node_range = nullptr;
    return;
  }

  _current_algebraic_bnd_node_range = std::make_unique<ConstBndNodeRange>(*range);
}

unsigned short
FEProblemBase::getCurrentICState()
{
  return _current_ic_state;
}
