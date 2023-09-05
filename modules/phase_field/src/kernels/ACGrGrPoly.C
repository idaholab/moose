//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACGrGrPoly.h"

registerMooseObject("PhaseFieldApp", ACGrGrPoly);

InputParameters
ACGrGrPoly::validParams()
{
  InputParameters params = ACGrGrBase::validParams();
  params.addClassDescription("Grain-Boundary model poly-crystalline interface Allen-Cahn Kernel");
  return params;
}

ACGrGrPoly::ACGrGrPoly(const InputParameters & parameters)
  : ACGrGrBase(parameters), _gamma(getMaterialProperty<Real>("gamma_asymm"))
{
}

Real
ACGrGrPoly::assignThisOp()
{
  return _u[_qp];
}

std::vector<Real>
ACGrGrPoly::assignOtherOps()
{
  std::vector<Real> other_ops(_op_num);
  for (unsigned int i = 0; i < _op_num; ++i)
    other_ops[i] = (*_vals[i])[_qp];

  return other_ops;
}

Real
ACGrGrPoly::computeDFDOP(PFFunctionType type)
{
  // assign op and other_ops
  Real op = assignThisOp();
  std::vector<Real> other_ops(_op_num);
  other_ops = assignOtherOps();

  // Sum all other order parameters
  Real SumOPj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumOPj += other_ops[i] * other_ops[i];

  // Calculate either the residual or Jacobian of the grain growth free energy
  switch (type)
  {
    case Residual:
    {
      return _mu[_qp] * (op * op * op - op + 2.0 * _gamma[_qp] * op * SumOPj);
    }

    case Jacobian:
    {
      return _mu[_qp] * (_phi[_j][_qp] * (3.0 * op * op - 1.0 + 2.0 * _gamma[_qp] * SumOPj));
    }

    default:
      mooseError("Invalid type passed in");
  }
}

Real
ACGrGrPoly::computeQpOffDiagJacobian(unsigned int jvar)
{
  // assign op and other_ops
  Real op = assignThisOp();
  std::vector<Real> other_ops(_op_num);
  other_ops = assignOtherOps();

  for (unsigned int i = 0; i < _op_num; ++i)
    if (jvar == _vals_var[i])
    {
      // Derivative of Sumopj
      const Real dSumOPj = 2.0 * other_ops[i] * _phi[_j][_qp];
      const Real dDFDOP = _mu[_qp] * 2.0 * _gamma[_qp] * op * dSumOPj;

      return _L[_qp] * _test[_i][_qp] * dDFDOP;
    }

  return 0.0;
}
