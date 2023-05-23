//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HomogenizedTotalLagrangianStressDivergence.h"

registerMooseObject("TensorMechanicsApp", HomogenizedTotalLagrangianStressDivergence);

InputParameters
HomogenizedTotalLagrangianStressDivergence::validParams()
{
  InputParameters params = TotalLagrangianStressDivergence::validParams();
  params.addClassDescription("Total Lagrangian stress equilibrium kernel with "
                             "homogenization constraint Jacobian terms");
  params.addRequiredCoupledVar("macro_gradient", "Optional scalar field with the macro gradient");
  params.addRequiredParam<UserObjectName>("homogenization_constraint",
                                          "The UserObject defining the homogenization constraint");

  return params;
}

HomogenizedTotalLagrangianStressDivergence::HomogenizedTotalLagrangianStressDivergence(
    const InputParameters & parameters)
  : TotalLagrangianStressDivergence(parameters),
    _macro_gradient_num(coupledScalar("macro_gradient")),
    _constraint(getUserObject<HomogenizationConstraint>("homogenization_constraint")),
    _cmap(_constraint.getConstraintMap())
{
}

void
HomogenizedTotalLagrangianStressDivergence::computeOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _macro_gradient_num)
  {
    prepareMatrixTag(_assembly, _var.number(), jvar, _ken);
    prepareMatrixTag(_assembly, jvar, _var.number(), _kne);

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const Real dV = _JxW[_qp] * _coord[_qp];
      unsigned int h = 0;
      for (auto && [indices, constraint] : _cmap)
      {
        auto && [i, j] = indices;
        auto && ctype = constraint.first;
        for (_i = 0; _i < _test.size(); _i++)
        {
          // This assumes Galerkin, i.e. the test and trial functions are the same
          _j = _i;

          // Base Jacobian
          _ken(_i, h) += _dpk1[_qp].contractionKl(i, j, gradTest(_alpha)) * dV;

          // Constraint Jacobian
          if (ctype == Homogenization::ConstraintType::Stress)
            _kne(h, _i) += _dpk1[_qp].contractionIj(i, j, gradTrial(_alpha)) * dV;
          else if (ctype == Homogenization::ConstraintType::Strain)
            if (_large_kinematics)
              _kne(h, _i) += Real(i == _alpha) * gradTrial(_alpha)(i, j) * dV;
            else
              _kne(h, _i) += 0.5 *
                             (Real(i == _alpha) * gradTrial(_alpha)(i, j) +
                              Real(j == _alpha) * gradTrial(_alpha)(j, i)) *
                             dV;
          else
            mooseError("Unknown constraint type in kernel calculation!");
        }
        h++;
      }
    }
    accumulateTaggedLocalMatrix(_assembly, _var.number(), jvar, _ken);
    accumulateTaggedLocalMatrix(_assembly, jvar, _var.number(), _kne);
  }
}
