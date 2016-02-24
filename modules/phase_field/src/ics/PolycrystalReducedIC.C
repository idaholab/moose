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

template<>
InputParameters validParams<PolycrystalReducedIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addClassDescription("Random Voronoi tesselation polycrystal (used by PolycrystalVoronoiICAction)");
  params.addRequiredParam<unsigned int>("op_num", "Number of order parameters");
  params.addRequiredParam<unsigned int>("grain_num", "Number of grains being represented by the order parameters");
  params.addRequiredParam<unsigned int>("op_index", "The index for the current order parameter");
  params.addParam<unsigned int>("rand_seed", 12444, "The random seed");
  params.addParam<bool>("cody_test", false, "Use set grain center points for Cody's test. Grain num MUST equal 10");
  params.addParam<bool>("columnar_3D", false, "3D microstructure will be columnar in the z-direction?");
  return params;
}

PolycrystalReducedIC::PolycrystalReducedIC(const InputParameters & parameters) :
    InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _dim(_mesh.dimension()),
    _nl(_fe_problem.getNonlinearSystem()),
    _op_num(getParam<unsigned int>("op_num")),
    _grain_num(getParam<unsigned int>("grain_num")),
    _op_index(getParam<unsigned int>("op_index")),
    _rand_seed(getParam<unsigned int>("rand_seed")),
    _cody_test(getParam<bool>("cody_test")),
    _columnar_3D(getParam<bool>("columnar_3D"))
{
}

void
PolycrystalReducedIC::initialSetup()
{
 //Set up domain bounds with mesh tools
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  if (_op_num > _grain_num)
     mooseError("ERROR in PolycrystalReducedIC: Number of order parameters (op_num) can't be larger than the number of grains (grain_num)");

  if (_cody_test)
    if (_op_num != 5 || _grain_num != 10)
      mooseError("ERROR in PolycrystalReducedIC: Numbers aren't correct for Cody's test.");

  MooseRandom::seed(_rand_seed);

  //Randomly generate the centers of the individual grains represented by the Voronoi tesselation
  _centerpoints.resize(_grain_num);
  _assigned_op.resize(_grain_num);
  std::vector<Real> distances(_grain_num);

  std::vector<Point> holder;

  if (_cody_test)
  {
    holder.resize(_grain_num);
    holder[0] = Point(0.2, 0.99, 0.0);
    holder[1] = Point(0.5, 0.99, 0.0);
    holder[2] = Point(0.8, 0.99, 0.0);

    holder[3] = Point(0.2, 0.5, 0.0);
    holder[4] = Point(0.5, 0.5, 0.0);
    holder[5] = Point(0.8, 0.5, 0.0);

    holder[6] = Point(0.1, 0.1, 0.0);
    holder[7] = Point(0.5, 0.05, 0.0);
    holder[8] = Point(0.9, 0.1, 0.0);

    holder[9] = Point(0.5, 0.1, 0.0);
  }

  //Assign actual center point positions
  for (unsigned int grain = 0; grain < _grain_num; grain++)
  {
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    {
      if (_cody_test)
        _centerpoints[grain](i) = _bottom_left(i) + _range(i)*holder[grain](i);
      else
        _centerpoints[grain](i) = _bottom_left(i) + _range(i)*MooseRandom::rand();
    }
    if (_columnar_3D)
        _centerpoints[grain](2) = _bottom_left(2) + _range(2)*0.5;
  }

  //Assign grains to each order parameter
  if (_cody_test)
  {
    _assigned_op[0] = 0.0;
    _assigned_op[1] = 0.0;
    _assigned_op[2] = 0.0;
    _assigned_op[3] = 1.0;
    _assigned_op[4] = 1.0;
    _assigned_op[5] = 1.0;
    _assigned_op[6] = 2.0;
    _assigned_op[7] = 0.0;
    _assigned_op[8] = 2.0;
    _assigned_op[9] = 4.0;
  }
  else
    //Assign grains to specific order parameters in a way that maximizes the distance
    _assigned_op = PolycrystalICTools::assignPointsToVariables(_centerpoints, _op_num, _mesh, _var);
}

Real
PolycrystalReducedIC::value(const Point & p)
{
  Real val = 0.0;

  unsigned int min_index = PolycrystalICTools::assignPointToGrain(p, _centerpoints, _mesh, _var, _range.size());

  //If the current order parameter index (_op_index) is equal to the min_index, set the value to 1.0
  if (_assigned_op[min_index] == _op_index) //Make sure that the _op_index goes from 0 to _op_num-1
    val = 1.0;

  if (val > 1.0)
    val = 1.0;

  if (val < 0.0)
    val = 0.0;

  return val;
}
