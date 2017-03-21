/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PolycrystalReducedIC.h"
#include "IndirectSort.h"
#include "MooseRandom.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<PolycrystalReducedIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addClassDescription(
      "Random Voronoi tesselation polycrystal (used by PolycrystalVoronoiICAction)");
  params.addRequiredParam<unsigned int>("op_num", "Number of order parameters");
  params.addRequiredParam<unsigned int>(
      "grain_num", "Number of grains being represented by the order parameters");
  params.addRequiredParam<unsigned int>("op_index", "The index for the current order parameter");
  params.addParam<unsigned int>("rand_seed", 12444, "The random seed");
  params.addParam<bool>(
      "columnar_3D", false, "3D microstructure will be columnar in the z-direction?");
  params.addParam<bool>("advanced_op_assignment",
                        false,
                        "Enable advanced grain to op assignment (avoid invalid graph coloring)");
  return params;
}

PolycrystalReducedIC::PolycrystalReducedIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _dim(_mesh.dimension()),
    _op_num(getParam<unsigned int>("op_num")),
    _grain_num(getParam<unsigned int>("grain_num")),
    _op_index(getParam<unsigned int>("op_index")),
    _rand_seed(getParam<unsigned int>("rand_seed")),
    _columnar_3D(getParam<bool>("columnar_3D")),
    _advanced_op_assignment(getParam<bool>("advanced_op_assignment"))
{
}

void
PolycrystalReducedIC::initialSetup()
{
  // Set up domain bounds with mesh tools
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  if (_op_num > _grain_num)
    mooseError("ERROR in PolycrystalReducedIC: Number of order parameters (op_num) can't be larger "
               "than the number of grains (grain_num)");

  MooseRandom::seed(_rand_seed);

  // Randomly generate the centers of the individual grains represented by the Voronoi tesselation
  _centerpoints.resize(_grain_num);
  std::vector<Real> distances(_grain_num);

  for (unsigned int grain = 0; grain < _grain_num; grain++)
  {
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
      _centerpoints[grain](i) = _bottom_left(i) + _range(i) * MooseRandom::rand();
    if (_columnar_3D)
      _centerpoints[grain](2) = _bottom_left(2) + _range(2) * 0.5;
  }

  if (!_advanced_op_assignment)
    // Assign grains to specific order parameters in a way that maximizes the distance
    _assigned_op = PolycrystalICTools::assignPointsToVariables(_centerpoints, _op_num, _mesh, _var);
  else
  {
    std::map<dof_id_type, unsigned int> entity_to_grain;

    // TODO: Add a nodal option
    if (false)
    {
      /**
       * We first need to build a node to grain map (i.e. every node in the mesh needs to say
       * which grain it belongs to). For Voronoi, this is straightforward and we have a utility
       * already setup to handle this case.
       */
      const MeshBase::node_iterator end = _mesh.getMesh().active_nodes_end();
      for (MeshBase::node_iterator nl = _mesh.getMesh().active_nodes_begin(); nl != end; ++nl)
      {
        unsigned int grain_index =
            PolycrystalICTools::assignPointToGrain(**nl, _centerpoints, _mesh, _var, _range.norm());

        entity_to_grain.insert(std::pair<dof_id_type, unsigned int>((*nl)->id(), grain_index));
      }
    }
    else
    {
      const MeshBase::element_iterator end = _mesh.getMesh().active_elements_end();
      for (MeshBase::element_iterator el = _mesh.getMesh().active_elements_begin(); el != end; ++el)
      {
        Point centroid = (*el)->centroid();

        unsigned int grain_index = PolycrystalICTools::assignPointToGrain(
            centroid, _centerpoints, _mesh, _var, _range.norm());

        entity_to_grain.insert(std::pair<dof_id_type, unsigned int>((*el)->id(), grain_index));
      }
    }

    /**
     * Now we need to construct a neighbor graph using our node to grain map information.
     * We have a utility for this too. This one makes no assumptions about how the
     * grain structure was built. It uses the entity_to_grain map.
     */
    AdjacencyGraph grain_neighbor_graph =
        PolycrystalICTools::buildGrainAdjacencyGraph(entity_to_grain, _mesh, _grain_num, true);

    /**
     * Now we need to assign ops in some optimal fashion.
     */
    _assigned_op = PolycrystalICTools::assignOpsToGrains(grain_neighbor_graph, _grain_num, _op_num);
  }
}

Real
PolycrystalReducedIC::value(const Point & p)
{
  unsigned int min_index =
      PolycrystalICTools::assignPointToGrain(p, _centerpoints, _mesh, _var, _range.norm());

  // If the current order parameter index (_op_index) is equal to the min_index, set the value to
  // 1.0
  if (_assigned_op[min_index] == _op_index) // Make sure that the _op_index goes from 0 to _op_num-1
    return 1.0;
  else
    return 0.0;
}
