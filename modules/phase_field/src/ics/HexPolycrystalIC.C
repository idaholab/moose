/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HexPolycrystalIC.h"
#include "MooseRandom.h"

#include <cmath>

template<>
InputParameters validParams<HexPolycrystalIC>()
{
  InputParameters params = validParams<PolycrystalReducedIC>();
  params.addClassDescription("Perturbed hexagonal polycrystal");

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
     mooseError("ERROR in PolycrystalReducedIC: Number of order parameters (op_num) can't be larger than the number of grains (grain_num)");

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

  //Assign grains to specific order parameters in a way that maximizes the distance
  _assigned_op = PolycrystalICTools::assignPointsToVariables(_centerpoints,_op_num, _mesh, _var);
}
