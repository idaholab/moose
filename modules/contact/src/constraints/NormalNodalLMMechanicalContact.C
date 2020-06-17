//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalNodalLMMechanicalContact.h"
#include "PenetrationLocator.h"
#include "PenetrationInfo.h"
#include "SystemBase.h"
#include "Assembly.h"

#include "DualRealOps.h"

using MetaPhysicL::DualNumber;

registerMooseObject("MooseApp", NormalNodalLMMechanicalContact);

InputParameters
NormalNodalLMMechanicalContact::validParams()
{
  InputParameters params = NodeFaceConstraint::validParams();
  params.set<bool>("use_displaced_mesh") = true;

  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<Real>("c", 1, "Parameter for balancing the size of the gap and contact pressure");

  MooseEnum ncp_function_type("min fb", "min");
  params.addParam<MooseEnum>(
      "ncp_function_type", ncp_function_type, "The type of NCP function to use");

  params.addClassDescription("Implements the KKT conditions for normal contact using an NCP "
                             "function. Requires that either the gap distance or the normal "
                             "contact pressure (represented by the value of `variable`) is zero. "
                             "The LM variable must be of the same order as the mesh");
  return params;
}

NormalNodalLMMechanicalContact::NormalNodalLMMechanicalContact(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _disp_y_id(coupled("disp_y")),
    _disp_z_id(coupled("disp_z")),
    _c(getParam<Real>("c")),
    _epsilon(std::numeric_limits<Real>::epsilon()),
    _ncp_type(getParam<MooseEnum>("ncp_function_type")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _displaced_problem(_fe_problem.getDisplacedProblem().get())
{
  _overwrite_secondary_residual = false;

  // Make sure interpolation orders are consistent
  const auto & disp_names = _displaced_problem->getDisplacementVarNames();

  const auto disp_type = _sys.system().variable_type(disp_names[0]);
  const auto lm_type = _sys.system().variable_type(_var.name());

  if (disp_type.order.get_order() != lm_type.order.get_order())
    paramError(_var.name(),
               "The mechanical contact Lagrange multiplier has to be the same order as the mesh. "
               "NormalNodalLMMechanicalContact is applied pointwise at every secondary node.");
}

Real
NormalNodalLMMechanicalContact::computeQpSecondaryValue()
{
  return _u_secondary[_qp];
}

void
NormalNodalLMMechanicalContact::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  _qp = 0;

  re(0) = computeQpResidual(Moose::Secondary);
}

void
NormalNodalLMMechanicalContact::computeJacobian()
{
  _Kee.resize(1, 1);
  _connected_dof_indices.clear();
  _connected_dof_indices.push_back(_var.nodalDofIndex());

  _qp = 0;

  _Kee(0, 0) += computeQpJacobian(Moose::SecondarySecondary);
}

void
NormalNodalLMMechanicalContact::computeOffDiagJacobian(unsigned jvar)
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

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar);

  for (_j = 0; _j < _phi_primary.size(); ++_j)
    Ken(0, _j) += computeQpOffDiagJacobian(Moose::SecondaryPrimary, jvar);
}

Real NormalNodalLMMechanicalContact::computeQpResidual(Moose::ConstraintType /*type*/)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      Real a = -pinfo->_distance * _c;
      mooseAssert(
          _qp < _u_secondary.size(),
          "qp is greater than the size of u_secondary in NormalNodalLMMechanicalContact. Check and "
          "make sure that your Lagrange multiplier variable has the same order as the mesh");
      Real b = _u_secondary[_qp];

      if (_ncp_type == "fb")
        return a + b - std::sqrt(a * a + b * b + _epsilon);
      else
        return std::min(a, b);
    }
  }
  return _u_secondary[_qp];
}

// Note that the Jacobians below are inexact. To really make them exact, the most algorithmically
// rigorous way will be to accomplish libmesh/libmesh#2121

Real NormalNodalLMMechanicalContact::computeQpJacobian(Moose::ConstraintJacobianType /*type*/)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      DualNumber<Real, Real> dual_u_secondary(_u_secondary[_qp]);
      dual_u_secondary.derivatives() = 1.;

      auto a = -pinfo->_distance * _c;
      auto b = dual_u_secondary;

      if (_ncp_type == "fb")
        return (a + b - std::sqrt(a * a + b * b + _epsilon)).derivatives();
      else
        return std::min(a, b).derivatives();
    }
  }
  return 1;
}

Real
NormalNodalLMMechanicalContact::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                         unsigned jvar)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      DualNumber<Real, Real> gap(-pinfo->_distance);

      unsigned comp;
      if (jvar == _primary_var_num)
        comp = 0;
      else if (jvar == _disp_y_id)
        comp = 1;
      else if (jvar == _disp_z_id)
        comp = 2;
      else
        return 0;

      gap.derivatives() = pinfo->_normal(comp);

      switch (type)
      {
        case Moose::SecondarySecondary:
          gap.derivatives() *= 1;
          break;
        case Moose::SecondaryPrimary:
          gap.derivatives() *= -_phi_primary[_j][_qp];
          break;
        default:
          mooseError("LMs do not have a primary contribution.");
      }

      auto a = gap * _c;
      auto b = _u_secondary[_qp];

      if (_ncp_type == "fb")
        return (a + b - std::sqrt(a * a + b * b + _epsilon)).derivatives();
      else
        return std::min(a, b).derivatives();
    }
  }
  return 0.;
}
