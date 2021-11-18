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

#include "HomogenizationConstraintIntegral.h"

#include "Function.h"

registerMooseObject("TensorMechanicsApp", HomogenizationConstraintIntegral);

InputParameters
HomogenizationConstraintIntegral::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredCoupledVar("displacements", "The problem displacements");
  MultiMooseEnum constraintType("strain stress");
  params.addRequiredParam<MultiMooseEnum>("constraint_types",
                                          HomogenizationConstants::mooseConstraintType,
                                          "Type of each constraint: strain or stress");
  params.addRequiredParam<std::vector<FunctionName>>("targets",
                                                     "Functions giving the targets to hit");
  params.addParam<bool>("large_kinematics", false, "Using large displacements?");

  params.addParam<std::string>("base_name", "Material property base name");

  return params;
}

HomogenizationConstraintIntegral::HomogenizationConstraintIntegral(
    const InputParameters & parameters)
  : ElementUserObject(parameters),
    _large_kinematics(getParam<bool>("large_kinematics")),
    _ndisp(coupledComponents("displacements")),
    _ncomps(HomogenizationConstants::required.at(_large_kinematics)[_ndisp - 1]),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _F(getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient")),
    _pk1_stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "pk1_stress")),
    _pk1_jacobian(getMaterialPropertyByName<RankFourTensor>(_base_name + "pk1_jacobian")),
    _indices(HomogenizationConstants::indices.at(_large_kinematics)[_ndisp - 1])
{
  const std::vector<FunctionName> & names = getParam<std::vector<FunctionName>>("targets");

  unsigned int nfns = names.size();
  if (nfns != _ncomps)
  {
    mooseError("Homogenization constraint user object needs ", _ncomps, " functions");
  }
  for (unsigned int i = 0; i < nfns; i++)
  {
    const Function * const f = &getFunctionByName(names[i]);
    if (!f)
      mooseError("Function ", names[i], " not found.");
    _targets.push_back(f);
  }

  auto types = getParam<MultiMooseEnum>("constraint_types");
  if (types.size() != _ncomps)
  {
    mooseError("Number of constraint types must match the number of "
               "functions");
  }
  for (unsigned int i = 0; i < _ncomps; i++)
  {
    _ctypes.push_back(static_cast<HomogenizationConstants::ConstraintType>(types.get(i)));
  }
}

void
HomogenizationConstraintIntegral::initialize()
{
  _residual.zero();
  _jacobian.zero();
}

void
HomogenizationConstraintIntegral::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    Real dV = _JxW[_qp] * _coord[_qp];
    _residual += computeResidual() * dV;
    _jacobian += computeJacobian() * dV;
  }
}

void
HomogenizationConstraintIntegral::threadJoin(const UserObject & y)
{
  const HomogenizationConstraintIntegral & other =
      static_cast<const HomogenizationConstraintIntegral &>(y);
  _residual += other._residual;
  _jacobian += other._jacobian;
}

void
HomogenizationConstraintIntegral::finalize()
{
  std::vector<Real> residual(&_residual(0, 0), &_residual(0, 0) + 9);
  std::vector<Real> jacobian(&_jacobian(0, 0, 0, 0), &_jacobian(0, 0, 0, 0) + 81);

  gatherSum(residual);
  gatherSum(jacobian);

  std::copy(residual.begin(), residual.end(), &_residual(0, 0));
  std::copy(jacobian.begin(), jacobian.end(), &_jacobian(0, 0, 0, 0));
}

const RankTwoTensor &
HomogenizationConstraintIntegral::getResidual() const
{
  return _residual;
}

const RankFourTensor &
HomogenizationConstraintIntegral::getJacobian() const
{
  return _jacobian;
}

RankTwoTensor
HomogenizationConstraintIntegral::computeResidual()
{
  RankTwoTensor res;
  res.zero();

  // Loop over each term
  for (_h = 0; _h < _ncomps; _h++)
  {
    if (_large_kinematics)
    {
      // PK stress
      if (_ctypes[_h] == HomogenizationConstants::ConstraintType::Stress)
        res(_indices[_h].first, _indices[_h].second) =
            _pk1_stress[_qp](_indices[_h].first, _indices[_h].second) -
            _targets[_h]->value(_t, _q_point[_qp]);
      // Deformation gradient
      else if (_ctypes[_h] == HomogenizationConstants::ConstraintType::Strain)
      {
        Real f = (_indices[_h].first == _indices[_h].second) ? 1.0 : 0.0;
        res(_indices[_h].first, _indices[_h].second) =
            _F[_qp](_indices[_h].first, _indices[_h].second) -
            (f + _targets[_h]->value(_t, _q_point[_qp]));
      }
      else
        mooseError("Unknown constraint type in the integral!");
    }
    else
    {
      // Small stress
      if (_ctypes[_h] == HomogenizationConstants::ConstraintType::Stress)
        res(_indices[_h].first, _indices[_h].second) =
            _pk1_stress[_qp](_indices[_h].first, _indices[_h].second) -
            _targets[_h]->value(_t, _q_point[_qp]);
      // Small strain
      else if (_ctypes[_h] == HomogenizationConstants::ConstraintType::Strain)
      {
        Real f = (_indices[_h].first == _indices[_h].second) ? 1.0 : 0.0;
        res(_indices[_h].first, _indices[_h].second) =
            0.5 * (_F[_qp](_indices[_h].first, _indices[_h].second) +
                   _F[_qp](_indices[_h].second, _indices[_h].first)) -
            (f + _targets[_h]->value(_t, _q_point[_qp]));
      }
      else
        mooseError("Unknown constraint type in the integral!");
    }
  }
  return res;
}

RankFourTensor
HomogenizationConstraintIntegral::computeJacobian()
{
  RankFourTensor res;
  res.zero();

  for (_h = 0; _h < _ncomps; _h++)
  {
    unsigned int i = _indices[_h].first;
    unsigned int j = _indices[_h].second;
    for (_hh = 0; _hh < _ncomps; _hh++)
    {
      unsigned int a = _indices[_hh].first;
      unsigned int b = _indices[_hh].second;
      if (_ctypes[_h] == HomogenizationConstants::ConstraintType::Stress)
        res(i, j, a, b) = stressJacobian(i, j, a, b);
      else if (_ctypes[_h] == HomogenizationConstants::ConstraintType::Strain)
      {
        if (_large_kinematics)
          res(i, j, a, b) = ldStrainJacobian(i, j, a, b);
        else
          res(i, j, a, b) = sdStrainJacobian(i, j, a, b);
      }
      else
        mooseError("Unknown constraint type in Jacobian calculator!");
    }
  }

  return res;
}

Real
HomogenizationConstraintIntegral::stressJacobian(unsigned int i,
                                                 unsigned int j,
                                                 unsigned int a,
                                                 unsigned int b)
{
  return _pk1_jacobian[_qp](i, j, a, b);
}

Real
HomogenizationConstraintIntegral::sdStrainJacobian(unsigned int i,
                                                   unsigned int j,
                                                   unsigned int a,
                                                   unsigned int b)
{
  Real res = 0.0;
  if ((i == a) && (j == b))
    res += 0.5;
  if ((i == b) && (j == a))
    res += 0.5;
  return res;
}

Real
HomogenizationConstraintIntegral::ldStrainJacobian(unsigned int i,
                                                   unsigned int j,
                                                   unsigned int a,
                                                   unsigned int b)
{
  if ((i == a) && (j == b))
    return 1.0;
  else
    return 0.0;
}
