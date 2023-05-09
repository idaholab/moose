//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarLagrangeMultiplier.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", ScalarLagrangeMultiplier);

InputParameters
ScalarLagrangeMultiplier::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("This class is used to enforce integral of phi = V_0 with a "
                             "Lagrange multiplier approach.");
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier variable");

  return params;
}

ScalarLagrangeMultiplier::ScalarLagrangeMultiplier(const InputParameters & parameters)
  : Kernel(parameters), _lambda_var(coupledScalar("lambda")), _lambda(coupledScalarValue("lambda"))
{
}

ScalarLagrangeMultiplier::~ScalarLagrangeMultiplier() {}

Real
ScalarLagrangeMultiplier::computeQpResidual()
{
  return _lambda[0] * _test[_i][_qp];
}

void
ScalarLagrangeMultiplier::computeOffDiagJacobianScalar(unsigned int jvar)
{
  // Note: Here we are assembling the contributions for _both_ Eq. (9) and (10)
  // in the detailed writeup on this problem [0]. This is important to highlight
  // because it is slightly different from the way things usually work in MOOSE
  // because we are computing Jacobian contributions _for a different equation_
  // than the equation which this Kernel is assigned in the input file. We do
  // this for both simplicity (results in slightly less code) and efficiency:
  // the contribution is symmetric, so it can be computed once and used twice.
  //
  // [0]: https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf

  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);

  prepareMatrixTag(_assembly, _var.number(), jvar);
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jv.order(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobianScalar(jvar);
  accumulateTaggedLocalMatrix();

  const auto ke_copy = _local_ke;
  prepareMatrixTag(_assembly, jvar, _var.number());
  ke_copy.get_transpose(_local_ke);
  accumulateTaggedLocalMatrix();
}

Real
ScalarLagrangeMultiplier::computeQpOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _lambda_var)
    return _test[_i][_qp];
  else
    return 0.;
}
