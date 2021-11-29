//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TiedValueConstraint.h"

// MOOSE includes
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/sparse_matrix.h"

registerMooseObject("MooseApp", TiedValueConstraint);

InputParameters
TiedValueConstraint::validParams()
{
  InputParameters params = NodeFaceConstraint::validParams();
  params.addClassDescription("Constraint that forces the value of a variable to be the same on "
                             "both sides of an interface.");
  params.addParam<Real>("scaling", 1, "scaling factor to be applied to constraint equations");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

TiedValueConstraint::TiedValueConstraint(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _scaling(getParam<Real>("scaling")),
    _residual_copy(_sys.residualGhosted())
{
}

Real
TiedValueConstraint::computeQpSecondaryValue()
{
  return _u_primary[_qp];
}

Real
TiedValueConstraint::computeQpResidual(Moose::ConstraintType type)
{
  Real scaling_factor = _var.scalingFactor();
  Real secondary_resid = 0;
  Real retVal = 0;
  switch (type)
  {
    case Moose::Secondary:
      retVal = (_u_secondary[_qp] - _u_primary[_qp]) * _test_secondary[_i][_qp] * _scaling;
      break;
    case Moose::Primary:
      secondary_resid =
          _residual_copy(_current_node->dof_number(0, _var.number(), 0)) / scaling_factor;
      retVal = secondary_resid * _test_primary[_i][_qp];
      break;
    default:
      break;
  }
  return retVal;
}

Real
TiedValueConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  Real scaling_factor = _var.scalingFactor();
  Real secondary_jac = 0;
  Real retVal = 0;
  switch (type)
  {
    case Moose::SecondarySecondary:
      retVal = _phi_secondary[_j][_qp] * _test_secondary[_i][_qp] * _scaling;
      break;
    case Moose::SecondaryPrimary:
      retVal = -_phi_primary[_j][_qp] * _test_secondary[_i][_qp] * _scaling;
      break;
    case Moose::PrimarySecondary:
      secondary_jac =
          (*_jacobian)(_current_node->dof_number(0, _var.number(), 0), _connected_dof_indices[_j]);
      retVal = secondary_jac * _test_primary[_i][_qp] / scaling_factor;
      break;
    case Moose::PrimaryPrimary:
      retVal = 0;
      break;
    default:
      mooseError("Unsupported type");
      break;
  }
  return retVal;
}
