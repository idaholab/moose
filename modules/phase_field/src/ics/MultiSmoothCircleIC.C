#include "MultiSmoothCircleIC.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<MultiSmoothCircleIC>()
{
  InputParameters params = validParams<SmoothCircleBaseIC>();
  params.addRequiredParam<unsigned int>("numbub", "The number of bubbles to be placed on GB");
  params.addRequiredParam<Real>("bubspac", "minimum spacing of bubbles, measured from center to center");
  params.addRequiredParam<Real>("Lx", "length of simulation domain in x-direction");
  params.addRequiredParam<Real>("Ly", "length of simulation domain in y-direction");
  params.addParam<Real>("Lz", 0.0, "length of simulation domain in z-direction");
  params.addParam<unsigned int>("rand_seed", 2000, "random seed");
  params.addParam<unsigned int>("numtries", 1000, "The number of tries");
  params.addRequiredParam<Real>("radius", "Mean radius value for the circels");
  params.addParam<Real>("radius_variation", 0.0, "Plus or minus fraction of random variation in the bubble radius");

  return params;
}

MultiSmoothCircleIC::MultiSmoothCircleIC(const std::string & name,
                                         InputParameters parameters) :
    SmoothCircleBaseIC(name, parameters),
    _numbub(getParam<unsigned int>("numbub")),
    _bubspac(getParam<Real>("bubspac")),
    _Lx(getParam<Real>("Lx")),
    _Ly(getParam<Real>("Ly")),
    _Lz(getParam<Real>("Lz")),
    _numtries(getParam<unsigned int>("numtries")),
    _radius(getParam<Real>("radius")),
    _radius_variation(getParam<Real>("radius_variation"))
{
  MooseRandom::seed(getParam<unsigned int>("rand_seed"));
}

void
MultiSmoothCircleIC::computeCircleRadii()
{
  _radii.resize(_numbub);

  for (unsigned int i = 0; i < _numbub; i++)
  {
    //Vary bubble radius
    _radii[i] = _radius * (1.0 + (1.0 - 2.0*MooseRandom::rand()) * _radius_variation);
    if (_radii[i] < 0.0) _radii[i] = 0.0;
  }
}


void
MultiSmoothCircleIC::computeCircleCenters()
{
  _centers.resize(_numbub);

  //Set up domain bounds with mesh tools
  Point top_right; //Actually a point containing the max dimensions in x, y, and z.
  Point bottom_left; //Actually a point containing the min dimensions in x, y, and z.

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    top_right(i) = _mesh.getMaxInDimension(i);
    bottom_left(i) = _mesh.getMinInDimension(i);
  }

  Point range = top_right - bottom_left;

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

      Real ran1 = MooseRandom::rand();
      Real ran2 = MooseRandom::rand();
      Real ran3 = MooseRandom::rand();

      newcenter(0) = ran1*(_Lx - _bubspac) + 0.5*_bubspac;
      newcenter(1) = ran2*(_Ly - _bubspac) + 0.5*_bubspac;

      if (_Lz != 0.0)
        newcenter(2) = ran3 * (_Lz - _bubspac) + 0.5 * _bubspac;

      for (unsigned int j = 0; j < i; j++)
      {
        if (j == 0) rr = 1000.0;

        Real tmp_rr = _mesh.minPeriodicDistance(_var.number(), _centers[j], newcenter);
        if (tmp_rr < rr)
          rr = tmp_rr;
      }

      if (i == 0) rr = range.size();
    }

    if (num_tries == _numtries)
      mooseError("Too many tries in MultiSmoothCircleIC");

    _centers[i] = newcenter;
  }
}
