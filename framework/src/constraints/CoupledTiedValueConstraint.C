//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledTiedValueConstraint.h"

// MOOSE includes
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/sparse_matrix.h"

registerMooseObject("MooseApp", CoupledTiedValueConstraint);

InputParameters
CoupledTiedValueConstraint::validParams()
{
  InputParameters params = NodeFaceConstraint::validParams();
  params.addClassDescription(
      "Requires the value of two variables to be the consistent on both sides of an interface.");
  params.addParam<Real>("scaling", 1, "scaling factor to be applied to constraint equations");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

CoupledTiedValueConstraint::CoupledTiedValueConstraint(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _scaling(getParam<Real>("scaling")),
    _residual_copy(_sys.residualGhosted())
{
}

Real
CoupledTiedValueConstraint::computeQpSecondaryValue()
{
  return _u_primary[_qp];
}

Real
CoupledTiedValueConstraint::computeQpResidual(Moose::ConstraintType type)
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
CoupledTiedValueConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
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
      retVal = 0;
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

Real
CoupledTiedValueConstraint::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                     unsigned int jvar)
{
  Real retVal = 0;

  if (jvar == _primary_var_num)
  {
    switch (type)
    {
      case Moose::SecondarySecondary:
        retVal = 0;
        break;
      case Moose::SecondaryPrimary:
        retVal = -_phi_primary[_j][_qp] * _test_secondary[_i][_qp] * _scaling;
        break;
      case Moose::PrimarySecondary:
        retVal = 0;
        break;
      case Moose::PrimaryPrimary:
        retVal = 0;
        break;
      default:
        mooseError("Unsupported type");
        break;
    }
  }

  return retVal;
}
