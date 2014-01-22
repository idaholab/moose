#include "RichardsPConstraint.h"

#include <iostream>


template<>
InputParameters validParams<RichardsPConstraint>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<Real>("a", 1.0E-20, "Weight of the constraint.  Care should be taken with this parameter choice as otherwise the contribution will dominate all the Richards physics!");
  params.addRequiredCoupledVar("lower_var", "Your porepressure variable will be constrainted to be greater than this lower_var variable.");
  params.addClassDescription("This adds a term to the residual that attempts to enforce porepressure > lower_var.  The term is a*(p - lower)^2 for p<lower, and zero otherwise");
  return params;
}

RichardsPConstraint::RichardsPConstraint(const std::string & name,
                                             InputParameters parameters) :
    Kernel(name,parameters),
    _a(getParam<Real>("a")),
    _lower(coupledValue("lower_var")),
    _lower_var_num(coupled("lower_var"))

{}


Real
RichardsPConstraint::computeQpResidual()
{
  if (_u[_qp] < _lower[_qp])
    {
      return _test[_i][_qp]*_a*std::pow(_u[_qp] - _lower[_qp], 2);
    }
  return 0.0;
}

Real
RichardsPConstraint::computeQpJacobian()
{
  if (_u[_qp] < _lower[_qp])
    {
      return _test[_i][_qp]*2*_a*(_u[_qp] - _lower[_qp])*_phi[_j][_qp];
    }
  return 0.0;
}

Real
RichardsPConstraint::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _lower_var_num && _u[_qp] < _lower[_qp])
    {
      return -_test[_i][_qp]*2*_a*(_u[_qp] - _lower[_qp])*_phi[_j][_qp];
    }
  return 0.0;
}


