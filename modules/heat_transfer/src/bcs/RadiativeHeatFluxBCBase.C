//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadiativeHeatFluxBCBase.h"
#include "Function.h"
#include "MathUtils.h"

template <bool is_ad>
InputParameters
RadiativeHeatFluxBCBaseTempl<is_ad>::validParams()
{
  InputParameters params = GenericIntegratedBC<is_ad>::validParams();
  params.addParam<Real>("stefan_boltzmann_constant", 5.670367e-8, "The Stefan-Boltzmann constant.");
  params.addParam<FunctionName>(
      "Tinfinity", "0", "Temperature of the body in radiative heat transfer.");
  params.addParam<Real>("boundary_emissivity", 1, "Emissivity of the boundary.");
  params.addClassDescription("Boundary condition for radiative heat flux where temperature and the"
                             "temperature of a body in radiative heat transfer are specified.");
  return params;
}

template <bool is_ad>
RadiativeHeatFluxBCBaseTempl<is_ad>::RadiativeHeatFluxBCBaseTempl(
    const InputParameters & parameters)
  : GenericIntegratedBC<is_ad>(parameters),
    _sigma_stefan_boltzmann(this->template getParam<Real>("stefan_boltzmann_constant")),
    _tinf(getFunction("Tinfinity")),
    _eps_boundary(this->template getParam<Real>("boundary_emissivity"))
{
}

template <bool is_ad>
GenericReal<is_ad>
RadiativeHeatFluxBCBaseTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> T4 = MathUtils::pow(_u[_qp], 4);
  GenericReal<is_ad> T4inf = MathUtils::pow(_tinf.value(_t, _q_point[_qp]), 4);
  return _test[_i][_qp] * _sigma_stefan_boltzmann * coefficient() * (T4 - T4inf);
}

template <>
Real
RadiativeHeatFluxBCBaseTempl<false>::computeQpJacobian()
{
  Real T3 = MathUtils::pow(_u[_qp], 3);
  return 4 * _sigma_stefan_boltzmann * _test[_i][_qp] * coefficient() * T3 * _phi[_j][_qp];
}

template <>
Real
RadiativeHeatFluxBCBaseTempl<true>::computeQpJacobian()
{
  return 0;
}

template class RadiativeHeatFluxBCBaseTempl<false>;
template class RadiativeHeatFluxBCBaseTempl<true>;
