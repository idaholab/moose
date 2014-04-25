#include "LatticeSmoothCircleIC.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<LatticeSmoothCircleIC>()
{
  InputParameters params = validParams<MultiSmoothCircleIC>();
  params.addParam<Real>("Rnd_variation", 0.0, "Variation from central lattice position");
  params.addRequiredParam<std::vector<unsigned int> >("circles_per_side", "Vector containing the number of bubbles along each side");
  params.set<unsigned int>("numbub") = 0;
  params.set<Real>("bubspac") = 0.0;
  params.set<Real>("x1") = 0.0;
  params.set<Real>("y1") = 0.0;

  return params;
}

LatticeSmoothCircleIC::LatticeSmoothCircleIC(const std::string & name,
                                             InputParameters parameters) :
    MultiSmoothCircleIC(name, parameters),
    _Rnd_variation(getParam<Real>("Rnd_variation")),
    _circles_per_side(getParam<std::vector<unsigned int> >("circles_per_side"))
{
}

void
LatticeSmoothCircleIC::initialSetup()
{
  MultiSmoothCircleIC::initialSetup();

  /*std::vector<unsigned int> circles_per_side;
  circles_per_side.resize(3);

  for (unsigned int i = 0; i < 3; ++i)
  circles_per_side[i] = _circles_per_side;*/

  //Moose::out << "1: "<< _circles_per_side[0] << " 2: "<< _circles_per_side[1] << " 3: " << _circles_per_side[2] << std::endl;

  if (_Ly != 0.0 && _circles_per_side[1] == 0)
    mooseError("If domain is > 1D, circles_per_side must have more than one value");

  if (_Lz != 0.0 && _circles_per_side[2] == 0)
    mooseError("If domain is 3D, circles_per_side must have three values");

  if (_Ly == 0.0)
  {
    _circles_per_side[1] = 0;
    _circles_per_side[2] = 0;
  }


  if (_Lz == 0.0)
  {
    _circles_per_side[2] = 0;
    _numbub = _circles_per_side[0] * _circles_per_side[1];
  }
  else
  {
    _numbub = _circles_per_side[0] * _circles_per_side[1] * _circles_per_side[2];
    /*if (_Lz < _Lx) //For non-uniform domain.  This needs to be changed
      circles_per_side[2] = _circles_per_side[0]*_Lz/_Lx;*/
  }


  _bubcent.resize(_numbub);
  _bubradi.resize(_numbub);

  MooseRandom::seed(_rnd_seed);

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
         //Vary circle radius
        _bubradi[cnt] = _radius*(1.0 + (1.0 - 2.0*MooseRandom::rand())*_radius_variation);

        if (_bubradi[cnt] < 0.0) _bubradi[cnt] = 0.0;

        Real xx = x_sep/2.0 + i*x_sep;
        Real yy = y_sep/2.0 + j*y_sep;
        Real zz = z_sep/2.0 + k*z_sep;

        //Vary circle position
        xx = xx + (1.0 - 2.0*MooseRandom::rand()) * _Rnd_variation;
        yy = yy + (1.0 - 2.0*MooseRandom::rand()) * _Rnd_variation;

        if (_Lz != 0.0)
          zz = zz + (1.0 - 2.0*MooseRandom::rand()) * _Rnd_variation;

        //Verify not out of bounds
        if (xx < _bubradi[cnt] + _int_width)
          xx = _bubradi[cnt] + _int_width;
        if (xx > _Lx - (_bubradi[cnt] + _int_width))
          xx = _Lx - (_bubradi[cnt] + _int_width);
        if (yy < _bubradi[cnt] + _int_width)
          yy = _bubradi[cnt] + _int_width;
        if (yy > _Ly - (_bubradi[cnt] + _int_width))
          yy = _Ly - (_bubradi[cnt] + _int_width);
        if (_Lz != 0.0)
        {
          if (zz < _bubradi[cnt] + _int_width)
            zz = _bubradi[cnt] + _int_width;
          if (zz > _Lz - (_bubradi[cnt] + _int_width))
            zz = _Lz - (_bubradi[cnt] + _int_width);
        }

        _bubcent[cnt](0) = xx;
        _bubcent[cnt](1) = yy;
        _bubcent[cnt](2) = zz;

        cnt++;
      }
}
