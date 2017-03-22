/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "OneDContactConstraint.h"

#include "SystemBase.h"
#include "PenetrationLocator.h"

// libMesh includes
#include "libmesh/string_to_enum.h"
#include "libmesh/sparse_matrix.h"

template <>
InputParameters
validParams<OneDContactConstraint>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<bool>("jacobian_update",
                        false,
                        "Whether or not to update the 'in contact' list "
                        "every jacobian evaluation (by default it will "
                        "happen once per timestep");
  return params;
}

OneDContactConstraint::OneDContactConstraint(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _residual_copy(_sys.residualGhosted()),
    _jacobian_update(getParam<bool>("jacobian_update"))
{
}

void
OneDContactConstraint::timestepSetup()
{
  updateContactSet();
}

void
OneDContactConstraint::jacobianSetup()
{
  if (_jacobian_update)
    updateContactSet();
}

void
OneDContactConstraint::updateContactSet()
{
  std::set<dof_id_type> & has_penetrated = _penetration_locator._has_penetrated;

  std::map<dof_id_type, PenetrationInfo *>::iterator
      it = _penetration_locator._penetration_info.begin(),
      end = _penetration_locator._penetration_info.end();

  for (; it != end; ++it)
  {
    PenetrationInfo * pinfo = it->second;

    // Skip this pinfo if there are no DOFs on this node.
    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _var.number()) < 1)
      continue;

    if (pinfo->_distance > 0)
    {
      dof_id_type slave_node_num = it->first;
      has_penetrated.insert(slave_node_num);
    }
  }
}

bool
OneDContactConstraint::shouldApply()
{
  std::set<dof_id_type>::iterator hpit =
      _penetration_locator._has_penetrated.find(_current_node->id());
  return (hpit != _penetration_locator._has_penetrated.end());
}

Real
OneDContactConstraint::computeQpSlaveValue()
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  Moose::err << std::endl
             << "Popping out node: " << _current_node->id() << std::endl
             << "Closest Point x: " << pinfo->_closest_point(0) << std::endl
             << "Current Node x: " << (*_current_node)(0) << std::endl
             << "Current Value: " << _u_slave[_qp] << std::endl
             << std::endl;

  return pinfo->_closest_point(0) - ((*_current_node)(0) - _u_slave[_qp]);
}

Real
OneDContactConstraint::computeQpResidual(Moose::ConstraintType type)
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  switch (type)
  {
    case Moose::Slave:
      // return (_u_slave[_qp] - _u_master[_qp])*_test_slave[_i][_qp];
      return ((*_current_node)(0) - pinfo->_closest_point(0)) * _test_slave[_i][_qp];
    case Moose::Master:
      double slave_resid = _residual_copy(_current_node->dof_number(0, _var.number(), 0));
      return slave_resid * _test_master[_i][_qp];
  }
  return 0;
}

Real
OneDContactConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  double slave_jac = 0;
  switch (type)
  {
    case Moose::SlaveSlave:
      return _phi_slave[_j][_qp] * _test_slave[_i][_qp];
    case Moose::SlaveMaster:
      return -_phi_master[_j][_qp] * _test_slave[_i][_qp];
    case Moose::MasterSlave:
      slave_jac =
          (*_jacobian)(_current_node->dof_number(0, _var.number(), 0), _connected_dof_indices[_j]);
      return slave_jac * _test_master[_i][_qp];
    case Moose::MasterMaster:
      return 0;
  }
  return 0;
}
