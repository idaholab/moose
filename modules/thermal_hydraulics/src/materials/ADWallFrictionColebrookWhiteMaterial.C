//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRealForward.h"
#include "ADWallFrictionColebrookWhiteMaterial.h"
#include "Numerics.h"
#include <cmath>

registerMooseObject("ThermalHydraulicsApp", ADWallFrictionColebrookWhiteMaterial);

InputParameters
ADWallFrictionColebrookWhiteMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Computes the Darcy friction factor using the Colebrook-White correlation.");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density");
  params.addRequiredParam<MaterialPropertyName>("vel", "x-component of the velocity");
  params.addRequiredParam<MaterialPropertyName>("D_h", "hydraulic diameter");

  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factor material property");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity material property");

  params.addParam<Real>("roughness", 0, "Surface roughness");
  params.declareControllable("roughness");

  params.addParam<Real>("rtol", 1e-14, "Relative tolerance for implicit solve.");
  params.addParam<unsigned int>("max_iterations", 20, "Max iterations for iterative solve.");
  MooseEnum max_its_behaviour{"error warn accept", "error"};
  params.addParam<MooseEnum>("max_iterations_behaviour",
                             max_its_behaviour,
                             "Whether to error, warn or accept when max iterations is reached");
  return params;
}

ADWallFrictionColebrookWhiteMaterial::ADWallFrictionColebrookWhiteMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _f_D_name(getParam<MaterialPropertyName>("f_D")),
    _f_D(declareADProperty<Real>(_f_D_name)),

    _mu(getADMaterialProperty<Real>("mu")),
    _rho(getADMaterialProperty<Real>("rho")),
    _vel(getADMaterialProperty<Real>("vel")),
    _D_h(getADMaterialProperty<Real>("D_h")),
    _roughness(getParam<Real>("roughness")),
    _max_its(getParam<unsigned int>("max_iterations")),
    _max_its_behaviour(getParam<MooseEnum>("max_iterations_behaviour")),
    _tol(getParam<Real>("rtol"))
{
  if (_tol < 0. || _tol >= 1.)
    mooseError("Colebrook-White friction factor relative tolerance must be between 0 and 1");
}

void
ADWallFrictionColebrookWhiteMaterial::computeQpProperties()
{
  ADReal Re = THM::Reynolds(1, _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]);
  if (Re < 4000.)
  {
    mooseDoOnce(mooseWarning("Calculated Reynolds number below 4000 (",
                             Re,
                             "), consider using different friction factor"));
  }

  // Colebrook-white equation has implicit formulation must use iteration
  ADReal & f_D = _f_D[_qp];
  ADReal f_D_old = 0;
  f_D = 0.01; // initial guess

  unsigned int it = 0;
  for (; it < _max_its; ++it)
  {
    f_D_old = f_D;
    f_D = pow(-2. * log10(_roughness / (3.7 * _D_h[_qp]) + 2.51 / (Re * sqrt(f_D))), -2.);
    if (abs(f_D - f_D_old) / f_D < _tol)
      break;
  }

  if (it == _max_its)
  {
    if (_max_its_behaviour == "error")
      mooseError("Colebrook-White friction factor maximum iterations reached: ", _max_its, ".");
    else if (_max_its_behaviour == "warn")
      mooseWarning("Colebrook-White friction factor maximum iterations reached: ", _max_its, ".");
  }
}
