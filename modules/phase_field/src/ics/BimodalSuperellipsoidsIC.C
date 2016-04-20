/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "BimodalSuperellipsoids.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<BimodalSuperellipsoidsIC>()
{
  InputParameters params = validParams<SmoothSuperellipsoidBaseIC>();
  params.addClassDescription("Bimodal size distribution of large particles (specified in input file) and small particles (placed randomly)");
  params.addRequiredParam<std::vector<Real> >("x_positions", "The x-coordinate for each large superellipsoid center");
  params.addRequiredParam<std::vector<Real> >("y_positions", "The y-coordinate for each large superellipsoid center");
  params.addRequiredParam<std::vector<Real> >("z_positions", "The z-coordinate for each large superellipsoid center");
  params.addRequiredParam<std::vector<Real> >("as", "Semiaxis a for each large superellipsoid");
  params.addRequiredParam<std::vector<Real> >("bs", "Semiaxis b for each large superellipsoid");
  params.addRequiredParam<std::vector<Real> >("cs", "Semiaxis c for each large superellipsoid");
  params.addRequiredParam<std::vector<Real> >("ns", "Exponent n for each large superellipsoid");
  params.addRequiredParam<unsigned int>("npart", "The number of random (small) particles to place");
  params.addRequiredParam<Real>("small_spac", "minimum spacing between small particles, measured from closest edge to closest edge");
  params.addRequiredParam<Real>("large_spac", "minimum spacing between large and small particles, measured from closest edge to closest edge");
  params.addRequiredParam<Real>("small_a", "Mean semiaxis a value for the randomly placed (small) superellipsoids");
  params.addRequiredParam<Real>("small_b", "Mean semiaxis b value for the randomly placed (small) superellipsoids");
  params.addRequiredParam<Real>("small_c", "Mean semiaxis c value for the randomly placed (small) superellipsoids");
  params.addRequiredParam<Real>("small_n", "Exponent n for the randomly placed (small) superellipsoids");
  params.addParam<Real>("size_variation", 0.0, "Plus or minus fraction of random variation in the semiaxes for uniform, standard deviation for normal");
  MooseEnum rand_options("uniform normal none","none");
  params.addParam<MooseEnum>("size_variation_type", rand_options, "Type of distribution that random semiaxes will follow");
  params.addParam<unsigned int>("numtries", 1000, "The number of tries to place the random particles");
  return params;
}

BimodalSuperellipsoidsIC::BimodalSuperellipsoidsIC(const InputParameters & parameters) :
    SmoothCircleBaseIC(parameters),
    _x_positions(getParam<std::vector<Real> >("x_positions")),
    _y_positions(getParam<std::vector<Real> >("y_positions")),
    _z_positions(getParam<std::vector<Real> >("z_positions")),
    _input_as(getParam<std::vector<Real> >("as")),
    _input_bs(getParam<std::vector<Real> >("bs")),
    _input_cs(getParam<std::vector<Real> >("cs")),
    _input_ns(getParam<std::vector<Real> >("ns")),
    _npart(getParam<unsigned int>("npart")),
    _small_spac(getParam<Real>("small_spac")),
    _large_spac(getParam<Real>("large_spac")),
    _small_a(getParam<Real>("small_a")),
    _small_b(getParam<Real>("small_b")),
    _small_c(getParam<Real>("small_c")),
    _small_n(getParam<Real>("small_n")),
    _size_variation(getParam<Real>("size_variation")),
    _size_variation_type(getParam<MooseEnum>("size_variation_type")),
    _numtries(getParam<unsigned int>("numtries"))
{
}

void
BimodalSuperellipsoidsIC::initialSetup()
{

  //Set up domain bounds with mesh tools
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  switch (_size_variation_type)
  {
  case 2: //No variation
    if (_size_variation > 0.0)
      mooseError("If size_variation > 0.0, you must pass in a size_variation_type in BimodalSuperellipsoidsIC");
    break;
  }

  SmoothSuperellipsoidBaseIC::initialSetup();
}

void
BimodalSuperellipsoidsIC::computeCircleRadii()
{
  _radii.resize(_numbub);

  for (unsigned int i = 0; i < _numbub; i++)
  {
    //Vary bubble radius
    switch (_radius_variation_type)
    {
    case 0: //Uniform distrubtion
      _radii[i] = _radius*(1.0 + (1.0 - 2.0 * _random.rand(_tid)) * _radius_variation);
      break;
    case 1: //Normal distribution
      _radii[i] = _random.randNormal(_tid, _radius,_radius_variation);
      break;
    case 2: //No variation
      _radii[i] = _radius;
    }

    if (_radii[i] < 0.0) _radii[i] = 0.0;
  }
}


void
BimodalSuperellipsoidsIC::computeCircleCenters()
{
  _centers.resize(_numbub);

  for (unsigned int i = 0; i < _numbub; i++)
  {
    //Vary circle center positions
    unsigned int num_tries = 0;

    Real rr = 0.0;
    Point newcenter = 0.0;

    while (rr < _bubspac && num_tries < _numtries)
    {
      num_tries++;
      //Moose::out<<"num_tries: "<<num_tries<<std::endl;

      Real ran1 = _random.rand(_tid);
      Real ran2 = _random.rand(_tid);
      Real ran3 = _random.rand(_tid);

      newcenter(0) = _bottom_left(0) + ran1*_range(0);
      newcenter(1) = _bottom_left(1) + ran2*_range(1);
      newcenter(2) = _bottom_left(2) + ran3*_range(2);

      for (unsigned int j = 0; j < i; j++)
      {
        if (j == 0) rr = _range.norm();

        Real tmp_rr = _mesh.minPeriodicDistance(_var.number(), _centers[j], newcenter);

        if (tmp_rr < rr) rr = tmp_rr;
      }

      if (i == 0) rr = _range.norm();
    }

    if (num_tries == _numtries)
      mooseError("Too many tries in BimodalSuperellipsoidsIC");

    _centers[i] = newcenter;
  }
}
