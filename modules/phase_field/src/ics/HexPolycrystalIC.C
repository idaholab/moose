#include "HexPolycrystalIC.h"
#include "MooseRandom.h"

#include <cmath>

template<>
InputParameters validParams<HexPolycrystalIC>()
{
  InputParameters params = validParams<PolycrystalReducedIC>();

  params.addParam<Real>("x_offset", 0.5, "Specifies offset of hexagon grid in x-direction");
  params.addParam<Real>("perturbation_percent", 0.0, "The percent to randomly perturbate centers of grains relative to the size of the grain");

  params.addParam<unsigned int>("rand_seed", 12444, "The random seed");

  params.set<int>("typ") = 1;

  return params;
}

HexPolycrystalIC::HexPolycrystalIC(const std::string & name,
                                   InputParameters parameters) :
    PolycrystalReducedIC(name, parameters),
    _x_offset(getParam<Real>("x_offset")),
    _perturbation_percent(getParam<Real>("perturbation_percent"))
{
  if (_perturbation_percent < 0.0 || _perturbation_percent > 1.0)
    mooseError("perturbation_percent out of range");
}

void
HexPolycrystalIC::initialSetup()
{
  MooseRandom::seed(_rand_seed);

  unsigned int root = std::floor(std::pow(_grain_num, 1.0/_mesh.dimension()));

  if (_grain_num != std::pow((float)root, (float)_mesh.dimension()))
  {
    root++;  // Try "ceiling due to round off error
    if (_grain_num != std::pow((float)root, (float)_mesh.dimension()))
      mooseError("HexPolycrystalIC requires a square or cubic number depending on the mesh dimension");
  }

  unsigned int third_dimension_iterations = _mesh.dimension() == 3 ? root : 1;
  Real ndist = 1.0/root;

  // Set up domain bounds with mesh tools
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  if (_op_num > _grain_num)
     mooseError("ERROR in PolycrystalReducedIC: Number of order parameters (crys_num) can't be larger than the number of grains (grain_num)");

  _centerpoints.resize(_grain_num);
  _assigned_op.resize(_grain_num);
  std::vector<Real> distances(_grain_num);

  std::vector<Point> holder(_grain_num);

  unsigned int count = 0;
  // Assign the relative center points positions, defining the grains according to a hexagonal pattern
  for (unsigned int k = 0; k < third_dimension_iterations; ++k)
    for (unsigned int j = 0; j < root; ++j)
      for (unsigned int i = 0; i < root; ++i)
      {
        // set x-coordinate
        holder[count](0) = i*ndist + (0.5*ndist*(j%2)) + _x_offset*ndist;

        // set y-coordinate
        holder[count](1) = j*ndist + (0.5*ndist*(k%2));

        // set z-coordinate
        holder[count](2) = k*ndist;

        //increment counter
        count++;
      }

  // Assign center point values
  for (unsigned int grain=0; grain < _grain_num; grain++)
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    {
      if (_range(i) == 0)
        continue;

      Real perturbation_dist = (_range(i)/root * (MooseRandom::rand()*2 - 1.0)) * _perturbation_percent;  // Perturb -100 to 100%
      _centerpoints[grain](i) = _bottom_left(i) + _range(i)*holder[grain](i) + perturbation_dist;

      if (_centerpoints[grain](i) > _top_right(i))
        _centerpoints[grain](i) = _top_right(i);
      if (_centerpoints[grain](i) < _bottom_left(i))
        _centerpoints[grain](i) = _bottom_left(i);
    }

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
        Real dist =  _mesh.minPeriodicDistance(_var.number(), _centerpoints[grain], _centerpoints[i]);
        if (min_op_dist[_assigned_op[i]] > dist)
        {
          min_op_dist[_assigned_op[i]] = dist;
          min_op_ind[_assigned_op[i]] = i;
        }
      }
    }

    // Assign the current center point to the order parameter that is furthest away.
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
