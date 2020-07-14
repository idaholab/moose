//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatrixTau.h"
#include "NS.h"

#include "libmesh/dense_matrix_impl.h"
#include "libmesh/dense_matrix_base_impl.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", MatrixTau);

defineADValidParams(MatrixTau, TauMaterial, );

MatrixTau::MatrixTau(const InputParameters & parameters)
  : TauMaterial(parameters),
    _Tau(declareADProperty<DenseMatrix<Real>>(nms::matrix_tau))
{
}

void
MatrixTau::computeTau()
{
  _Tau[_qp].resize(_N, _N);
  _Tau[_qp].zero();

  // mass equation
  _Tau[_qp](0, 0) = _mass_tau;

  // momentum equation(s)
  for (unsigned int i = 1; i < _N - 1; ++i)
    _Tau[_qp](i, i) = _momentum_tau;

  // energy equation
  _Tau[_qp](_N - 1, _N - 1) = _energy_tau;
}
