/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "WaterFluidPropertiesTestMaterial.h"

template<>
InputParameters validParams<WaterFluidPropertiesTestMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("pressure", "pressure (Pa)");
  params.addRequiredCoupledVar("temperature", "temperature (K)");
  params.addCoupledVar("density", 1000, "density (kg/m^3)");
  params.addParam<bool>("region4", false, "Set true if calculating saturation curve properties");
  params.addParam<bool>("b23", false, "Set true if calculating properties for the boundary between regions 2 and 3");
  params.addParam<bool>("viscosity", false, "Set true if calculating viscosity using a given density");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription("Material to test water properties");
  return params;
}

WaterFluidPropertiesTestMaterial::WaterFluidPropertiesTestMaterial(const InputParameters & parameters) :
    Material(parameters),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),
    _density(coupledValue("density")),
    _region4(getParam<bool>("region4")),
    _b23(getParam<bool>("b23")),
    _viscosity(getParam<bool>("viscosity")),

    _rho(declareProperty<Real>("density")),
    _mu(declareProperty<Real>("viscosity")),
    _e(declareProperty<Real>("internal_energy")),
    _h(declareProperty<Real>("enthalpy")),
    _s(declareProperty<Real>("entropy")),
    _cp(declareProperty<Real>("cp")),
    _cv(declareProperty<Real>("cv")),
    _c(declareProperty<Real>("c")),
    _k(declareProperty<Real>("k")),
    _psat(declareProperty<Real>("psat")),
    _Tsat(declareProperty<Real>("Tsat")),
    _b23p(declareProperty<Real>("b23p")),
    _b23T(declareProperty<Real>("b23T")),

    _fp(getUserObject<Water97FluidProperties>("fp"))
{
}

WaterFluidPropertiesTestMaterial::~WaterFluidPropertiesTestMaterial()
{
}

void
WaterFluidPropertiesTestMaterial::computeQpProperties()
{
  if (_viscosity)
    _mu[_qp] = _fp.mu(_density[_qp], _temperature[_qp]);
  else
  {
    _rho[_qp] = _fp.rho(_pressure[_qp], _temperature[_qp]);
    _mu[_qp] = _fp.mu(_rho[_qp], _temperature[_qp]);
    _e[_qp] = _fp.e(_pressure[_qp], _temperature[_qp]);
    _h[_qp] = _fp.h(_pressure[_qp], _temperature[_qp]);
    _s[_qp] = _fp.s(_pressure[_qp], _temperature[_qp]);
    _cp[_qp] = _fp.cp(_pressure[_qp], _temperature[_qp]);
    _cv[_qp] = _fp.cv(_pressure[_qp], _temperature[_qp]);
    _c[_qp] = _fp.c(_pressure[_qp], _temperature[_qp]);
    _k[_qp] = _fp.k(_rho[_qp], _temperature[_qp]);

    if (_region4)
    {
      _psat[_qp] = _fp.pSat(_temperature[_qp]);
      _Tsat[_qp] = _fp.TSat(_pressure[_qp]);
    }
    else
    {
      _psat[_qp] = 0.0;
      _Tsat[_qp] = 0.0;
    }

    if (_b23)
    {
      _b23p[_qp] = _fp.b23p(_temperature[_qp]);
      _b23T[_qp] = _fp.b23T(_pressure[_qp]);
    }
    else
    {
      _b23p[_qp] = 0.0;
      _b23T[_qp] = 0.0;
    }
  }
}
