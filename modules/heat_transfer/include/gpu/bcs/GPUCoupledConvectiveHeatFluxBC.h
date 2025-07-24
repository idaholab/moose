//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUIntegratedBC.h"

/**
 * Boundary condition for convective heat flux where temperature and heat transfer coefficient are
 * given by auxiliary variables.  Typically used in multi-app coupling scenario. It is possible to
 * couple in a vector variable where each entry corresponds to a "phase".
 */
class KokkosCoupledConvectiveHeatFluxBC final
  : public Moose::Kokkos::IntegratedBC<KokkosCoupledConvectiveHeatFluxBC>
{
public:
  static InputParameters validParams();

  KokkosCoupledConvectiveHeatFluxBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
  {
    Real q = 0;
    Real u = _u(datum, qp);
    for (unsigned int c = 0; c < _n_components; c++)
      q += _alpha(datum, qp, c) * _htc(datum, qp, c) * (u - _T_infinity(datum, qp, c));
    return _test(datum, i, qp) * q * _scale_factor(datum, qp);
  }
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
  {
    Real dq = 0;
    Real phi = _phi(datum, j, qp);
    for (unsigned int c = 0; c < _n_components; c++)
      dq += _alpha(datum, qp, c) * _htc(datum, qp, c) * phi;
    return _test(datum, i, qp) * dq * _scale_factor(datum, qp);
  }

private:
  /// The number of components
  const unsigned int _n_components;
  /// Far-field temperature fields for each component
  Moose::Kokkos::VariableValue _T_infinity;
  /// Convective heat transfer coefficient
  Moose::Kokkos::VariableValue _htc;
  /// Volume fraction of individual phase
  Moose::Kokkos::VariableValue _alpha;
  /// Scale factor
  Moose::Kokkos::VariableValue _scale_factor;
};
