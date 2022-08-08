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
  params.addParam<MultiMooseEnum>("constraint_types",
                                  HomogenizationConstants::mooseConstraintType,
                                  "Type of each constraint: strain or stress");

  return params;
}

HomogenizedTotalLagrangianStressDivergence::HomogenizedTotalLagrangianStressDivergence(
    const InputParameters & parameters)
  : TotalLagrangianStressDivergence(parameters),
    _macro_gradient_num(coupledScalar("macro_gradient")),
    _mg_order(getScalarVar("macro_gradient", 0)->order()),
    _indices(HomogenizationConstants::indices.at(_large_kinematics)[_ndisp - 1])
{
  // Check the order of the scalar variable
  unsigned int needed = HomogenizationConstants::required.at(_large_kinematics)[_ndisp - 1];
  if (_mg_order != needed)
    mooseError("The homogenization macro gradient variable must have order ", needed);

  // Check the number of constraints
  auto types = getParam<MultiMooseEnum>("constraint_types");
  if (types.size() != needed)
    mooseError("The kernel must be supplied ", needed, " constraint types.");

  // Setup the types of constraints
  if (isCoupledScalar("macro_gradient", 0))
  {
    auto types = getParam<MultiMooseEnum>("constraint_types");
    for (unsigned int i = 0; i < types.size(); i++)
      _ctypes.push_back(static_cast<HomogenizationConstants::ConstraintType>(types.get(i)));
  }
}

void
HomogenizedTotalLagrangianStressDivergence::computeOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _macro_gradient_num)
  {
    DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), jvar);
    DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar, _var.number());

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const Real dV = _JxW[_qp] * _coord[_qp];
      for (_h = 0; _h < _mg_order; _h++)
      {
        for (_i = 0; _i < _test.size(); _i++)
        { // This assumes Galerkin, i.e. the test and trial functions are the
          // same
          _j = _i;
          ken(_i, _h) += computeBaseJacobian() * dV;
          kne(_h, _i) += computeConstraintJacobian() * dV;
        }
      }
    }
  }
}

Real
HomogenizedTotalLagrangianStressDivergence::computeBaseJacobian()
{
  return materialBaseJacobian();
}

Real
HomogenizedTotalLagrangianStressDivergence::materialBaseJacobian()
{
  return _dpk1[_qp].contractionKl(_indices[_h].first, _indices[_h].second, gradTest(_alpha));
}

Real
HomogenizedTotalLagrangianStressDivergence::computeConstraintJacobian()
{
  if (_ctypes[_h] == HomogenizationConstants::ConstraintType::Stress)
    return materialConstraintJacobianStress();
  else if (_ctypes[_h] == HomogenizationConstants::ConstraintType::Strain)
    if (_large_kinematics)
      return ldConstraintJacobianStrain();
    else
      return sdConstraintJacobianStrain();
  else
    mooseError("Unknown constraint type in kernel calculation!");
}

Real
HomogenizedTotalLagrangianStressDivergence::sdConstraintJacobianStrain()
{
  Real value = 0.0;
  if (_indices[_h].first == _alpha)
    value += 0.5 * gradTrial(_alpha)(_indices[_h].first, _indices[_h].second);
  if (_indices[_h].second == _alpha)
    value += 0.5 * gradTrial(_alpha)(_indices[_h].second, _indices[_h].first);
  return value;
}

Real
HomogenizedTotalLagrangianStressDivergence::ldConstraintJacobianStrain()
{
  unsigned int i = _indices[_h].first;
  unsigned int j = _indices[_h].second;
  if (i == _alpha)
    return gradTrial(_alpha)(i, j);
  else
    return 0;
}

Real
HomogenizedTotalLagrangianStressDivergence::materialConstraintJacobianStress()
{
  return _dpk1[_qp].contractionIj(_indices[_h].first, _indices[_h].second, gradTrial(_alpha));
}
