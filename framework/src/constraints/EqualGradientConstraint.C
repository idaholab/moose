/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "EqualGradientConstraint.h"
#include "SubProblem.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<EqualGradientConstraint>()
{
  InputParameters params = validParams<FaceFaceConstraint>();
  params.addRequiredParam<unsigned int>("component", "Gradient component to constrain");
  return params;
}

EqualGradientConstraint::EqualGradientConstraint(const InputParameters & parameters)
  : FaceFaceConstraint(parameters), _component(getParam<unsigned int>("component"))
{
}

Real
EqualGradientConstraint::computeQpResidual()
{
  return (_grad_u_master[_qp](_component) - _grad_u_slave[_qp](_component)) * _test[_i][_qp];
}

Real
EqualGradientConstraint::computeQpResidualSide(Moose::ConstraintType res_type)
{
  switch (res_type)
  {
    case Moose::Master:
      return _lambda[_qp] * _grad_test_master[_i][_qp](_component);
    case Moose::Slave:
      return -_lambda[_qp] * _grad_test_slave[_i][_qp](_component);
    default:
      return 0;
  }
}

Real
EqualGradientConstraint::computeQpJacobianSide(Moose::ConstraintJacobianType jac_type)
{
  switch (jac_type)
  {
    case Moose::MasterMaster:
    case Moose::SlaveMaster:
      return _phi[_j][_qp] * _grad_test_master[_i][_qp](_component);

    case Moose::MasterSlave:
    case Moose::SlaveSlave:
      return -_phi[_j][_qp] * _grad_test_slave[_i][_qp](_component);

    default:
      return 0;
  }
}
