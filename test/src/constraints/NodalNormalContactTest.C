//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNormalContactTest.h"
#include "PenetrationLocator.h"
#include "PenetrationInfo.h"
#include "SystemBase.h"
#include "Assembly.h"

registerMooseObject("MooseTestApp", NodalNormalContactTest);

template <>
InputParameters
validParams<NodalNormalContactTest>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  params.set<bool>("use_displaced_mesh") = true;

  params.addRequiredCoupledVar("lambda", "The normal lagrange multiplier");
  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");
  return params;
}

NodalNormalContactTest::NodalNormalContactTest(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _lambda(getVar("lambda", 0)->nodalValue()),
    _lambda_id(coupled("lambda")),
    _epsilon(std::numeric_limits<Real>::epsilon()),
    _component(getParam<MooseEnum>("component"))
{
  _overwrite_slave_residual = false;
}

Real
NodalNormalContactTest::computeQpSlaveValue()
{
  return _u_slave[_qp];
}

Real
NodalNormalContactTest::computeQpResidual(Moose::ConstraintType type)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo)
    {
      switch (type)
      {
        case Moose::ConstraintType::Slave:
          // This normal appears to point in the opposite direction of the slave surface so we need
          // a negative sign
          return _lambda * -pinfo->_normal(_component);

        case Moose::ConstraintType::Master:
          return _test_master[_i][_qp] * _lambda * pinfo->_normal(_component);

        default:
          return 0;
      }
    }
  }
  return 0;
}

Real NodalNormalContactTest::computeQpJacobian(Moose::ConstraintJacobianType /*type*/) { return 0; }

Real
NodalNormalContactTest::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type, unsigned jvar)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo && jvar == _lambda_id)
    {
      switch (type)
      {
        case Moose::SlaveSlave:
          return -pinfo->_normal(_component);
        case Moose::MasterSlave:
          return _test_master[_i][_qp] * pinfo->_normal(_component);
        default:
          return 0;
      }
    }
  }

  return 0;
}
