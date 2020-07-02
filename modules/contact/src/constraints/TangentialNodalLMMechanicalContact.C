//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TangentialNodalLMMechanicalContact.h"
#include "PenetrationLocator.h"
#include "PenetrationInfo.h"
#include "SystemBase.h"
#include "Assembly.h"

#include "DualRealOps.h"

registerMooseObject("ContactApp", TangentialNodalLMMechanicalContact);

InputParameters
TangentialNodalLMMechanicalContact::validParams()
{
  InputParameters params = NodeFaceConstraint::validParams();
  params.set<bool>("use_displaced_mesh") = true;

  params.addRequiredCoupledVar(
      "contact_pressure",
      "The contact pressure. If using LM, this should be the normal lagrange multiplier");
  params.addRequiredCoupledVar("disp_y", "The y velocity");
  params.addRequiredParam<Real>("mu", "The coefficient of friction.");

  MooseEnum ncp_function_type("min fb", "min");
  params.addParam<MooseEnum>(
      "ncp_function_type", ncp_function_type, "The type of NCP function to use");

  params.addRequiredParam<Real>("mu", "The friction coefficient for the Coulomb friction law");

  params.addParam<Real>(
      "k_abs",
      10,
      "The smoothing parameter for the function used to approximate std::abs. The approximating "
      "function is courtesy of https://math.stackexchange.com/a/1115033/408963");

  params.addClassDescription("Implements the KKT conditions for frictional Coulomb contact using "
                             "an NCP function. Requires that either the relative tangential "
                             "velocity is zero or the tangential stress is equal to the friction "
                             "coefficient times the normal contact pressure.");
  return params;
}

TangentialNodalLMMechanicalContact::TangentialNodalLMMechanicalContact(
    const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _contact_pressure(getVar("contact_pressure", 0)->nodalValue()),
    _contact_pressure_id(coupled("contact_pressure")),
    _disp_x_dot(_primary_var.nodalValueDot()),
    _disp_y_dot(getVar("disp_y", 0)->nodalValueDot()),
    _disp_y_id(coupled("disp_y")),
    _du_dot_du(_primary_var.dofValuesDuDotDu()),

    _mu(getParam<Real>("mu")),
    _epsilon(std::numeric_limits<Real>::epsilon()),
    _ncp_type(getParam<MooseEnum>("ncp_function_type"))
{
  _overwrite_secondary_residual = false;
}

Real
TangentialNodalLMMechanicalContact::computeQpSecondaryValue()
{
  return _u_secondary[_qp];
}

void
TangentialNodalLMMechanicalContact::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  _qp = 0;

  re(0) = computeQpResidual(Moose::Secondary);
}

void
TangentialNodalLMMechanicalContact::computeJacobian()
{
  _Kee.resize(1, 1);
  _connected_dof_indices.clear();
  _connected_dof_indices.push_back(_var.nodalDofIndex());

  _qp = 0;

  _Kee(0, 0) += computeQpJacobian(Moose::SecondarySecondary);
}

void
TangentialNodalLMMechanicalContact::computeOffDiagJacobian(unsigned jvar)
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
  _Kee(0, 0) += computeQpOffDiagJacobian(Moose::SecondarySecondary, jvar);
}

Real TangentialNodalLMMechanicalContact::computeQpResidual(Moose::ConstraintType /*type*/)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      if (_contact_pressure < _epsilon)
        return _u_secondary[_qp];
      else
      {
        RealVectorValue tangent_vec(pinfo->_normal(1), -pinfo->_normal(0), 0);
        Real v_dot_tan = RealVectorValue(_disp_x_dot, _disp_y_dot, 0) * tangent_vec;

        // NCP part 1: requirement that either there is no slip **or** slip velocity and
        // frictional force exerted **by** the secondary side are in the same direction
        Real a;
        if (v_dot_tan * _u_secondary[_qp] < 0)
          a = -std::abs(v_dot_tan);
        else
          a = std::abs(v_dot_tan);

        // NCP part 2: require that the frictional force can never exceed the frictional
        // coefficient times the normal force.
        auto b = _mu * _contact_pressure - std::abs(_u_secondary[_qp]);

        if (_ncp_type == "fb")
          return a + b - std::sqrt(a * a + b * b + _epsilon);
        else
          return std::min(a, b);
      }
    }
  }
  return 0;
}

Real TangentialNodalLMMechanicalContact::computeQpJacobian(Moose::ConstraintJacobianType /*type*/)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      if (_contact_pressure < _epsilon)
        return 1.;
      else
      {
        RealVectorValue tangent_vec(pinfo->_normal(1), -pinfo->_normal(0), 0);
        Real v_dot_tan = RealVectorValue(_disp_x_dot, _disp_y_dot, 0) * tangent_vec;

        // NCP part 1: requirement that either there is no slip **or** slip velocity and
        // frictional force exerted **by** the secondary side are in the same direction
        Real a;
        if (v_dot_tan * _u_secondary[_qp] < 0)
          a = -std::abs(v_dot_tan);
        else
          a = std::abs(v_dot_tan);

        // NCP part 2: require that the frictional force can never exceed the frictional
        // coefficient times the normal force.
        DualNumber<Real, Real> dual_u_secondary(_u_secondary[_qp]);
        dual_u_secondary.derivatives() = 1;

        auto b = _mu * _contact_pressure - std::abs(dual_u_secondary);

        if (_ncp_type == "fb")
          return (a + b - std::sqrt(a * a + b * b + _epsilon)).derivatives();
        else
          return std::min(a, b).derivatives();
      }
    }
  }
  return 0;
}

Real
TangentialNodalLMMechanicalContact::computeQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                                             unsigned jvar)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      if (_contact_pressure < _epsilon)
        return 0.;

      // Our local dual number is going to depend on only three degrees of freedom: the secondary
      // nodal dofs for disp_x (index 0), disp_y (index 1), and the contact pressure (index 2). The
      // latter of course exists only on the secondary side
      typedef DualNumber<Real, DNDerivativeSize<3>> LocalDN;

      RealVectorValue tangent_vec(pinfo->_normal(1), -pinfo->_normal(0), 0);

      LocalDN dual_disp_x_dot(_disp_x_dot);
      // We index the zeroth entry of the _du_dot_du member variable because there is only one
      // degree of freedom on the node
      Moose::derivInsert(dual_disp_x_dot.derivatives(), 0, _du_dot_du[0]);

      LocalDN dual_disp_y_dot(_disp_y_dot);
      Moose::derivInsert(dual_disp_y_dot.derivatives(), 1, _du_dot_du[0]);

      LocalDN dual_contact_pressure(_contact_pressure);
      Moose::derivInsert(dual_contact_pressure.derivatives(), 2, 1);

      auto v_dot_tan = VectorValue<LocalDN>(dual_disp_x_dot, dual_disp_y_dot, 0) * tangent_vec;

      // NCP part 1: requirement that either there is no slip **or** slip velocity and
      // frictional force exerted **by** the secondary side are in the same direction
      LocalDN a;
      if (v_dot_tan * _u_secondary[_qp] < 0)
        a = -std::abs(v_dot_tan);
      else
        a = std::abs(v_dot_tan);

      // NCP part 2: require that the frictional force can never exceed the frictional
      // coefficient times the normal force.
      auto b = _mu * dual_contact_pressure - std::abs(_u_secondary[_qp]);

      LocalDN ncp_value;
      if (_ncp_type == "fb")
        ncp_value = a + b - std::sqrt(a * a + b * b + _epsilon);
      else
        ncp_value = std::min(a, b);

      if (jvar == _contact_pressure_id)
        return ncp_value.derivatives()[2];
      else if (jvar == _primary_var.number())
        return ncp_value.derivatives()[0];
      else if (jvar == _disp_y_id)
        return ncp_value.derivatives()[1];
    }
  }
  return 0.;
}
