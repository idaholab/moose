/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "BimodalInverseSuperellipsoidsIC.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<BimodalInverseSuperellipsoidsIC>()
{
  InputParameters params = validParams<BimodalSuperellipsoidsIC>();
  params.addClassDescription("Bimodal size distribution of large particles (specified in input file, value invalue) and small particles (placed randomly inside the larger particles, value outvalue)");
  return params;
}

BimodalInverseSuperellipsoidsIC::BimodalInverseSuperellipsoidsIC(const InputParameters & parameters) :
    BimodalSuperellipsoidsIC(parameters)
{
}

void
BimodalInverseSuperellipsoidsIC::initialSetup()
{
  if (_size_variation_type == 2 && _size_variation > 0.0)
    mooseError("If size_variation > 0.0, you must pass in a size_variation_type in BimodalInverseSuperellipsoidsIC");

  BimodalSuperellipsoidsIC::initialSetup();
}

Real
BimodalInverseSuperellipsoidsIC::value(const Point & p)
{
  Real value = _outvalue;
  Real val2 = 0.0;

  //First loop over the specified superellipsoids
  for (unsigned int ellip = 0; ellip < _x_positions.size() && value != _invalue; ++ellip)
  {
    val2 = computeSuperellipsoidValue(p, _centers[ellip], _as[ellip], _bs[ellip], _cs[ellip], _ns[ellip]);
    if ( (val2 > value && _invalue > _outvalue) || (val2 < value && _outvalue > _invalue) )
      value = val2;
  }

  //Then loop over the randomly positioned particles and set value inside them back to outvalue
  for (unsigned int ellip = _x_positions.size(); ellip < _x_positions.size() + _npart; ++ellip)
  {
    val2 = computeSuperellipsoidInverseValue(p, _centers[ellip], _as[ellip], _bs[ellip], _cs[ellip], _ns[ellip]);
    if ( (val2 < value && _invalue > _outvalue) || (val2 > value && _outvalue > _invalue) )
      value = val2;
  }

  return value;
}


void
BimodalInverseSuperellipsoidsIC::computeSuperellipsoidCenters()
{
  _centers.resize(_x_positions.size() + _npart);

  //First place the specified (large) particles from the input file
  for (unsigned int i = 0; i < _x_positions.size(); ++i)
  {
    _centers[i](0) = _x_positions[i];
    _centers[i](1) = _y_positions[i];
    _centers[i](2) = _z_positions[i];
  }

  //Next place the randomly positioned (small) particles
  for (unsigned int i = _x_positions.size(); i < _x_positions.size() + _npart; i++)
  {
    unsigned int num_tries = 0;

    Real dsmall_min = 0.0; //minimum distance from the random particle to another random particle
    Real dlarge_max = 0.0; //minimum distance from the random particle to edge of the specified (large) particle

    Point newcenter = 0.0;

    while ((dsmall_min < _small_spac || dlarge_max < _large_spac) && num_tries < _numtries)
    {
      num_tries++;

      Real ran1 = _random.rand(_tid);
      Real ran2 = _random.rand(_tid);
      Real ran3 = _random.rand(_tid);

      newcenter(0) = _bottom_left(0) + ran1*_range(0);
      newcenter(1) = _bottom_left(1) + ran2*_range(1);
      newcenter(2) = _bottom_left(2) + ran3*_range(2);

      //First check to make sure we are INSIDE a larger particle
      for (unsigned int j = 0; j < _x_positions.size(); j++)
      {
        if (j == 0)
          dlarge_max = - _range.norm();

        //Compute the distance r1 from the center of each specified superellipsoid to its outside edge
        //along the vector between the specified superellipsoid and the current randomly
        //positioned one
        //This uses the equation for a superellipse in polar coordinates and substitutes
        //distances for sin, cos functions
        Real dist = _mesh.minPeriodicDistance(_var.number(), _centers[j], newcenter);
        Point dist_vec = _mesh.minPeriodicVector(_var.number(), _centers[j], newcenter);

        //First calculate rmn1 = r1^(-n), replacing sin, cos functions with distances
        Real rmn1 = (std::pow(std::abs(dist_vec(0) / dist / _as[j]), _ns[j])
                  + std::pow(std::abs(dist_vec(1) / dist / _bs[j]), _ns[j])
                  + std::pow(std::abs(dist_vec(2) / dist / _cs[j]), _ns[j]) );
        //Then calculate r1 from rmn1
        Real r1 = std::pow(rmn1, (-1.0/_ns[j]));

        //Now calculate the distance r2 from the center of the randomly placed superellipsoid
        //to its outside edge in the same manner
        Real rmn2 = (std::pow(std::abs(dist_vec(0) / dist / _as[i]), _ns[i])
                  + std::pow(std::abs(dist_vec(1) / dist / _bs[i]), _ns[i])
                  + std::pow(std::abs(dist_vec(2) / dist / _cs[i]), _ns[i]) );
        Real r2 = std::pow(rmn2, (-1.0/_ns[i]));

        //Calculate the distance between the edges for an interior particle
        Real tmp_dlarge_max = r1 - dist - r2;

        if (tmp_dlarge_max > dlarge_max)
          dlarge_max = tmp_dlarge_max;
      }

      //Then check for collisions between the randomly placed particles
      for (unsigned int j = _x_positions.size(); j < i; j++)
      {
        if (j == _x_positions.size())
          dsmall_min = _range.norm();

        Real dist = _mesh.minPeriodicDistance(_var.number(), _centers[j], newcenter);
        Point dist_vec = _mesh.minPeriodicVector(_var.number(), _centers[j], newcenter);

        Real rmn1 = (std::pow(std::abs(dist_vec(0) / dist / _as[j]), _ns[j])
                  + std::pow(std::abs(dist_vec(1) / dist / _bs[j]), _ns[j])
                  + std::pow(std::abs(dist_vec(2) / dist / _cs[j]), _ns[j]) );
        Real r1 = std::pow(rmn1, (-1.0/_ns[j]));

        Real rmn2 = (std::pow(std::abs(dist_vec(0) / dist / _as[i]), _ns[i])
                  + std::pow(std::abs(dist_vec(1) / dist / _bs[i]), _ns[i])
                  + std::pow(std::abs(dist_vec(2) / dist / _cs[i]), _ns[i]) );
        Real r2 = std::pow(rmn2, (-1.0/_ns[i]));

        //Calculate the distance between the edges
        Real tmp_dsmall_min = dist - r1 - r2;

        if (tmp_dsmall_min < dsmall_min)
          dsmall_min = tmp_dsmall_min;
      }
      //Cause while statement to exit for the first randomly placed particle
      if (i == _x_positions.size())
        dsmall_min = _range.norm();
    }

    if (num_tries == _numtries)
      mooseError("Too many tries in BimodalInverseSuperellipsoidsIC");

    _centers[i] = newcenter;
  }
}
