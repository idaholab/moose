#include "LatticeSmoothCircleIC.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<LatticeSmoothCircleIC>()
{
  InputParameters params = validParams<SmoothCircleBaseIC>();
  params.addParam<Real>("Rnd_variation", 0.0, "Variation from central lattice position");
  params.addRequiredParam<std::vector<unsigned int> >("circles_per_side", "Vector containing the number of bubbles along each side");
  params.addRequiredParam<Real>("Lx", "length of simulation domain in x-direction");
  params.addRequiredParam<Real>("Ly", "length of simulation domain in y-direction");
  params.addParam<Real>("Lz", 0.0, "length of simulation domain in z-direction");
  params.addParam<unsigned int>("rand_seed", 2000, "random seed");
  params.addRequiredParam<Real>("radius", "Mean radius value for the circels");
  params.addParam<Real>("radius_variation", 0.0, "Plus or minus fraction of random variation in the bubble radius");

  return params;
}

LatticeSmoothCircleIC::LatticeSmoothCircleIC(const std::string & name,
                                             InputParameters parameters) :
    SmoothCircleBaseIC(name, parameters),
    _lattice_variation(getParam<Real>("Rnd_variation")),
    _circles_per_side(getParam<std::vector<unsigned int> >("circles_per_side")),
    _Lx(getParam<Real>("Lx")),
    _Ly(getParam<Real>("Ly")),
    _Lz(getParam<Real>("Lz")),
    _radius(getParam<Real>("radius")),
    _radius_variation(getParam<Real>("radius_variation"))
{
  //Error checks
  if (_Ly != 0.0 && _circles_per_side[1] == 0)
    mooseError("If domain is > 1D, circles_per_side must have more than one value");

  if (_Lz != 0.0 && _circles_per_side[2] == 0)
    mooseError("If domain is 3D, circles_per_side must have three values");

  if (_Ly == 0.0)
  {
    _circles_per_side[1] = 0;
    _circles_per_side[2] = 0;
  }

  //Set _numbub
  if (_Lz == 0.0)
  {
    _circles_per_side[2] = 0;
    _numbub = _circles_per_side[0] * _circles_per_side[1];
  }
  else
    _numbub = _circles_per_side[0] * _circles_per_side[1] * _circles_per_side[2];

  //Set random seed
  MooseRandom::seed(getParam<unsigned int>("rand_seed"));
}

void
LatticeSmoothCircleIC::computeCircleRadii()
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
LatticeSmoothCircleIC::computeCircleCenters()
{
  _centers.resize(_numbub);

  Real x_sep = _Lx / _circles_per_side[0];
  Real y_sep = _Ly / _circles_per_side[1];

  Real z_sep = 0.0;
  unsigned int z_num = 1.0;

  if (_Lz > 0.0)
  {
    z_sep = _Lz / _circles_per_side[2];
    z_num = _circles_per_side[2];
  }

  unsigned int cnt = 0;
  for (unsigned int i = 0; i < _circles_per_side[0]; i++)
    for (unsigned int j = 0; j < _circles_per_side[1]; j++)
      for (unsigned int k = 0; k < z_num; k++)
      {
        Real xx = x_sep/2.0 + i*x_sep;
        Real yy = y_sep/2.0 + j*y_sep;
        Real zz = z_sep/2.0 + k*z_sep;

        //Vary circle position
        xx = xx + (1.0 - 2.0*MooseRandom::rand()) * _lattice_variation;
        yy = yy + (1.0 - 2.0*MooseRandom::rand()) * _lattice_variation;

        if (_Lz != 0.0)
          zz = zz + (1.0 - 2.0*MooseRandom::rand()) * _lattice_variation;

        //Verify not out of bounds
        if (xx < _radii[cnt] + _int_width)
          xx = _radii[cnt] + _int_width;
        if (xx > _Lx - (_radii[cnt] + _int_width))
          xx = _Lx - (_radii[cnt] + _int_width);
        if (yy < _radii[cnt] + _int_width)
          yy = _radii[cnt] + _int_width;
        if (yy > _Ly - (_radii[cnt] + _int_width))
          yy = _Ly - (_radii[cnt] + _int_width);
        if (_Lz != 0.0)
        {
          if (zz < _radii[cnt] + _int_width)
            zz = _radii[cnt] + _int_width;
          if (zz > _Lz - (_radii[cnt] + _int_width))
            zz = _Lz - (_radii[cnt] + _int_width);
        }

        _centers[cnt](0) = xx;
        _centers[cnt](1) = yy;
        _centers[cnt](2) = zz;

        cnt++;
      }
}
