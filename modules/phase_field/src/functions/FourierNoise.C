//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FourierNoise.h"
#include "MooseRandom.h"
#include "FEProblemBase.h"
#include "libmesh/utility.h"

registerMooseObject("PhaseFieldApp", FourierNoise);

InputParameters
FourierNoise::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription("Generate noise from a fourier series");
  params.addRequiredParam<Real>("lambda",
                                "Wavelength cut off (set to about twice the interfacial width)");
  params.addParam<unsigned int>(
      "num_terms",
      "Number of random fourier series terms (this will result in non-periodic noise). Omit this "
      "parameter to obtain a periodic noise distribution.");
  params.addParam<unsigned int>("seed", 12455, "Random number generator seed");
  return params;
}

FourierNoise::FourierNoise(const InputParameters & parameters)
  : Function(parameters),
    _lambda(getParam<Real>("lambda")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  MooseRandom rng;
  rng.seed(0, getParam<unsigned int>("seed"));

  if (isParamValid("num_terms"))
  {
    // random terms
    _series.resize(getParam<unsigned int>("num_terms"));
    const Real scale = 2.0 * (2.0 * libMesh::pi) / _lambda;

    // check
    if (_series.empty())
      paramError("num_terms",
                 "If specifying the number of terms, supply a number greater than zero.");

    // fill terms
    for (auto & f : _series)
    {
      // get a vector with length <= 0.5
      Real r2;
      do
      {
        const Real x = rng.rand(0) - 0.5;
        const Real y = rng.rand(0) - 0.5;
        f.k = RealVectorValue(x, y, 0.0);
        r2 = f.k.norm_sq();
      } while (r2 > 0.25);

      // scale maximum to a wavelength of lambda
      f.k *= scale;

      f.c = rng.randNormal(0, 0.0, 1.0);
      f.s = rng.randNormal(0, 0.0, 1.0);
    }
  }
  else
  {
    // k-space grid resulting in periodic noise
    MooseMesh & mesh = _fe_problem.mesh();
    if (!mesh.isRegularOrthogonal())
      mooseError("Periodic Fourier Noise requires a regular orthogonal mesh.");

    const Real dx = 2.0 * libMesh::pi / mesh.dimensionWidth(0);
    const Real dy = 2.0 * libMesh::pi / mesh.dimensionWidth(1);
    const Real rmax = 2.0 * libMesh::pi / _lambda;

    const int xmax = rmax / dx;
    const int ymax = rmax / dy;

    const Real rmax2 = rmax * rmax;
    for (int x = 0; x < xmax; ++x)
      for (int y = -ymax; y < ymax; ++y)
        if (x > 0 || y > 0)
        {
          SeriesItem f;
          f.k = RealVectorValue(x * dx, y * dy, 0.0);
          if (f.k.norm_sq() <= rmax2)
          {
            f.c = rng.randNormal(0, 0.0, 1.0);
            f.s = rng.randNormal(0, 0.0, 1.0);
            _series.push_back(f);
          }
        }
  }

  _scale = std::sqrt(1.0 / _series.size());
}

Real
FourierNoise::value(Real, const Point & p) const
{
  Real v = 0.0;
  for (const auto & f : _series)
    v += f.s * std::sin(p * f.k) + f.c * std::cos(p * f.k);
  return v * _scale;
}

ADReal
FourierNoise::value(const ADReal &, const ADPoint & p) const
{
  ADReal v = 0.0;
  for (const auto & f : _series)
    v += f.s * std::sin(p * f.k) + f.c * std::cos(p * f.k);
  return v * _scale;
}
