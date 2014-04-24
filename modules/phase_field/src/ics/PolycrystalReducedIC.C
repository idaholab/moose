#include "PolycrystalReducedIC.h"
#include "IndirectSort.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<PolycrystalReducedIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<unsigned int>("crys_num", "Number of order parameters");
  params.addRequiredParam<unsigned int>("grain_num", "Number of grains being represented by the order parameters");
  params.addRequiredParam<unsigned int>("crys_index", "The index for the current order parameter");

  params.addParam<unsigned int>("rand_seed", 12444, "The random seed");

  params.addParam<bool>("cody_test", false, "Use set grain center points for Cody's test. Grain num MUST equal 10");

  params.addParam<bool>("columnar_3D", false, "3D microstructure will be columnar in the z-direction?");

  return params;
}

PolycrystalReducedIC::PolycrystalReducedIC(const std::string & name,
                                           InputParameters parameters) :
    InitialCondition(name, parameters),
    _mesh(_fe_problem.mesh()),
    _nl(_fe_problem.getNonlinearSystem()),
    _op_num(getParam<unsigned int>("crys_num")),
    _grain_num(getParam<unsigned int>("grain_num")),
    _op_index(getParam<unsigned int>("crys_index")),
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
     mooseError("ERROR in PolycrystalReducedIC: Number of order parameters (crys_num) can't be larger than the number of grains (grain_num)");

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
    holder[0] = Point(0.2, 0.85, 0.0);
    holder[1] = Point(0.5, 0.85, 0.0);
    holder[2] = Point(0.8, 0.85, 0.0);

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
    _assigned_op[7] = 3.0;
    _assigned_op[8] = 2.0;
    _assigned_op[9] = 4.0;
  }
  else
  {
    for (unsigned int grain = 0; grain < _grain_num; grain++) //Assign grains to specific order parameters in a way that maximized the distance
    {
      std::vector<int> min_op_ind;
      std::vector<Real> min_op_dist;
      min_op_ind.resize(_op_num);
      min_op_dist.resize(_op_num);
      //Determine the distance to the closest center assigned to each order parameter
      if (grain >= _op_num)
      {
        std::fill(min_op_dist.begin() , min_op_dist.end(), _range.size());
        for (unsigned int i = 0; i < grain; i++)
        {
          Real dist = _mesh.minPeriodicDistance(_var.number(), _centerpoints[grain], _centerpoints[i]);
          if (min_op_dist[_assigned_op[i]] > dist)
          {
            min_op_dist[_assigned_op[i]] = dist;
            min_op_ind[_assigned_op[i]] = i;
          }
        }
      }

      //Assign the current center point to the order parameter that is furthest away.
      Real mx;
      if (grain < _op_num)
        _assigned_op[grain] = grain;
      else
      {
        mx = 0.0;
        unsigned int mx_ind = 1e6;
        for (unsigned int i = 0; i < _op_num; i++) //Find index of max
          if (mx < min_op_dist[i])
          {
            mx = min_op_dist[i];
            mx_ind = i;
          }

        _assigned_op[grain] = mx_ind;
      }
      //Moose::out << "For grain " << grain << ", center point = " << _centerpoints[grain](0) << " " << _centerpoints[grain](1) << "\n";
      //Moose::out << "Max index is " << _assigned_op[grain] << ", with a max distance of " << mx << "\n";
    }
  }
}

Real
PolycrystalReducedIC::value(const Point & p)
{
  // Assumption: We are going to assume that all variables are periodic together
  // _mesh.initPeriodicDistanceForVariable(_nl, _var.number());

  Real min_distance = _top_right(0)*1e5;
  Real val = 0.0;
  unsigned int min_index = _grain_num + 100;
  //Loops through all of the grain centers and finds the center that is closest to the point p
  for (unsigned int grain = 0; grain < _grain_num; grain++)
  {
    Real distance = _mesh.minPeriodicDistance(_var.number(), _centerpoints[grain], p);

    if (min_distance > distance)
    {
      min_distance = distance;
      min_index = grain;
    }
  }

  if (min_index > _grain_num)
    mooseError("ERROR in PolycrystalReducedIC: didn't find minimum values");

  //If the current order parameter index (_op_index) is equal to the min_index, set the value to 1.0
  if (_assigned_op[min_index] == _op_index) //Make sure that the _op_index goes from 0 to _op_num-1
    val = 1.0;

  if (val > 1.0)
    val = 1.0;

  if (val < 0.0)
    val = 0.0;

  return val;
}
