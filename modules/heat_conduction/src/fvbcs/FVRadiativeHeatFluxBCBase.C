//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVRadiativeHeatFluxBCBase.h"
#include "Function.h"
#include "MathUtils.h"

InputParameters
FVRadiativeHeatFluxBCBase::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addCoupledVar("temperature", "temperature variable");
  params.addParam<Real>("stefan_boltzmann_constant", 5.670367e-8, "The Stefan-Boltzmann constant.");
  params.addParam<FunctionName>(
      "Tinfinity", "0", "Temperature of the body in radiative heat transfer.");
  params.addParam<Real>("boundary_emissivity", 1, "Emissivity of the boundary.");
  params.addClassDescription("Boundary condition for radiative heat flux where temperature and the"
                             "temperature of a body in radiative heat transfer are specified.");
  return params;
}

FVRadiativeHeatFluxBCBase::FVRadiativeHeatFluxBCBase(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _T(isParamValid("temperature") ? adCoupledValue("temperature") : _u),
    _sigma_stefan_boltzmann(getParam<Real>("stefan_boltzmann_constant")),
    _tinf(getFunction("Tinfinity")),
    _eps_boundary(getParam<Real>("boundary_emissivity"))
{
}

ADReal
FVRadiativeHeatFluxBCBase::computeQpResidual()
{
  const auto T4 = Utility::pow<4>(_T[_qp]);
  const auto T4inf = Utility::pow<4>(_tinf.value(_t, _face_info->faceCentroid()));
  return _sigma_stefan_boltzmann * coefficient() * (T4 - T4inf);
}
