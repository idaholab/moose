//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSKernel.h"
#include "NS.h"
#include "SubProblem.h"

namespace nms = NS;

defineADValidParams(
    CNSKernel,
    ADKernel,
    params.addParam<bool>("single_equation_SUPG", false,
      "whether to include the single-equation form of the SUPG stabilization for this kernel");
    params.addClassDescription("Base kernel for all fluid conservation equation kernels to quickly toggle "
      "the single-equation Streamline Upwind Petrov Galerkin stabilization."););

CNSKernel::CNSKernel(const InputParameters & parameters)
  : ADKernel(parameters),
    _rz_coord(_subproblem.getAxisymmetricRadialCoord()),
    _single_eqn_supg(getParam<bool>("single_equation_SUPG")),
    _tau(_single_eqn_supg ? &getADMaterialProperty<RealVectorValue>(nms::vector_tau) : nullptr)
{
  if (_single_eqn_supg && hasMaterialPropertyByName<DenseMatrix<Real>>(nms::matrix_tau))
    mooseError("The full-equation SUPG stabilization should not be applied in conjunction with "
      "the single-equation SUPG stabilization! Please disable the 'MatrixTau' material.");
}

ADReal
CNSKernel::computeQpResidual()
{
  ADReal r = weakResidual();

  if (_single_eqn_supg)
    r += (*_tau)[_qp] * strongResidual() * _grad_test[_i][_qp];

  return r;
}
