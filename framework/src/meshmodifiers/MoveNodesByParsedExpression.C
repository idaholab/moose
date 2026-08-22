//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MoveNodesByParsedExpression.h"
#include "Function.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "AuxiliarySystem.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "Assembly.h"

#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/parallel_ghost_sync.h"
#include <libmesh/int_range.h>
#include <algorithm>
#include <set>

registerMooseObject("MooseApp", MoveNodesByParsedExpression);

const std::string MoveNodesByParsedExpression::_disp_name[3] = {
    "displacement_x", "displacement_y", "displacement_z"};

InputParameters
MoveNodesByParsedExpression::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addClassDescription(
      "Actively displaces the selected mesh nodes by parsed expressions for the x, y, and z "
      "displacement components, evaluated relative to each node's original position.");

  params.addParam<std::vector<BoundaryName>>(
      "boundary", {}, "List of boundaries whose nodes are displaced");
  params.addParam<std::vector<SubdomainName>>(
      "block",
      {},
      "List of blocks whose nodes are displaced. If neither 'block' nor 'boundary' is specified, "
      "all blocks in the mesh are used.");

  params.addParam<ParsedFunctionExpression>(
      _disp_name[0], "0", "Parsed expression for the displacement in the x direction");
  params.addParam<ParsedFunctionExpression>(
      _disp_name[1], "0", "Parsed expression for the displacement in the y direction");
  params.addParam<ParsedFunctionExpression>(
      _disp_name[2], "0", "Parsed expression for the displacement in the z direction");

  params.addParam<std::vector<VariableName>>(
      "coupled_variables", {}, "Nodal variables usable as symbols in the displacement expressions");
  params.addParam<std::vector<FunctionName>>(
      "functions", {}, "Functions usable as symbols in the displacement expressions");
  params.addParam<std::vector<PostprocessorName>>(
      "postprocessors", {}, "Postprocessors usable as symbols in the displacement expressions");

  params.addParam<std::vector<std::string>>(
      "constant_names", {}, "Vector of constants used in the parsed function");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");

  // Optional outputs. Each is enabled only when its parameter is provided. The
  // target aux variables must be created by the user in the input.
  params.addParam<std::vector<AuxVariableName>>(
      "original_coordinate_variables",
      {},
      "If set, the original (undisplaced) node coordinates are written to these three nodal "
      "auxiliary variables, given in x, y, z order, which must be created by the user.");
  params.addParam<std::vector<AuxVariableName>>(
      "displacement_variables",
      {},
      "If set, the current node displacement (current minus original position) is written to these "
      "three nodal auxiliary variables, given in x, y, z order, which must be created by the "
      "user.");
  params.addParam<AuxVariableName>(
      "density_factor_variable",
      "",
      "If set, the per-element density adjustment factor (original volume divided by current "
      "volume) is written to this elemental aux variable, which must be created by the user.");

  params.addParam<bool>(
      "notify_mesh_changed",
      false,
      "Whether to notify the problem that the mesh has changed (by calling meshChanged) after the "
      "nodes are moved, so that mesh-dependent caches, the displaced mesh, geometric searches, and "
      "outputs are updated.");

  // By default don't execute; the user selects a schedule via execute_on.
  params.set<ExecFlagEnum>("execute_on") = "NONE";

  return params;
}

MoveNodesByParsedExpression::MoveNodesByParsedExpression(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    FunctionParserUtils<false>(parameters),
    _mesh(_subproblem.mesh()),
    _boundary_ids(_mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("boundary"))),
    _subdomain_ids(_mesh.getSubdomainIDs(getParam<std::vector<SubdomainName>>("block"))),
    _original_position(
        declareRestartableData<std::unordered_map<dof_id_type, Point>>("original_position")),
    _output_coordinates(
        !getParam<std::vector<AuxVariableName>>("original_coordinate_variables").empty()),
    _output_displacements(
        !getParam<std::vector<AuxVariableName>>("displacement_variables").empty()),
    _output_density_factor(!getParam<AuxVariableName>("density_factor_variable").empty()),
    _aux_sys_num(0),
    _density_factor_var(0),
    _assembly(nullptr),
    _original_volume(
        declareRestartableData<std::unordered_map<dof_id_type, Real>>("original_volume")),
    _original_volume_recorded(declareRestartableData<bool>("original_volume_recorded", false)),
    _notify_mesh_changed(getParam<bool>("notify_mesh_changed"))
{
  const auto & var_names = getParam<std::vector<VariableName>>("coupled_variables");
  const auto & func_names = getParam<std::vector<FunctionName>>("functions");
  const auto & pp_names = getParam<std::vector<PostprocessorName>>("postprocessors");

  // Build the comma-separated symbol list in evaluation order:
  // coupled variables, functions, postprocessors, then x, y, z, t.
  std::string symbols;
  auto add_symbol = [&symbols](const std::string & s)
  { symbols += (symbols.empty() ? "" : ",") + s; };

  for (const auto & name : var_names)
  {
    const MooseVariable & var = _subproblem.getStandardVariable(_tid, name);
    if (!var.isNodal())
      paramError("coupled_variables",
                 "Variable '",
                 name,
                 "' is not nodal. Only nodal variables can be used in the displacement "
                 "expressions.");
    _coupled_vars.push_back(&var);
    add_symbol(name);
  }
  for (const auto & name : func_names)
  {
    _functions.push_back(&getFunctionByName(name));
    add_symbol(name);
  }
  for (const auto & name : pp_names)
  {
    _postprocessors.push_back(&getPostprocessorValueByName(name));
    add_symbol(name);
  }

  // x, y, z, t are always available; guard against name collisions.
  for (const auto & reserved : {"x", "y", "z", "t"})
  {
    if (std::find(var_names.begin(), var_names.end(), reserved) != var_names.end() ||
        std::find(func_names.begin(), func_names.end(), reserved) != func_names.end() ||
        std::find(pp_names.begin(), pp_names.end(), reserved) != pp_names.end())
      mooseError("The symbol '",
                 reserved,
                 "' is reserved for coordinates/time and cannot be used as a coupled variable, "
                 "function, or postprocessor name.");
    add_symbol(reserved);
  }

  const auto & constant_names = getParam<std::vector<std::string>>("constant_names");
  const auto & constant_expressions = getParam<std::vector<std::string>>("constant_expressions");
  for (const auto i : make_range(3))
  {
    _displacement[i] = std::make_shared<SymFunction>();
    parsedFunctionSetup(_displacement[i],
                        getParam<ParsedFunctionExpression>(_disp_name[i]),
                        symbols,
                        constant_names,
                        constant_expressions,
                        comm());
  }

  _func_params.resize(_coupled_vars.size() + _functions.size() + _postprocessors.size() + 4);

  // Set up the optional output aux variables (created by the user in the input).
  if (_output_coordinates || _output_displacements || _output_density_factor)
    _aux_sys_num = _fe_problem.getAuxiliarySystem().number();

  if (_output_coordinates)
    setupNodalOutputVariables(
        "original_coordinate_variables",
        getParam<std::vector<AuxVariableName>>("original_coordinate_variables"),
        _coordinate_var);

  if (_output_displacements)
    setupNodalOutputVariables("displacement_variables",
                              getParam<std::vector<AuxVariableName>>("displacement_variables"),
                              _displacement_var);

  if (_output_density_factor)
  {
    const auto name = getParam<AuxVariableName>("density_factor_variable");
    if (!_subproblem.hasVariable(name))
      paramError("density_factor_variable",
                 "No auxiliary variable named '",
                 name,
                 "' was found. Create an elemental (family = MONOMIAL, order = CONSTANT) auxiliary "
                 "variable with that name.");
    const MooseVariable & var = _subproblem.getStandardVariable(_tid, name);
    if (var.isNodal())
      paramError("density_factor_variable",
                 "Aux variable '",
                 name,
                 "' must be an elemental variable (e.g. family = MONOMIAL, order = CONSTANT).");
    _density_factor_var = var.number();
    _assembly = &_fe_problem.assembly(_tid, 0);
  }
}

void
MoveNodesByParsedExpression::setupNodalOutputVariables(const std::string & param_name,
                                                       const std::vector<AuxVariableName> & names,
                                                       std::vector<unsigned int> & var_numbers)
{
  if (names.size() != 3)
    paramError(param_name,
               "Exactly three variable names must be provided, for the x, y, and z components.");

  for (const auto & name : names)
  {
    if (!_subproblem.hasVariable(name))
      paramError(param_name,
                 "No auxiliary variable named '",
                 name,
                 "' was found. Create a nodal (e.g. LAGRANGE) auxiliary variable with that name.");
    const MooseVariable & var = _subproblem.getStandardVariable(_tid, name);
    if (!var.isNodal())
      paramError(param_name, "Auxiliary variable '", name, "' must be a nodal variable.");
    var_numbers.push_back(var.number());
  }
}

void
MoveNodesByParsedExpression::execute()
{
  moveNodes();

  // Optionally notify the problem that the mesh changed so dependent systems update.
  // This object does not respond to meshChanged() (it modifies the mesh actively), so
  // the resulting broadcast does not call back into it and no guard is needed.
  if (_notify_mesh_changed)
    _fe_problem.meshChanged(
        /*intermediate_change=*/false, /*contract_mesh=*/false, /*clean_refinement_flags=*/false);
}

void
MoveNodesByParsedExpression::prepare()
{
  // Gather each coupled-variable system's solution onto all ranks so that nodal
  // values can be read at any moved node (including non-semilocal nodes on a
  // replicated mesh) during parallel execution. Refreshed every pass.
  std::set<unsigned int> localized;
  for (const auto * const var : _coupled_vars)
  {
    const auto sys_num = var->sys().number();
    if (!localized.insert(sys_num).second)
      continue;
    const SystemBase & sys = var->sys();
    const auto n_dofs = sys.system().n_dofs();
    auto & vec = _localized_solution[sys_num];
    if (!vec)
      vec = libMesh::NumericVector<libMesh::Number>::build(comm());
    // size() asserts on an uninitialized vector, so check initialized() first: the
    // vector is uninitialized on the first pass, and n_dofs can change under adaptivity.
    if (!vec->initialized() || vec->size() != n_dofs)
      vec->init(n_dofs, false, libMesh::SERIAL);
    sys.currentSolution()->localize(*vec);
  }

  // Record the original (undisplaced) coordinate-aware element volumes once, on the
  // first execution while the mesh is still in its reference state. These are
  // compared against the post-move volumes to form the density adjustment factor.
  if (_output_density_factor && !_original_volume_recorded)
  {
    for (const auto * const elem : _mesh.getMesh().active_local_element_ptr_range())
      _original_volume[elem->id()] = _assembly->elementVolume(elem);
    _original_volume_recorded = true;
  }
}

void
MoveNodesByParsedExpression::moveNodes()
{
  prepare();

  auto & mesh = _mesh.getMesh();

  // Displace nodes on the requested boundaries.
  for (const auto & boundary_id : _boundary_ids)
    for (const auto & node_id : _mesh.getNodeList(boundary_id))
      if (Node * const node = mesh.query_node_ptr(node_id))
        displaceNode(*node);

  // Displace nodes by block. When the user specifies neither 'block' nor 'boundary',
  // operate on all blocks. Iterate the selected subdomains' elements (including
  // ghosted elements) and displace their nodes; this is safe on a DistributedMesh
  // and moves ghosted node copies consistently on every rank that holds them.
  const bool all_blocks = !isParamSetByUser("block") && !isParamSetByUser("boundary");
  if (all_blocks || !_subdomain_ids.empty())
  {
    const std::set<SubdomainID> subdomains(_subdomain_ids.begin(), _subdomain_ids.end());
    std::set<dof_id_type> displaced_nodes;
    for (auto * const elem : mesh.active_element_ptr_range())
    {
      if (!all_blocks && !subdomains.count(elem->subdomain_id()))
        continue;
      for (auto & node : elem->node_ref_range())
        if (displaced_nodes.insert(node.id()).second)
          displaceNode(node);
    }
  }

  // Synchronize node positions across ranks. On a DistributedMesh a node shared
  // between partitions may be moved on the rank that holds the block element (or
  // boundary) containing it, yet remain unmoved on another rank that only holds the
  // node through a different, non-selected element. SyncNodalPositions copies each
  // node's position from its owning rank to every other rank holding a copy. Skipped
  // on a ReplicatedMesh, where every rank has already moved every node identically.
  if (!mesh.is_replicated())
  {
    libMesh::SyncNodalPositions sync_positions(mesh);
    libMesh::Parallel::sync_dofobject_data_by_id(
        mesh.comm(), mesh.nodes_begin(), mesh.nodes_end(), sync_positions);
  }

  writeOutputs();
}

void
MoveNodesByParsedExpression::displaceNode(Node & node)
{
  // Capture the original position on first touch (covers nodes created by adaptivity).
  auto [it, inserted] = _original_position.try_emplace(node.id(), Point(node));
  const Point & ref = it->second;

  std::size_t k = 0;
  for (const auto * const var : _coupled_vars)
  {
    const auto sys_num = var->sys().number();
    // A moved node may carry no DOF for this variable (e.g. the variable is
    // defined on a different block than the one being moved).
    if (node.n_dofs(sys_num, var->number()) == 0)
      mooseError("Coupled variable '",
                 var->name(),
                 "' has no degrees of freedom at node ",
                 node.id(),
                 ". All 'coupled_variables' must be defined on the blocks/boundaries being moved.");
    const auto dof = node.dof_number(sys_num, var->number(), 0);
    _func_params[k++] = (*_localized_solution.at(sys_num))(dof);
  }
  for (const auto * const func : _functions)
    _func_params[k++] = func->value(_t, ref);
  for (const auto * const pp : _postprocessors)
    _func_params[k++] = *pp;
  _func_params[k++] = ref(0);
  _func_params[k++] = ref(1);
  _func_params[k++] = ref(2);
  _func_params[k++] = _t;

  for (const auto i : make_range(3))
    node(i) = ref(i) + evaluate(_displacement[i], _disp_name[i]);
}

void
MoveNodesByParsedExpression::writeOutputs()
{
  if (!_output_coordinates && !_output_displacements && !_output_density_factor)
    return;

  auto & mesh = _mesh.getMesh();
  auto & aux_solution = _fe_problem.getAuxiliarySystem().solution();

  // Nodal outputs: original coordinates and/or displacement. The _original_position
  // map holds every node that was moved; write the value for the nodes this rank
  // owns (the dof of a nodal variable belongs to the node's owner).
  if (_output_coordinates || _output_displacements)
    for (const auto & [node_id, original] : _original_position)
    {
      const Node * const node = mesh.query_node_ptr(node_id);
      if (!node || node->processor_id() != processor_id())
        continue;
      for (const auto i : make_range(Moose::dim))
      {
        if (_output_coordinates)
          aux_solution.set(node->dof_number(_aux_sys_num, _coordinate_var[i], 0), original(i));
        if (_output_displacements)
          aux_solution.set(node->dof_number(_aux_sys_num, _displacement_var[i], 0),
                           (*node)(i)-original(i));
      }
    }

  // Elemental output: density adjustment factor = original volume / current volume.
  if (_output_density_factor)
    for (auto * const elem : mesh.active_local_element_ptr_range())
    {
      const Real v_new = _assembly->elementVolume(elem);
      const Real factor = (v_new != 0.0) ? _original_volume[elem->id()] / v_new : 1.0;
      aux_solution.set(elem->dof_number(_aux_sys_num, _density_factor_var, 0), factor);
    }

  aux_solution.close();
}
