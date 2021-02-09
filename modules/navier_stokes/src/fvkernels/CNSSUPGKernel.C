//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSSUPGKernel.h"
#include "NS.h"
#include "MooseMesh.h"
#include "Assembly.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", CNSSUPGKernel);

defineADValidParams(
    CNSSUPGKernel,
    ADKernel,
    params.addRequiredCoupledVar(nms::porosity, "porosity");
    params.addRequiredRangeCheckedParam<unsigned int>(
        "var_num",
        "var_num < 6",
        "index of the equation (0 = continuity, 1..ns = momentum, ns + 1 = energy) "
        "we are currently solving for.");
    params.addClassDescription("Streamline Upwind Petrov Galerkin stabilization kernel for the "
      "Navier-Stokes and Euler model fluid conservation equations."););

CNSSUPGKernel::CNSSUPGKernel(const InputParameters & parameters)
  : ADKernel(parameters),
    _mesh_dimension(KernelBase::_mesh.dimension()),
    _n_vars(_mesh_dimension + 2),
    _var_num(getParam<unsigned int>("var_num")),
    _eps(coupledValue(nms::porosity)),
    _A(getADMaterialProperty<std::vector<DenseMatrix<Real>>>(nms::A)),
    _Tau(getADMaterialProperty<DenseMatrix<Real>>(nms::matrix_tau)),
    _R(getADMaterialProperty<DenseVector<Real>>(nms::R)),
    _res_tmp(_n_vars)
{
}

ADReal
CNSSUPGKernel::computeQpResidual()
{
  // The code below is an optimized version of the following algo:
  //
  //    ADReal r = 0;
  //    for (unsigned int d = 0; d < _mesh_dimension; ++d)
  //      for (unsigned int a = 0; a < _n_vars; ++a)
  //        for (unsigned int b = 0; b < _n_vars; ++b)
  //          r += _eps[_qp] * _grad_test[_i][_qp](d) * _A[_qp][d](_var_num, a) * _Tau[_qp](a,b) * _R[_qp](b);
  // This condition computes non _i dependent values only when _i resets to
  // zero - assuming _i loop is inside the _qp loop.  This way we can reuse
  // qp-dependent values across multiple iterations of the _i loop.  This is
  // dangerous - what if MOOSE starts iterating over _i values be walking in
  // reverse down to zero? or swaps nesting of _qp and _i loops?  ADKernelGrad
  // could work but does not allow us to pull the ...*eps[_qp] multiplication
  // outside the dot product inside ADKernelGrad between precompute's return
  // value and grad_test[i][qp].  We could write our own qp, i loops in
  // precalculateResidual, but then we would need to store separately indexed
  // precalculated values for each qp and i and then index into and return
  // them in the main ADKernel's loops - which would also be ugly and likely
  // slower.
  if (_i == 0)
  {
    // this assumes Tau is a diagonal matrix
    for (unsigned int a = 0; a < _n_vars; ++a)
    {
      _res_tmp[a] = _Tau[_qp](a, a);
      _res_tmp[a] *= _R[_qp](a);
    }

    for (unsigned int d = 0; d < _mesh_dimension; ++d)
    {
      _supg_mult[d] = 0;
      for (unsigned int a = 0; a < _n_vars; ++a)
      {
        _tmp = _res_tmp[a];
        _tmp *= _A[_qp][d](_var_num, a);
        _supg_mult[d] += _tmp;
      }
    }
  }

  _r = 0;
  for (unsigned int d = 0; d < _mesh_dimension; ++d)
  {
    _tmp = _supg_mult[d];
    _tmp *= _grad_test[_i][_qp](d);
    _r += _tmp;
  }
  _r *= _eps[_qp];
  return _r;
}
