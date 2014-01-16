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

#include "EqualValueConstraint.h"
#include "SubProblem.h"
#include "FEProblem.h"

template<>
InputParameters validParams<EqualValueConstraint>()
{
  InputParameters params = validParams<FaceFaceConstraint>();
  return params;
}

EqualValueConstraint::EqualValueConstraint(const std::string & name, InputParameters parameters) :
    FaceFaceConstraint(name, parameters)
{
}

EqualValueConstraint::~EqualValueConstraint()
{
}

Real
EqualValueConstraint::computeQpResidual()
{
  return (_u_master[_qp] - _u_slave[_qp]) * _test[_i][_qp];
}

Real
EqualValueConstraint::computeQpResidualSide(Moose::ConstraintType res_type)
{
  switch (res_type)
  {
  case Moose::Master: return  _lambda[_qp] * _test_master[_i][_qp];
  case Moose::Slave:  return -_lambda[_qp] * _test_slave[_i][_qp];
  default: return 0;
  }
}

Real
EqualValueConstraint::computeQpJacobianSide(Moose::ConstraintJacobianType jac_type)
{
  switch (jac_type)
  {
  case Moose::MasterMaster:
  case Moose::SlaveMaster:
    return  _phi[_j][_qp] * _test_master[_i][_qp];

  case Moose::MasterSlave:
  case Moose::SlaveSlave:
    return -_phi[_j][_qp] * _test_slave[_i][_qp];

  default:
    return 0;
  }
}
