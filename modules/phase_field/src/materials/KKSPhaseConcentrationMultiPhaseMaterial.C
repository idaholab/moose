//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSPhaseConcentrationMultiPhaseMaterial.h"
#include "MatrixTools.h"

registerMooseObject("PhaseFieldApp", KKSPhaseConcentrationMultiPhaseMaterial);

InputParameters
KKSPhaseConcentrationMultiPhaseMaterial::validParams()
{
  InputParameters params = DerivativeMaterialInterface<Material>::validParams();
  params.addClassDescription(
      "Computes the KKS phase concentrations by using a nested Newton iteration "
      "to solve the equal chemical potential and concentration conservation equations for "
      "multiphase systems. This class is intented to be used with "
      "KKSPhaseConcentrationMultiPhaseDerivatives.");
  params.addRequiredCoupledVar("global_cs", "The interpolated concentrations c, b, etc.");
  params.addRequiredCoupledVar("all_etas", "Order parameters.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "Switching functions in the same order as all_etas.");
  params.addRequiredParam<std::vector<MaterialName>>(
      "Fj_names", "Free energy material objects in the same order as all_etas.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "ci_names",
      "Phase concentrations. They must have the same order as Fj_names and global_cs, for "
      "example, c1, c2, b1, b2.");
  params.addRequiredParam<std::vector<Real>>("ci_IC",
                                             "Initial values of ci in the same order of ci_names");
  params.addParam<MaterialPropertyName>(
      "nested_iterations",
      "The output number of nested Newton iterations at each quadrature point.");
  params.addCoupledVar("args", "The coupled variables of free energies.");
  params.addParam<bool>(
      "damped_Newton", false, "Whether or not to use the damped Newton's method.");
  params.addParam<MaterialName>("conditions",
                                "C",
                                "Material property that checks bounds and conditions on the "
                                "material properties being solved for.");
  params += NestedSolve::validParams();
  return params;
}

KKSPhaseConcentrationMultiPhaseMaterial::KKSPhaseConcentrationMultiPhaseMaterial(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _prop_c(coupledValues("global_cs")),
    _num_c(coupledComponents("global_cs")),
    _num_j(coupledComponents("all_etas")),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_hj(_num_j),
    _Fj_names(getParam<std::vector<MaterialName>>("Fj_names")),
    _prop_Fi(_num_j),
    _ci_names(getParam<std::vector<MaterialPropertyName>>("ci_names")),
    _prop_ci(_num_c * _num_j),
    _ci_old(_num_c * _num_j),
    _ci_IC(getParam<std::vector<Real>>("ci_IC")),
    _dFidci(_num_j),
    _d2Fidcidbi(_num_j),
    _args_names(coupledNames("args")),
    _n_args(coupledComponents("args")),
    _dFidarg(_num_j),
    _d2F1dc1darg(_num_c),
    _iter(declareProperty<Real>("nested_iterations")),
    _abs_tol(getParam<Real>("absolute_tolerance")),
    _rel_tol(getParam<Real>("relative_tolerance")),
    _damped_newton(getParam<bool>("damped_Newton")),
    _condition_name(getParam<MaterialName>("conditions")),
    _nested_solve(NestedSolve(parameters))

{
  // phase concentrations
  for (const auto m : make_range(_num_c * _num_j))
  {
    _ci_old[m] = &getMaterialPropertyOld<Real>(_ci_names[m]);
    _prop_ci[m] = &declareProperty<Real>(_ci_names[m]);
  }

  // free energies
  for (const auto m : make_range(_num_j))
    _prop_Fi[m] = &getMaterialPropertyByName<Real>(_Fj_names[m]);

  // free energy derivatives w.r.t. phase concentrations
  for (const auto m : make_range(_num_j))
  {
    _prop_hj[m] = &getMaterialPropertyByName<Real>(_hj_names[m]);

    _dFidci[m].resize(_num_c);
    _d2Fidcidbi[m].resize(_num_c);

    for (const auto n : make_range(_num_c))
    {
      _dFidci[m][n] = &getMaterialPropertyDerivative<Real>(_Fj_names[m], _ci_names[m + n * _num_j]);

      _d2Fidcidbi[m][n].resize(_num_c);

      for (unsigned int l = 0; l < _num_c; ++l)
      {
        _d2Fidcidbi[m][n][l] = &getMaterialPropertyDerivative<Real>(
            _Fj_names[m], _ci_names[m + n * _num_j], _ci_names[m + l * _num_j]);
      }
    }
  }

  // derivative of free energies wrt coupled variables
  for (const auto m : make_range(_num_j))
  {
    _dFidarg[m].resize(_n_args);

    for (const auto n : make_range(_n_args))
      _dFidarg[m][n] = &getMaterialPropertyDerivative<Real>(_Fj_names[m], _args_names[n]);
  }

  // second derivatives of F1 wrt c1 and other coupled variables
  for (const auto m : make_range(_num_c))
  {
    _d2F1dc1darg[m].resize(_n_args);

    for (const auto n : make_range(_n_args))
    {
      _d2F1dc1darg[m][n] =
          &getMaterialPropertyDerivative<Real>(_Fj_names[0], _ci_names[m * _num_j], _args_names[n]);
    }
  }

  if (_damped_newton)
    _C = &getMaterialPropertyByName<Real>(_condition_name);
  else
    _C = nullptr;
}

void
KKSPhaseConcentrationMultiPhaseMaterial::initQpStatefulProperties()
{
  for (const auto m : make_range(_num_c * _num_j))
    (*_prop_ci[m])[_qp] = _ci_IC[m];
}

void
KKSPhaseConcentrationMultiPhaseMaterial::initialSetup()
{
  _Fj_mat.resize(_num_j);

  for (unsigned int m = 0; m < _num_j; ++m)
    _Fj_mat[m] = &getMaterialByName(_Fj_names[m]);
  if (_damped_newton)
    _condition = &getMaterialByName(_condition_name);
}

void
KKSPhaseConcentrationMultiPhaseMaterial::computeQpProperties()
{
  // parameters for nested Newton iteration
  NestedSolve::Value<> solution(_num_c * _num_j);

  // initialize first guess using the solution from previous step
  for (const auto m : make_range(_num_c * _num_j))
    solution(m) = (*_ci_old[m])[_qp];

  _nested_solve.setAbsoluteTolerance(_abs_tol);
  _nested_solve.setRelativeTolerance(_rel_tol);

  auto compute = [&](const NestedSolve::Value<> & guess,
                     NestedSolve::Value<> & residual,
                     NestedSolve::Jacobian<> & jacobian)
  {
    for (const auto m : make_range(_num_c * _num_j))
      (*_prop_ci[m])[_qp] = guess(m);

    for (const auto m : make_range(_num_j))
      _Fj_mat[m]->computePropertiesAtQp(_qp);

    // assign residual functions
    for (const auto m : make_range(_num_c))
    {
      for (const auto n : make_range(_num_j - 1))
        residual(m * _num_j + n) = (*_dFidci[n][m])[_qp] - (*_dFidci[n + 1][m])[_qp];

      residual((m + 1) * _num_j - 1) = -(*_prop_c[m])[_qp];

      for (const auto l : make_range(_num_j))
        residual((m + 1) * _num_j - 1) += (*_prop_hj[l])[_qp] * (*_prop_ci[m * _num_j + l])[_qp];
    }

    jacobian.setZero();

    // fill in the non-zero terms in jacobian
    for (const auto m : make_range(_num_c))
    {
      // equal chemical potential derivative equations
      for (const auto n : make_range(_num_j - 1))
      {
        for (const auto l : make_range(_num_c))
        {
          jacobian(m * _num_j + n, n + l * _num_j) = (*_d2Fidcidbi[n][m][l])[_qp];
          jacobian(m * _num_j + n, n + l * _num_j + 1) = -(*_d2Fidcidbi[n + 1][m][l])[_qp];
        }
      }

      // concentration conservation derivative equations
      for (const auto n : make_range(_num_j))
        jacobian((m + 1) * _num_j - 1, m * _num_j + n) = (*_prop_hj[n])[_qp];
    }
  };

  auto computeCondition = [&](const NestedSolve::Value<> & guess) -> Real
  {
    for (const auto m : make_range(_num_c * _num_j))
      (*_prop_ci[m])[_qp] = guess(m);
    _condition->computePropertiesAtQp(_qp);
    return ((*_C)[_qp]);
  };

  // choose between Newton or damped Newton's method
  if (!_damped_newton)
    _nested_solve.nonlinear(solution, compute);
  else
    _nested_solve.nonlinearDamped(solution, compute, computeCondition);

  _iter[_qp] = _nested_solve.getIterations();

  if (_nested_solve.getState() == NestedSolve::State::NOT_CONVERGED)
    mooseException("Nested Newton iteration did not converge.");

  // assign solution to ci
  for (const auto m : make_range(_num_c * _num_j))
    (*_prop_ci[m])[_qp] = solution[m];
}
