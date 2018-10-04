//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalUserObjectBase.h"
#include "NonlinearSystemBase.h"
#include "MooseMesh.h"
#include "MooseVariable.h"

#include "libmesh/dense_matrix.h"

#include <vector>
#include <map>
#include <algorithm>

template <>
InputParameters
validParams<PolycrystalUserObjectBase>()
{
  InputParameters params = validParams<FeatureFloodCount>();
  params.addClassDescription("This object provides the base capability for creating proper reduced "
                             "order parameter polycrystal initial conditions.");
  params.addRequiredCoupledVarWithAutoBuild(
      "variable", "var_name_base", "op_num", "Array of coupled variables");
  params.addParam<bool>("output_adjacency_matrix",
                        false,
                        "Output the Grain Adjacency Matrix used in the coloring algorithms. "
                        "Additionally, the grain to OP assignments will be printed");
  params.addParam<MooseEnum>("coloring_algorithm",
                             PolycrystalUserObjectBase::coloringAlgorithms(),
                             PolycrystalUserObjectBase::coloringAlgorithmDescriptions());

  params.registerRelationshipManagers("GrainTrackerHaloRM ElementPointNeighbors",
                                      "GEOMETRIC ALGEBRAIC");
  params.addPrivateParam<unsigned short>("element_point_neighbor_layers", 1);

  // Hide the output of the IC objects by default, it doesn't change over time
  params.set<std::vector<OutputName>>("outputs") = {"none"};

  /// Run this user object more than once on the initial condition to handle initial adaptivity
  params.set<bool>("allow_duplicate_execution_on_initial") = true;

  // This object should only be executed _before_ the initial condition
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = EXEC_INITIAL;
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

PolycrystalUserObjectBase::PolycrystalUserObjectBase(const InputParameters & parameters)
  : FeatureFloodCount(parameters),
    _dim(_mesh.dimension()),
    _op_num(_vars.size()),
    _coloring_algorithm(getParam<MooseEnum>("coloring_algorithm")),
    _colors_assigned(false),
    _output_adjacency_matrix(getParam<bool>("output_adjacency_matrix"))
{
  mooseAssert(_single_map_mode, "Do not turn off single_map_mode with this class");
}

void
PolycrystalUserObjectBase::initialSetup()
{
  /**
   * For polycrystal ICs we need to assume that each of the variables has the same periodicity.
   * Since BCs are handled elsewhere in the system, we'll have to check this case explicitly.
   */
  if (_op_num < 1)
    mooseError("No coupled variables found");

  for (unsigned int dim = 0; dim < _dim; ++dim)
  {
    bool first_variable_value = _mesh.isTranslatedPeriodic(_vars[0]->number(), dim);

    for (unsigned int i = 1; i < _vars.size(); ++i)
      if (_mesh.isTranslatedPeriodic(_vars[i]->number(), dim) != first_variable_value)
        mooseError("Coupled polycrystal variables differ in periodicity");
  }

  FeatureFloodCount::initialSetup();
}

void
PolycrystalUserObjectBase::initialize()
{
  if (_colors_assigned && !_fe_problem.hasInitialAdaptivity())
    return;

  _entity_to_grain_cache.clear();

  FeatureFloodCount::initialize();
}

void
PolycrystalUserObjectBase::execute()
{
  if (!_colors_assigned)
    precomputeGrainStructure();
  // No need to rerun the object if the mesh hasn't changed
  else if (!_fe_problem.hasInitialAdaptivity())
    return;

  /**
   * We need one map per grain when creating the initial condition to support overlapping features.
   * Luckily, this is a fairly sparse structure.
   */
  _entities_visited.resize(getNumGrains());

  /**
   * This loop is similar to the one found in the base class however, there are two key differences
   * between building up the initial condition and discovering features based on solution variables:
   *
   * 1) When building up the initial condition, we aren't inspecting the actual variable values so
   *    we don't need to loop over all of the coupled variables.
   * 2) We want to discover all features on a single pass since there may be thousands of features
   *    in a simulation. However, we can only actively flood a single feature at a time. To make
   *    sure that we pick up all features that might start on a given entity, we'll keep retrying
   *    the flood routine on the same entity as long as new discoveries are being made. We know
   *    this information from the return value of flood.
   */
  for (const auto & current_elem : _fe_problem.getEvaluableElementRange())
  {
    // Loop over elements or nodes
    if (_is_elemental)
      while (flood(current_elem, invalid_size_t))
        ;
    else
    {
      auto n_nodes = current_elem->n_vertices();
      for (auto i = decltype(n_nodes)(0); i < n_nodes; ++i)
      {
        const Node * current_node = current_elem->get_node(i);

        while (flood(current_node, invalid_size_t))
          ;
      }
    }
  }
}

void
PolycrystalUserObjectBase::finalize()
{
  if (_colors_assigned && !_fe_problem.hasInitialAdaptivity())
    return;

  // TODO: Possibly retrieve the halo thickness from the active GrainTracker object?
  constexpr unsigned int halo_thickness = 2;

  expandEdgeHalos(halo_thickness - 1);

  FeatureFloodCount::finalize();

  if (!_colors_assigned)
  {
    // Resize the color assignment vector here. All ranks need a copy of this
    _grain_to_op.resize(_feature_count, PolycrystalUserObjectBase::INVALID_COLOR);
    if (_is_master)
    {
      buildGrainAdjacencyMatrix();

      assignOpsToGrains();

      if (_output_adjacency_matrix)
        printGrainAdjacencyMatrix();
    }

    // Communicate the coloring with all ranks
    _communicator.broadcast(_grain_to_op);

    /**
     * All ranks: Update the variable indices based on the graph coloring algorithm. Here we index
     * into the _grain_to_op vector based on the grain_id to obtain the right assignment.
     */
    for (auto & grain : _feature_sets)
      grain._var_index = _grain_to_op[grain._id];
  }

  _colors_assigned = true;
}

void
PolycrystalUserObjectBase::mergeSets()
{
  /**
   * With initial conditions we know the grain IDs of every grain (even partial grains). We can use
   * this information to put all mergeable features adjacent to one and other in the list so that
   * merging is simply O(n).
   */
  _partial_feature_sets[0].sort();

  auto it1 = _partial_feature_sets[0].begin();
  auto it_end = _partial_feature_sets[0].end();
  while (it1 != it_end)
  {
    auto it2 = it1;
    if (++it2 == it_end)
      break;

    if (areFeaturesMergeable(*it1, *it2))
    {
      it1->merge(std::move(*it2));
      _partial_feature_sets[0].erase(it2);
    }
    else
      ++it1; // Only increment if we have a mismatch
  }
}

bool
PolycrystalUserObjectBase::isNewFeatureOrConnectedRegion(const DofObject * dof_object,
                                                         std::size_t & current_index,
                                                         FeatureData *& feature,
                                                         Status & status,
                                                         unsigned int & new_id)
{
  mooseAssert(_t_step == 0, "PolyIC only works if we begin in the initial condition");

  // Retrieve the id of the current entity
  auto entity_id = dof_object->id();
  auto grains_it = _entity_to_grain_cache.lower_bound(entity_id);

  if (grains_it == _entity_to_grain_cache.end() || grains_it->first != entity_id)
  {
    std::vector<unsigned int> grain_ids;

    if (_is_elemental)
      getGrainsBasedOnElem(*static_cast<const Elem *>(dof_object), grain_ids);
    else
      getGrainsBasedOnPoint(*static_cast<const Node *>(dof_object), grain_ids);

    grains_it = _entity_to_grain_cache.emplace_hint(grains_it, entity_id, std::move(grain_ids));
  }

  /**
   * When building the IC, we can't use the _entities_visited data structure the same way as we do
   * for the base class. We need to discover multiple overlapping grains in a single pass. However
   * we don't know what grain we are working on when we enter the flood routine (when that check is
   * normally made). Only after we've made the callback to the child class do we know which grains
   * we are operating on (at least until we've triggered the recursion). We need to see if there
   * is at least one active grain where we haven't already visited the current entity before
   * continuing.
   */
  auto saved_grain_id = invalid_id;
  if (current_index == invalid_size_t)
  {
    for (auto grain_id : grains_it->second)
    {
      mooseAssert(!_colors_assigned || grain_id < _grain_to_op.size(), "grain_id out of range");
      auto map_num = _colors_assigned ? _grain_to_op[grain_id] : grain_id;
      if (_entities_visited[map_num].find(entity_id) == _entities_visited[map_num].end())
      {
        saved_grain_id = grain_id;

        if (!_colors_assigned)
          current_index = grain_id;
        else
          current_index = _grain_to_op[grain_id];

        break;
      }
    }

    if (current_index == invalid_size_t)
      return false;
  }
  else if (_entities_visited[current_index].find(entity_id) !=
           _entities_visited[current_index].end())
    return false;

  if (!feature)
  {
    new_id = saved_grain_id;
    status &= ~Status::INACTIVE;

    return true;
  }
  else
  {
    const auto & grain_ids = grains_it->second;
    if (std::find(grain_ids.begin(), grain_ids.end(), feature->_id) != grain_ids.end())
      return true;

    /**
     * If we get here the current entity is not part of the active feature, however we now want to
     * look at neighbors.
     *
     */
    if (_is_elemental)
    {
      Elem * elem = _mesh.queryElemPtr(entity_id);
      mooseAssert(elem, "Element is nullptr");

      std::vector<const Elem *> all_active_neighbors;
      MeshBase & mesh = _mesh.getMesh();

      for (auto i = decltype(elem->n_neighbors())(0); i < elem->n_neighbors(); ++i)
      {
        const Elem * neighbor_ancestor = nullptr;

        /**
         * Retrieve only the active neighbors for each side of this element, append them to the list
         * of active neighbors
         */
        neighbor_ancestor = elem->neighbor(i);
        if (neighbor_ancestor)
          neighbor_ancestor->active_family_tree_by_neighbor(all_active_neighbors, elem, false);
        else // if (expand_halos_only /*&& feature->_periodic_nodes.empty()*/)
        {
          neighbor_ancestor = elem->topological_neighbor(i, mesh, *_point_locator, _pbs);

          /**
           * If the current element (passed into this method) doesn't have a connected neighbor but
           * does have a topological neighbor, this might be a new disjoint region that we'll
           * need to represent with a separate bounding box. To find out for sure, we'll need
           * see if the new neighbors are present in any of the halo or disjoint halo sets. If
           * they are not present, this is a new region.
           */
          if (neighbor_ancestor)
            neighbor_ancestor->active_family_tree_by_topological_neighbor(
                all_active_neighbors, elem, mesh, *_point_locator, _pbs, false);
        }
      }

      for (const auto neighbor : all_active_neighbors)
      {
        // Retrieve the id of the current entity
        auto neighbor_id = neighbor->id();
        auto neighbor_it = _entity_to_grain_cache.lower_bound(neighbor_id);

        if (neighbor_it == _entity_to_grain_cache.end() || neighbor_it->first != neighbor_id)
        {
          std::vector<unsigned int> more_grain_ids;

          getGrainsBasedOnElem(*static_cast<const Elem *>(neighbor), more_grain_ids);

          neighbor_it = _entity_to_grain_cache.emplace_hint(
              neighbor_it, neighbor_id, std::move(more_grain_ids));
        }

        const auto & more_grain_ids = neighbor_it->second;
        if (std::find(more_grain_ids.begin(), more_grain_ids.end(), feature->_id) !=
            more_grain_ids.end())
          return true;
      }
    }

    return false;
  }
}

bool
PolycrystalUserObjectBase::areFeaturesMergeable(const FeatureData & f1,
                                                const FeatureData & f2) const
{
  return _colors_assigned ? f1.mergeable(f2) : f1._id == f2._id;
}

void
PolycrystalUserObjectBase::buildGrainAdjacencyMatrix()
{
  mooseAssert(_is_master, "This routine should only be called on the master rank");

  _adjacency_matrix = libmesh_make_unique<DenseMatrix<Real>>(_feature_count, _feature_count);
  for (auto & grain1 : _feature_sets)
  {
    for (auto & grain2 : _feature_sets)
    {
      if (&grain1 == &grain2)
        continue;

      if (grain1.boundingBoxesIntersect(grain2) && grain1.halosIntersect(grain2))
      {
        (*_adjacency_matrix)(grain1._id, grain2._id) = 1.;
        (*_adjacency_matrix)(grain1._id, grain2._id) = 1.;
      }
    }
  }
}

void
PolycrystalUserObjectBase::assignOpsToGrains()
{
  mooseAssert(_is_master, "This routine should only be called on the master rank");

  Moose::perf_log.push("assignOpsToGrains()", "PolycrystalICTools");

  // Use a simple backtracking coloring algorithm
  if (_coloring_algorithm == "bt")
  {
    paramInfo("coloring_algorithm",
              "The backtracking algorithm has exponential complexity. If you are using very few "
              "order parameters,\nor you have several hundred grains or more, you should use one "
              "of the PETSc coloring algorithms such as \"jp\".");

    if (!colorGraph(0))
      paramError("op_num",
                 "Unable to find a valid grain to op coloring, Make sure you have created enough "
                 "variables to hold a\nvalid polycrystal initial condition (no grains represented "
                 "by the same variable should be allowed to\ntouch, ~8 for 2D, ~25 for 3D)?");
  }
  else // PETSc Coloring algorithms
  {
#ifdef LIBMESH_HAVE_PETSC
    const std::string & ca_str = _coloring_algorithm;
    Real * am_data = _adjacency_matrix->get_values().data();

    try
    {
      Moose::PetscSupport::colorAdjacencyMatrix(
          am_data, _feature_count, _vars.size(), _grain_to_op, ca_str.c_str());
    }
    catch (std::runtime_error & e)
    {
      paramError("op_num",
                 "Unable to find a valid grain to op coloring, Make sure you have created enough "
                 "variables to hold a\nvalid polycrystal initial condition (no grains represented "
                 "by the same variable should be allowed to\ntouch, ~8 for 2D, ~25 for 3D)?");
    }
#else
    mooseError("Selected coloring algorithm requires PETSc");
#endif
  }

  Moose::perf_log.pop("assignOpsToGrains()", "PolycrystalICTools");
}

bool
PolycrystalUserObjectBase::colorGraph(unsigned int vertex)
{
  // Base case: All grains are assigned
  if (vertex == _feature_count)
    return true;

  // Consider this grain and try different ops
  for (unsigned int color_idx = 0; color_idx < _op_num; ++color_idx)
  {
    // We'll try to spread these colors around a bit rather than
    // packing them all on the first few colors if we have several colors.
    unsigned int color = (vertex + color_idx) % _op_num;

    if (isGraphValid(vertex, color))
    {
      _grain_to_op[vertex] = color;

      if (colorGraph(vertex + 1))
        return true;

      // Backtrack...
      _grain_to_op[vertex] = PolycrystalUserObjectBase::INVALID_COLOR;
    }
  }

  return false;
}

bool
PolycrystalUserObjectBase::isGraphValid(unsigned int vertex, unsigned int color)
{
  // See if the proposed color is valid based on the current neighbor colors
  for (unsigned int neighbor = 0; neighbor < _feature_count; ++neighbor)
    if ((*_adjacency_matrix)(vertex, neighbor) && color == _grain_to_op[neighbor])
      return false;
  return true;
}

void
PolycrystalUserObjectBase::printGrainAdjacencyMatrix() const
{
  _console << "Grain Adjacency Matrix:\n";
  for (unsigned int i = 0; i < _adjacency_matrix->m(); i++)
  {
    for (unsigned int j = 0; j < _adjacency_matrix->n(); j++)
      _console << _adjacency_matrix->el(i, j) << "  ";
    _console << '\n';
  }

  _console << "Grain to OP assignments:\n";
  for (auto op : _grain_to_op)
    _console << op << "  ";
  _console << '\n' << std::endl;
}

MooseEnum
PolycrystalUserObjectBase::coloringAlgorithms()
{
  return MooseEnum("jp power greedy bt", "jp");
}

std::string
PolycrystalUserObjectBase::coloringAlgorithmDescriptions()
{
  return "The grain neighbor graph coloring algorithm to use: \"jp\" (DEFAULT) Jones and "
         "Plassmann, an efficient coloring algorithm, \"power\" an alternative stochastic "
         "algorithm, \"greedy\", a greedy assignment algorithm with stochastic updates to "
         "guarantee a valid coloring, \"bt\", a back tracking algorithm that produces good "
         "distributions but may experience exponential run time in the worst case scenario "
         "(works well on medium to large 2D problems)";
}

const unsigned int PolycrystalUserObjectBase::INVALID_COLOR =
    std::numeric_limits<unsigned int>::max();
const unsigned int PolycrystalUserObjectBase::HALO_THICKNESS = 4;
