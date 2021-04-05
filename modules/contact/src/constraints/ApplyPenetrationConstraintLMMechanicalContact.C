//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ApplyPenetrationConstraintLMMechanicalContact.h"
#include "PenetrationLocator.h"
#include "PenetrationInfo.h"
#include "SystemBase.h"
#include "Assembly.h"

#include "DualRealOps.h"

using MetaPhysicL::DualNumber;

registerMooseObject("ContactApp", ApplyPenetrationConstraintLMMechanicalContact);

InputParameters
ApplyPenetrationConstraintLMMechanicalContact::validParams()
{
  InputParameters params = NodeFaceConstraint::validParams();
  params.set<bool>("use_displaced_mesh") = true;

  params.addParam<Real>(
      "c", 1e6, "Parameter for balancing the size of the gap and contact pressure");

  params.addClassDescription(
      "Implements the KKT conditions for normal contact using an NCP "
      "function, in this case just the min function. This function enforces that either "
      "the gap distance or the normal contact pressure (represented by the value of `variable`) is "
      "zero.");
  return params;
}

ApplyPenetrationConstraintLMMechanicalContact::ApplyPenetrationConstraintLMMechanicalContact(
    const InputParameters & parameters)
  : NodeFaceConstraint(parameters), _c(getParam<Real>("c")), _residual_copy(_sys.residualGhosted())
{
}

bool
ApplyPenetrationConstraintLMMechanicalContact::shouldApply()
{
  const bool has_dofs = _current_node->n_dofs(_sys.number(), _var.number());
  _dof_number = has_dofs ? _current_node->dof_number(_sys.number(), _var.number(), /*comp=*/0)
                         : DofObject::invalid_id;
  return has_dofs;
}

bool
ApplyPenetrationConstraintLMMechanicalContact::overwriteSecondaryResidual()
{
  // If we didn't project the secondary node onto the primary surface, then gap has no meaning and
  // we just want to drive our Lagrange Multiplier to zero. If we did, then our LM drives the
  // residual if it is less than the gap (we're using min for our NCP function)
  return (!_projection_successful) || std::isnan(_node_to_integrated_gap.at(_current_node)) ||
         (_u_secondary[_qp] < _node_to_integrated_gap.at(_current_node));
}

void
ApplyPenetrationConstraintLMMechanicalContact::residualSetup()
{
  NodeFaceConstraint::residualSetup();
  _node_to_integrated_gap.clear();
}

Real
ApplyPenetrationConstraintLMMechanicalContact::computeQpSecondaryValue()
{
  return _u_secondary[_qp];
}

void
ApplyPenetrationConstraintLMMechanicalContact::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  _qp = 0;

  _secondary_residual = re(0) = computeQpResidual(Moose::Secondary);
  _secondary_residual_computed = true;
}

void
ApplyPenetrationConstraintLMMechanicalContact::computeJacobian()
{
  _Kee.resize(1, 1);
  _connected_dof_indices.clear();
  _connected_dof_indices.push_back(_var.nodalDofIndex());

  _qp = 0;

  _Kee(0, 0) += computeQpJacobian(Moose::SecondarySecondary);
}

void
ApplyPenetrationConstraintLMMechanicalContact::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _var.number())
  {
    computeJacobian();
    return;
  }

  // If the LM is determining the residual, then this is zero. If it is not, then we're not going to
  // do anything because we've already populated the off-diagonal Jacobian from the mortar object.
  // The only thing we do anything here is to make sure that our _projection_successful member gets
  // populated correctly such that calls to overwriteSecondaryJacobian (which calls to
  // overwriteSecondaryResidual) return the correct boolean

  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      _projection_successful = true;
      return;
    }
  }
  _projection_successful = false;
}

Real
ApplyPenetrationConstraintLMMechanicalContact::computeQpResidual(const Moose::ConstraintType)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      _projection_successful = true;
      // We've stored the integrated gap in the residual vector
      Real & integrated_gap = _node_to_integrated_gap[_current_node];
      integrated_gap = _residual_copy(_dof_number) / _var.scalingFactor();
      integrated_gap *= _c;

      // If our LM is less than the integrated gap, then we return the LM, else we return 0 because
      // the residual already is the weighted gap. This is implementing the nonlinear
      // complimentarity problem (NCP) function: min(constant * gap, LM) which enforces our contact
      // conditions:
      //
      // (weighted) gap >= 0
      // contact pressure = LM >= 0
      // gap * LM = 0
      if (std::isnan(integrated_gap) || (_u_secondary[_qp] < integrated_gap))
        return _u_secondary[_qp];
      else
        return 0;
    }
  }
  _projection_successful = false;
  return _u_secondary[_qp];
}

Real
ApplyPenetrationConstraintLMMechanicalContact::computeQpJacobian(
    const Moose::ConstraintJacobianType)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      _projection_successful = true;

      const auto & integrated_gap = _node_to_integrated_gap.at(_current_node);
      if (std::isnan(integrated_gap) || (_u_secondary[_qp] < integrated_gap))
        return 1;
      else
        return 0;
    }
  }
  _projection_successful = false;
  return 1;
}
