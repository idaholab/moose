/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


// Creates multiple superellipsoids that are positioned randomly throughout the domain
// each semiaxis can be varied by a uniform or normal distribution


#include "MultiSmoothSuperellipsoidIC.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<MultiSmoothSuperellipsoidIC>()
{
  InputParameters params = validParams<SmoothSuperellipsoidBaseIC>();
  params.addClassDescription("Random distribution of smooth ellipse with given minimum spacing");
  params.addRequiredParam<unsigned int>("numbub", "The number of bubbles to place");
  params.addRequiredParam<Real>("bubspac", "minimum spacing of bubbles, measured from center to center");
  params.addParam<unsigned int>("numtries", 1000, "The number of tries");
  params.addRequiredParam<Real>("semiaxis_a", "Mean semiaxis value in the x direction for the ellipse");
  params.addRequiredParam<Real>("semiaxis_b", "Mean semiaxis value in the y direction for the ellipse");
  params.addParam<Real>("exponent", 2,"Exponent n for each superellipsoid, n=2 is a normal ellipse");
  params.addParam<Real>("semiaxis_c", 1, "Mean semiaxis value in the z direction for the ellipse");
  params.addParam<Real>("semiaxis_a_variation", 0.0, "Plus or minus fraction of random variation in the bubble semiaxis in the x direction for uniform, standard deviation for normal");
  params.addParam<Real>("semiaxis_b_variation", 0.0, "Plus or minus fraction of random variation in the bubble semiaxis in the y direction for uniform, standard deviation for normal");
  params.addParam<Real>("semiaxis_c_variation", 0.0, "Plus or minus fraction of random variation in the bubble semiaxis in the z direction for uniform, standard deviation for normal");
  MooseEnum rand_options("uniform normal none","none");
  params.addParam<MooseEnum>("semiaxis_variation_type", rand_options, "Type of distribution that random superellipsoid semiaxes will follow");
  return params;
}

MultiSmoothSuperellipsoidIC::MultiSmoothSuperellipsoidIC(const InputParameters & parameters) :
    SmoothSuperellipsoidBaseIC(parameters),
    _numbub(getParam<unsigned int>("numbub")),
    _bubspac(getParam<Real>("bubspac")),
    _numtries(getParam<unsigned int>("numtries")),
    _exponent(getParam<Real>("exponent")),
    _semiaxis_a(getParam<Real>("semiaxis_a")),
    _semiaxis_b(getParam<Real>("semiaxis_b")),
    _semiaxis_c(getParam<Real>("semiaxis_c")),
    _semiaxis_a_variation(getParam<Real>("semiaxis_a_variation")),
    _semiaxis_b_variation(getParam<Real>("semiaxis_b_variation")),
    _semiaxis_c_variation(getParam<Real>("semiaxis_c_variation")),
    _semiaxis_variation_type(getParam<MooseEnum>("semiaxis_variation_type"))
{
}

void
MultiSmoothSuperellipsoidIC::initialSetup()
{

  //Set up domain bounds with mesh tools
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  if (_semiaxis_a_variation > 0.0 && _semiaxis_variation_type == 2)
    mooseError("If Semiaxis_a_variation > 0.0, you must pass in a Semiaxis_variation_type in MultiSmoothSuperellipsoidIC");
  if (_semiaxis_b_variation > 0.0 && _semiaxis_variation_type == 2)
    mooseError("If Semiaxis_b_variation > 0.0, you must pass in a Semiaxis_variation_type in MultiSmoothSuperellipsoidIC");
  if (_semiaxis_c_variation > 0.0 && _semiaxis_variation_type == 2)
    mooseError("If Semiaxis_c_variation > 0.0, you must pass in a Semiaxis_variation_type in MultiSmoothSuperellipsoidIC");

  SmoothSuperellipsoidBaseIC::initialSetup();
}

void
MultiSmoothSuperellipsoidIC::computeSuperellipsoidSemiaxes()
{
  _as.resize(_numbub);
  _bs.resize(_numbub);
  _cs.resize(_numbub);

  for (unsigned int i = 0; i < _numbub; i++)
  {
    //Vary bubble radius
    switch (_semiaxis_variation_type)
    {
    case 0: //Uniform distrubtion
      _as[i] = _semiaxis_a*(1.0 + (1.0 - 2.0 * _random.rand(_tid)) * _semiaxis_a_variation);
      _bs[i] = _semiaxis_b*(1.0 + (1.0 - 2.0 * _random.rand(_tid)) * _semiaxis_b_variation);
      _cs[i] = _semiaxis_c*(1.0 + (1.0 - 2.0 * _random.rand(_tid)) * _semiaxis_c_variation);
      break;
    case 1: //Normal distribution
      _as[i] = _random.randNormal(_tid, _semiaxis_a,_semiaxis_a_variation);
      _bs[i] = _random.randNormal(_tid, _semiaxis_b,_semiaxis_b_variation);
      _cs[i] = _random.randNormal(_tid, _semiaxis_c,_semiaxis_c_variation);
      break;
    case 2: //No variation
      _as[i] = _semiaxis_a;
      _bs[i] = _semiaxis_b;
      _cs[i] = _semiaxis_c;
    }

    if (_as[i] < 0.0) _as[i] = 0.0;
    if (_bs[i] < 0.0) _bs[i] = 0.0;
    if (_cs[i] < 0.0) _cs[i] = 0.0;
  }
}


void
MultiSmoothSuperellipsoidIC::computeSuperellipsoidCenters()
{
  _centers.resize(_numbub);

  for (unsigned int i = 0; i < _numbub; i++)
  {
    //Vary Superellipsoid center positions
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
      mooseError("Too many tries in MultiSuperellipsoidEllipseIC");

    _centers[i] = newcenter;
  }
}

void
MultiSmoothSuperellipsoidIC::computeSuperellipsoidExponents()
{
  _ns.resize(_numbub);

  for (unsigned int el = 0; el < _ns.size(); ++el)
    _ns[el] = _exponent;
}
