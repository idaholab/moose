//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBC.h"

/**
 * Boundary condition for radiative heat flux where temperature and the
 * temperature of a body in radiative heat transfer are specified.
 */
template <typename RadiativeHeatFluxBC>
class KokkosRadiativeHeatFluxBCBase : public Moose::Kokkos::IntegratedBC
{
public:
  static InputParameters validParams();

  KokkosRadiativeHeatFluxBCBase(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const;

protected:
  /// Stefan-Boltzmann constant
  const Real _sigma_stefan_boltzmann;

  /// The temperature of the body irhs
  const Real _tinf;
};

template <typename RadiativeHeatFluxBC>
InputParameters
KokkosRadiativeHeatFluxBCBase<RadiativeHeatFluxBC>::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addParam<Real>("stefan_boltzmann_constant", 5.670367e-8, "The Stefan-Boltzmann constant.");
  params.addParam<Real>("Tinfinity", 0, "Temperature of the body in radiative heat transfer.");
  params.addClassDescription("Boundary condition for radiative heat flux where temperature and the"
                             "temperature of a body in radiative heat transfer are specified.");
  return params;
}

template <typename RadiativeHeatFluxBC>
KokkosRadiativeHeatFluxBCBase<RadiativeHeatFluxBC>::KokkosRadiativeHeatFluxBCBase(
    const InputParameters & parameters)
  : IntegratedBC(parameters),
    _sigma_stefan_boltzmann(getParam<Real>("stefan_boltzmann_constant")),
    _tinf(getParam<Real>("Tinfinity"))
{
}

template <typename RadiativeHeatFluxBC>
KOKKOS_FUNCTION Real
KokkosRadiativeHeatFluxBCBase<RadiativeHeatFluxBC>::computeQpResidual(const unsigned int i,
                                                                      const unsigned int qp,
                                                                      ResidualDatum & datum) const
{
  auto bc = static_cast<const RadiativeHeatFluxBC *>(this);

  Real T = _u(datum, qp);
  Real T4 = T * T * T * T;
  Real T4inf = _tinf * _tinf * _tinf * _tinf;
  return _test(datum, i, qp) * _sigma_stefan_boltzmann * bc->coefficient() * (T4 - T4inf);
}

template <typename RadiativeHeatFluxBC>
KOKKOS_FUNCTION Real
KokkosRadiativeHeatFluxBCBase<RadiativeHeatFluxBC>::computeQpJacobian(const unsigned int i,
                                                                      const unsigned int j,
                                                                      const unsigned int qp,
                                                                      ResidualDatum & datum) const
{
  auto bc = static_cast<const RadiativeHeatFluxBC *>(this);

  Real T = _u(datum, qp);
  Real T3 = T * T * T;
  return 4 * _sigma_stefan_boltzmann * _test(datum, i, qp) * bc->coefficient() * T3 *
         _phi(datum, j, qp);
}
