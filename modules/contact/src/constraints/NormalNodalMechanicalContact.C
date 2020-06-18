//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalNodalMechanicalContact.h"
#include "PenetrationLocator.h"
#include "PenetrationInfo.h"
#include "SystemBase.h"
#include "Assembly.h"

registerMooseObject("ContactApp", NormalNodalMechanicalContact);

InputParameters
NormalNodalMechanicalContact::validParams()
{
  InputParameters params = NodeFaceConstraint::validParams();
  params.addClassDescription(
      "Applies the normal contact force to displacement residuals through a Lagrange Multiplier");
  params.set<bool>("use_displaced_mesh") = true;

  params.addRequiredCoupledVar("lambda", "The normal lagrange multiplier");
  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");
  return params;
}

NormalNodalMechanicalContact::NormalNodalMechanicalContact(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _lambda(getVar("lambda", 0)->nodalValue()),
    _lambda_id(coupled("lambda")),
    _epsilon(std::numeric_limits<Real>::epsilon()),
    _component(getParam<MooseEnum>("component"))
{
  _overwrite_secondary_residual = false;
}

Real
NormalNodalMechanicalContact::computeQpSlaveValue()
{
  return _u_secondary[_qp];
}

void
NormalNodalMechanicalContact::computeJacobian()
{
}

void
NormalNodalMechanicalContact::computeOffDiagJacobian(unsigned jvar)
{
  // Our residual only strongly depends on the lagrange multiplier (the normal vector does indeed
  // depend on displacements but it's complicated and shouldn't be too strong of a dependence)
  if (jvar != _lambda_id)
    return;

  MooseVariableFEBase & var = _sys.getVariable(0, jvar);
  _connected_dof_indices.clear();
  _connected_dof_indices.push_back(var.nodalDofIndex());

  _qp = 0;

  _Kee.resize(1, 1);
  _Kee(0, 0) = computeQpOffDiagJacobian(Moose::SlaveSlave, jvar);

  _Kne.resize(_test_primary.size(), 1);

  for (_i = 0; _i < _test_primary.size(); ++_i)
    _Kne(_i, 0) = computeQpOffDiagJacobian(Moose::MasterSlave, jvar);
}

Real
NormalNodalMechanicalContact::computeQpResidual(Moose::ConstraintType type)
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
          // This normal appears to point in the opposite direction of the secondary surface so we
          // need a negative sign
          return _lambda * -pinfo->_normal(_component);

        case Moose::ConstraintType::Master:
          return _test_primary[_i][_qp] * _lambda * pinfo->_normal(_component);

        default:
          return 0;
      }
    }
  }
  return 0;
}

Real NormalNodalMechanicalContact::computeQpJacobian(Moose::ConstraintJacobianType /*type*/)
{
  return 0;
}

Real
NormalNodalMechanicalContact::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                       unsigned jvar)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;

    // Latter check here is actually redundant because we don't call into this function unless jvar
    // == _lambda_id
    if (pinfo && jvar == _lambda_id)
    {
      switch (type)
      {
        case Moose::SlaveSlave:
          return -pinfo->_normal(_component);
        case Moose::MasterSlave:
          return _test_primary[_i][_qp] * pinfo->_normal(_component);
        default:
          return 0;
      }
    }
  }

  return 0;
}
