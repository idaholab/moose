//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HomogenizationConstraint.h"

#include "Function.h"

registerMooseObject("SolidMechanicsApp", HomogenizationConstraint);

InputParameters
HomogenizationConstraint::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Calculate element contribution to the homogenization constraint "
                             "depending on the homogenization constraint type.");
  params.addRequiredParam<MultiMooseEnum>(
      "constraint_types",
      Homogenization::constraintType,
      "Type of each constraint: strain, stress, or none. The types are specified in the "
      "column-major order, and there must be 9 entries in total.");
  params.addRequiredParam<std::vector<FunctionName>>(
      "targets", "Functions giving the targets to hit for constraint types that are not none.");
  params.addParam<bool>("large_kinematics", false, "Using large displacements?");
  params.addParam<std::string>("base_name", "Material property base name");
  return params;
}

HomogenizationConstraint::HomogenizationConstraint(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _large_kinematics(getParam<bool>("large_kinematics")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _F(getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient")),
    _pk1_stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "pk1_stress")),
    _pk1_jacobian(getMaterialPropertyByName<RankFourTensor>(_base_name + "pk1_jacobian"))
{
  // Constraint types
  auto types = getParam<MultiMooseEnum>("constraint_types");
  if (types.size() != Moose::dim * Moose::dim)
    mooseError("Number of constraint types must equal dim * dim. ", types.size(), " are provided.");

  // Targets to hit
  const std::vector<FunctionName> & fnames = getParam<std::vector<FunctionName>>("targets");

  // Prepare the constraint map
  unsigned int fcount = 0;
  for (const auto j : make_range(Moose::dim))
    for (const auto i : make_range(Moose::dim))
    {
      const auto idx = i + Moose::dim * j;
      const auto ctype = static_cast<Homogenization::ConstraintType>(types.get(idx));
      if (ctype != Homogenization::ConstraintType::None)
      {
        const Function * const f = &getFunctionByName(fnames[fcount++]);
        _cmap[{i, j}] = {ctype, f};
      }
    }
}

void
HomogenizationConstraint::initialize()
{
  _residual.zero();
  _jacobian.zero();
}

void
HomogenizationConstraint::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    Real dV = _JxW[_qp] * _coord[_qp];
    _residual += computeResidual() * dV;
    _jacobian += computeJacobian() * dV;
  }
}

void
HomogenizationConstraint::threadJoin(const UserObject & y)
{
  const HomogenizationConstraint & other = static_cast<const HomogenizationConstraint &>(y);
  _residual += other._residual;
  _jacobian += other._jacobian;
}

void
HomogenizationConstraint::finalize()
{
  std::vector<Real> residual(&_residual(0, 0), &_residual(0, 0) + RankTwoTensor::N2);
  std::vector<Real> jacobian(&_jacobian(0, 0, 0, 0), &_jacobian(0, 0, 0, 0) + RankFourTensor::N4);

  gatherSum(residual);
  gatherSum(jacobian);

  std::copy(residual.begin(), residual.end(), &_residual(0, 0));
  std::copy(jacobian.begin(), jacobian.end(), &_jacobian(0, 0, 0, 0));
}

RankTwoTensor
HomogenizationConstraint::computeResidual()
{
  RankTwoTensor res;

  for (auto && [indices, constraint] : _cmap)
  {
    auto && [i, j] = indices;
    auto && [ctype, ctarget] = constraint;

    if (_large_kinematics)
    {
      if (ctype == Homogenization::ConstraintType::Stress)
        res(i, j) = _pk1_stress[_qp](i, j) - ctarget->value(_t, _q_point[_qp]);
      else if (ctype == Homogenization::ConstraintType::Strain)
        res(i, j) = _F[_qp](i, j) - (Real(i == j) + ctarget->value(_t, _q_point[_qp]));
      else
        mooseError("Unknown constraint type in the integral!");
    }
    else
    {
      if (ctype == Homogenization::ConstraintType::Stress)
        res(i, j) = _pk1_stress[_qp](i, j) - ctarget->value(_t, _q_point[_qp]);
      else if (ctype == Homogenization::ConstraintType::Strain)
        res(i, j) = 0.5 * (_F[_qp](i, j) + _F[_qp](j, i)) -
                    (Real(i == j) + ctarget->value(_t, _q_point[_qp]));
      else
        mooseError("Unknown constraint type in the integral!");
    }
  }

  return res;
}

RankFourTensor
HomogenizationConstraint::computeJacobian()
{
  RankFourTensor res;

  for (auto && [indices1, constraint1] : _cmap)
  {
    auto && [i, j] = indices1;
    auto && ctype = constraint1.first;
    for (auto && indices2 : _cmap)
    {
      auto && [a, b] = indices2.first;
      if (ctype == Homogenization::ConstraintType::Stress)
        res(i, j, a, b) = _pk1_jacobian[_qp](i, j, a, b);
      else if (ctype == Homogenization::ConstraintType::Strain)
      {
        if (_large_kinematics)
          res(i, j, a, b) = Real(i == a && j == b);
        else
          res(i, j, a, b) = 0.5 * Real(i == a && j == b) + 0.5 * Real(i == b && j == a);
      }
      else
        mooseError("Unknown constraint type in Jacobian calculator!");
    }
  }

  return res;
}
