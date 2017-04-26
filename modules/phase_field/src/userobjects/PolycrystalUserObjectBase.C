/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
  params.addClassDescription("TODO");

  params.addRequiredCoupledVarWithAutoBuild(
      "variable", "var_name_base", "op_num", "Array of coupled variables");

  params.addRequiredParam<unsigned int>(
      "grain_num", "Number of grains being represented by the order parameters");
  params.addParam<MooseEnum>("coloring_algorithm",
                             PolycrystalUserObjectBase::coloringAlgorithms(),
                             PolycrystalUserObjectBase::coloringAlgorithmDescriptions());
  return params;
}

PolycrystalUserObjectBase::PolycrystalUserObjectBase(const InputParameters & parameters)
  : FeatureFloodCount(parameters),
    _dim(_mesh.dimension()),
    _op_num(_vars.size()),
    _grain_num(getParam<unsigned int>("grain_num")),
    _grain_to_op(_grain_num, PolycrystalUserObjectBase::INVALID_COLOR),
    _coloring_algorithm(getParam<MooseEnum>("coloring_algorithm")),
    _initialized(false)
{
  if (_op_num > _grain_num)
    mooseError("ERROR in PolycrystalVoronoi: Number of order parameters (op_num) can't be larger "
               "than the number of grains (grain_num)");
}

void
PolycrystalUserObjectBase::initialSetup()
{
  // For now, only support Replicated mesh
  _mesh.errorIfDistributedMesh("Graph Coloring");

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
PolycrystalUserObjectBase::execute()
{
  precomputeGrainStructure();

  FeatureFloodCount::execute();
}

void
PolycrystalUserObjectBase::finalize()
{
  // TODO: Possibly retrieve the halo thickness from the active GrainTracker object?
  constexpr unsigned int halo_thickness = 2;
  expandPointHalos();
  expandEdgeHalos(halo_thickness - 1);

  communicateAndMerge();

  if (_is_master)
  {
    buildGrainAdjacencyMatrix();

    assignOpsToGrains();

    // DEBUG
    std::cout << "Grain Adjacency Matrix:\n";
    for (unsigned int i = 0; i < _adjacency_matrix->m(); i++)
    {
      for (unsigned int j = 0; j < _adjacency_matrix->n(); j++)
        std::cout << _adjacency_matrix->el(i, j) << "  ";
      std::cout << '\n';
    }
  }

  // Communicate the coloring with all ranks
  _communicator.broadcast(_grain_to_op);

  // DEBUG
  std::cout << "Grain to OP assignments:\n";
  for (auto op : _grain_to_op)
    std::cout << op << "  ";
  std::cout << std::endl;
}

bool
PolycrystalUserObjectBase::isNewFeatureOrConnectedRegion(const DofObject * dof_object,
                                                         std::size_t current_index,
                                                         FeatureData *& feature,
                                                         Status & status,
                                                         unsigned int & new_id)
{
  mooseAssert(_t_step == 0, "PolyIC only works if we begin in the initial condition");
  // mooseAssert(_is_elemental, "PolyIC only works with elemental grain tracker");

  /**
   * When generating ICs we aren't looking at the op index at all. We need to return
   * false on all non-zero ops so we don't count each grain multiple times.
   */
  if (current_index != 0)
    return false;

  unsigned int grain_id;
  if (_is_elemental)
    grain_id = getGrainBasedOnElem(*static_cast<const Elem *>(dof_object));
  else
    grain_id = getGrainBasedOnPoint(*static_cast<const Node *>(dof_object));

  if (!feature)
  {
    new_id = grain_id;
    status &= ~Status::INACTIVE;

    return true;
  }
  else
    return feature->_id == grain_id;
}

bool
PolycrystalUserObjectBase::areFeaturesMergeable(const FeatureData & f1,
                                                const FeatureData & f2) const
{
  return f1._id == f2._id;
}

void
PolycrystalUserObjectBase::buildGrainAdjacencyMatrix()
{
  _grain_to_op.resize(_feature_count);

  if (_is_master)
  {
    std::cout << "Feature Count: " << _feature_count << '\n';
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
}

void
PolycrystalUserObjectBase::assignOpsToGrains()
{
  // Moose::perf_log.push("assignOpsToGrains()", "PolycrystalICTools");
  //
  // Use a simple backtracking coloring algorithm
  if (_coloring_algorithm == "bt")
  {
    if (!colorGraph(0))
      mooseError("Unable to find a valid grain to op coloring, do you have enough op variables?");
  }
  else // PETSc Coloring algorithms
  {
#ifdef LIBMESH_HAVE_PETSC
    const std::string & ca_str = _coloring_algorithm;
    Real * am_data = _adjacency_matrix->get_values().data();
    Moose::PetscSupport::colorAdjacencyMatrix(
        am_data, _grain_num, _vars.size(), _grain_to_op, ca_str.c_str());
#else
    mooseError("Selected coloring algorithm requires PETSc");
#endif
  }

  //  Moose::perf_log.pop("assignOpsToGrains()", "PolycrystalICTools");
}

bool
PolycrystalUserObjectBase::colorGraph(unsigned int vertex)
{
  // Base case: All grains are assigned
  if (vertex == _grain_num)
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
  for (unsigned int neighbor = 0; neighbor < _grain_num; ++neighbor)
    if ((*_adjacency_matrix)(vertex, neighbor) && color == _grain_to_op[neighbor])
      return false;
  return true;
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
