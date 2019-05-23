//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNormalContactLM.h"
#include "PenetrationLocator.h"
#include "PenetrationInfo.h"
#include "SystemBase.h"
#include "Assembly.h"

registerMooseObject("ContactApp", NodalNormalContactLM);

template <>
InputParameters
validParams<NodalNormalContactLM>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  params.set<bool>("use_displaced_mesh") = true;

  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  params.addClassDescription("Implements the KKT conditions for normal contact using an NCP "
                             "function. Requires that either the gap distance or the normal "
                             "contact pressure (represented by the value of `variable`) is zero.");
  params.addParam<Real>("c",
                        1,
                        "The algorithmic scaling parameter that can be used to bring the LM "
                        "variable and gap distance to the same order of magnitude");
  return params;
}

NodalNormalContactLM::NodalNormalContactLM(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _disp_y_id(coupled("disp_y")),
    _disp_z_id(coupled("disp_z")),
    _epsilon(std::numeric_limits<Real>::epsilon()),
    _c(getParam<Real>("c"))
{
  _overwrite_slave_residual = false;
}

Real
NodalNormalContactLM::computeQpSlaveValue()
{
  return _u_slave[_qp];
}

void
NodalNormalContactLM::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  _qp = 0;

  re(0) = computeQpResidual(Moose::Slave);
}

void
NodalNormalContactLM::computeJacobian()
{
  _Kee.resize(1, 1);
  _connected_dof_indices.clear();
  _connected_dof_indices.push_back(_var.nodalDofIndex());

  _qp = 0;

  _Kee(0, 0) += computeQpJacobian(Moose::SlaveSlave);
}

void
NodalNormalContactLM::computeOffDiagJacobian(unsigned jvar)
{
  if (jvar == _var.number())
  {
    computeJacobian();
    return;
  }

  MooseVariableFEBase & var = _sys.getVariable(0, jvar);
  _connected_dof_indices.clear();
  _connected_dof_indices.push_back(var.nodalDofIndex());

  _qp = 0;

  _Kee.resize(1, 1);
  _Kee(0, 0) += computeQpOffDiagJacobian(Moose::SlaveSlave, jvar);

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar);

  for (_j = 0; _j < _phi_master.size(); ++_j)
    Ken(0, _j) += computeQpOffDiagJacobian(Moose::SlaveMaster, jvar);
}

Real NodalNormalContactLM::computeQpResidual(Moose::ConstraintType /*type*/)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      Real a = -pinfo->_distance;
      Real b = _u_slave[_qp];
      return a + b - std::sqrt(a * a + b * b);
      // return std::min(a * _c, b);
    }
  }
  return 0;
}

Real NodalNormalContactLM::computeQpJacobian(Moose::ConstraintJacobianType /*type*/)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      Real a = -pinfo->_distance;
      Real b = _u_slave[_qp];
      // if (b > a * _c)
      //   return 0.;
      // else
      //   return 1.;
      if (std::abs(a) < _epsilon && std::abs(b) < _epsilon)
        return 0.;
      else
        return 1. - b / std::sqrt(a * a + b * b);
    }
  }
  return 0;
}

Real
NodalNormalContactLM::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type, unsigned jvar)
{
  // TODO: Make Jacobian generally applicable to higher order variables. This current Jacobian
  // implementation improves preconditioning when first order Lagranges are used, but actually
  // can degrade preconditioner performance when higher order variables are used.
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      Real a = -pinfo->_distance;
      Real b = _u_slave[_qp];

      unsigned comp;
      if (jvar == _master_var_num)
        comp = 0;
      else if (jvar == _disp_y_id)
        comp = 1;
      else if (jvar == _disp_z_id)
        comp = 2;
      else
        return 0;

      Real da_daj = pinfo->_normal(comp);

      switch (type)
      {
        case Moose::SlaveSlave:
          da_daj *= 1;
          break;
        case Moose::SlaveMaster:
          da_daj *= -_phi_master[_j][_qp];
          break;
        default:
          mooseError("LMs do not have a master contribution.");
      }

      // if (b > a * _c)
      //   return da_daj;
      // else
      //   return 0;
      if (std::abs(b) < _epsilon && std::abs(a) < _epsilon)
      {
        Real sign_a = da_daj > 0 ? 1 : -1;
        return da_daj * (1. - sign_a);
      }
      else
        return da_daj - a / std::sqrt(a * a + b * b) * da_daj;
    }
  }
  return 0.;
}
