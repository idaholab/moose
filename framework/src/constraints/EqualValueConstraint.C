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
EqualValueConstraint::computeQpResidualSide(Moose::ConstraintSideType side)
{
  switch (side)
  {
  case Moose::SIDE_MASTER: return  _lambda[_qp] * _test_master[_i][_qp];
  case Moose::SIDE_SLAVE:  return -_lambda[_qp] * _test_slave[_i][_qp];
  default: return 0;
  }
}

Real
EqualValueConstraint::computeQpJacobianSide(Moose::ConstraintSideType side)
{
  switch (side)
  {
  case Moose::SIDE_MASTER: return  _phi[_j][_qp] * _test_master[_i][_qp];
  case Moose::SIDE_SLAVE:  return -_phi[_j][_qp] * _test_slave[_i][_qp];
  default: return 0;
  }
}
