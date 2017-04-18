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
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("TODO");
  params.addRequiredParam<unsigned int>("op_num", "Number of order parameters");
  params.addRequiredParam<unsigned int>(
      "grain_num", "Number of grains being represented by the order parameters");
  params.addParam<MooseEnum>("coloring_algorithm",
                             PolycrystalUserObjectBase::coloringAlgorithms(),
                             PolycrystalUserObjectBase::coloringAlgorithmDescriptions());
  return params;
}

PolycrystalUserObjectBase::PolycrystalUserObjectBase(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    Coupleable(this, false),
    _mesh(_subproblem.mesh()),
    _pb(nullptr),
    _dim(_mesh.dimension()),
    _op_num(getParam<unsigned int>("op_num")),
    _grain_num(getParam<unsigned int>("grain_num")),
    _coloring_algorithm(getParam<MooseEnum>("coloring_algorithm")),
    _initialized(false),
    _vars(getCoupledMooseVars())
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

  Moose::perf_log.push("Calculate OP layout", "PolycrystalUserObjectBase");

  /**
   * For polycrystal ICs we need to assume that each of the variables has the same periodicity.
   * Since BCs are handled elsewhere in the system, we'll have to check this case explicitly.
   */
  if (_vars.size() < 1)
    mooseError("No coupled variables found");

  for (unsigned int dim = 0; dim < LIBMESH_DIM; ++dim)
  {
    bool first_variable_value = _mesh.isTranslatedPeriodic(_vars[0]->number(), dim);
    for (unsigned int i = 1; i < _vars.size(); ++i)
      if (_mesh.isTranslatedPeriodic(_vars[i]->number(), dim) != first_variable_value)
        mooseError("Coupled polycrystal variables differ in periodicity");
  }

  // Get a pointer to the PeriodicBoundaries buried in libMesh
  _pb = _fe_problem.getNonlinearSystemBase().dofMap().get_periodic_boundaries();
}

std::vector<unsigned int>
PolycrystalUserObjectBase::assignOpsToGrains(DenseMatrix<Real> & adjacency_matrix,
                                             unsigned int n_grains,
                                             unsigned int n_ops,
                                             const MooseEnum & coloring_algorithm)
{
  Moose::perf_log.push("assignOpsToGrains()", "PolycrystalICTools");

  std::vector<unsigned int> grain_to_op(n_grains, PolycrystalUserObjectBase::INVALID_COLOR);

  // Use a simple backtracking coloring algorithm
  if (coloring_algorithm == "bt")
  {
    if (!colorGraph(adjacency_matrix, grain_to_op, n_grains, n_ops, 0))
      ::mooseError(
          "Unable to find a valid Grain to op configuration, do you have enough op variables?");
  }
  else // PETSc Coloring algorithms
  {
#ifdef LIBMESH_HAVE_PETSC
    const std::string & ca_str = coloring_algorithm;
    Real * am_data = adjacency_matrix.get_values().data();
    Moose::PetscSupport::colorAdjacencyMatrix(
        am_data, n_grains, n_ops, grain_to_op, ca_str.c_str());
#else
    ::mooseError("Selected coloring algorithm requires PETSc");
#endif
  }

  Moose::perf_log.pop("assignOpsToGrains()", "PolycrystalICTools");

  return grain_to_op;
}

bool
PolycrystalUserObjectBase::colorGraph(const DenseMatrix<Real> & adjacency_matrix,
                                      std::vector<unsigned int> & colors,
                                      unsigned int n_vertices,
                                      unsigned int n_colors,
                                      unsigned int vertex)
{
  // Base case: All grains are assigned
  if (vertex == n_vertices)
    return true;

  // Consider this grain and try different ops
  for (unsigned int color_idx = 0; color_idx < n_colors; ++color_idx)
  {
    // We'll try to spread these colors around a bit rather than
    // packing them all on the first few colors if we have several colors.
    unsigned int color = (vertex + color_idx) % n_colors;

    if (isGraphValid(adjacency_matrix, colors, n_vertices, vertex, color))
    {
      colors[vertex] = color;

      if (colorGraph(adjacency_matrix, colors, n_vertices, n_colors, vertex + 1))
        return true;

      // Backtrack...
      colors[vertex] = PolycrystalUserObjectBase::INVALID_COLOR;
    }
  }

  return false;
}

bool
PolycrystalUserObjectBase::isGraphValid(const DenseMatrix<Real> & adjacency_matrix,
                                        std::vector<unsigned int> & colors,
                                        unsigned int n_vertices,
                                        unsigned int vertex,
                                        unsigned int color)
{
  // See if the proposed color is valid based on the current neighbor colors
  for (unsigned int neighbor = 0; neighbor < n_vertices; ++neighbor)
    if (adjacency_matrix(vertex, neighbor) && color == colors[neighbor])
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
