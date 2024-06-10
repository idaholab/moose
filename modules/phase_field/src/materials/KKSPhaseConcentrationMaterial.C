//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSPhaseConcentrationMaterial.h"
#include "MatrixTools.h"

registerMooseObject("PhaseFieldApp", KKSPhaseConcentrationMaterial);

InputParameters
KKSPhaseConcentrationMaterial::validParams()
{
  InputParameters params = DerivativeMaterialInterface<Material>::validParams();
  params.addClassDescription("Computes the KKS phase concentrations by using nested Newton "
                             "iteration to solve the equal chemical "
                             "potential and concentration conservation equations. This class is "
                             "intended to be used with KKSPhaseConcentrationDerivatives.");
  params.addRequiredCoupledVar("global_cs", "The interpolated concentrations c, b, etc.");
  params.addRequiredParam<MaterialPropertyName>("h_name", "Switching function h(eta).");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "ci_names",
      "Phase concentrations. The order must match Fa, Fb, and global_cs, for example, c1, c2, b1, "
      "b2, etc.");
  params.addRequiredParam<std::vector<Real>>("ci_IC",
                                             "Initial values of ci in the same order as ci_names.");
  params.addRequiredParam<MaterialName>("fa_name", "Fa material object.");
  params.addRequiredParam<MaterialName>("fb_name", "Fb material object.");
  params.addParam<MaterialPropertyName>(
      "nested_iterations",
      "The output number of nested Newton iterations at each quadrature point.");
  params.addCoupledVar("args", "The coupled variables of Fa and Fb.");
  params.addParam<bool>(
      "damped_Newton", false, "Whether or not to use the damped Newton's method.");
  params.addParam<MaterialName>("conditions",
                                "C",
                                "Material property that checks bounds and conditions on the "
                                "material properties being solved for.");
  params += NestedSolve::validParams();
  return params;
}

KKSPhaseConcentrationMaterial::KKSPhaseConcentrationMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _prop_c(coupledValues("global_cs")),
    _num_c(coupledComponents("global_cs")),
    _prop_h(getMaterialProperty<Real>("h_name")),
    _ci_names(getParam<std::vector<MaterialPropertyName>>("ci_names")),
    _prop_ci(_num_c * 2),
    _ci_old(_num_c * 2),
    _ci_IC(getParam<std::vector<Real>>("ci_IC")),
    _Fa_name(getParam<MaterialName>("fa_name")),
    _Fb_name(getParam<MaterialName>("fb_name")),
    _prop_Fi(2),
    _Fi_copy(2),
    _dFidci(_num_c * 2),
    _dFidci_copy(_num_c * 2),
    _d2Fidcidbi(2),
    _d2Fadc1db1_copy(_num_c),
    _args_names(coupledNames("args")),
    _n_args(coupledComponents("args")),
    _dFadarg(_n_args),
    _dFadarg_copy(_n_args),
    _dFbdarg(_n_args),
    _dFbdarg_copy(_n_args),
    _d2Fadcadarg(_n_args),
    _d2Fadcadarg_copy(_n_args),
    _iter(declareProperty<Real>("nested_iterations")),
    _abs_tol(getParam<Real>("absolute_tolerance")),
    _rel_tol(getParam<Real>("relative_tolerance")),
    _damped_newton(getParam<bool>("damped_Newton")),
    _condition_name(getParam<MaterialName>("conditions")),
    _nested_solve(NestedSolve(parameters))

{
  // phase concentrations
  for (const auto m : make_range(_num_c * 2))
  {
    _prop_ci[m] = &declareProperty<Real>(_ci_names[m]);
    _ci_old[m] = &getMaterialPropertyOld<Real>(_ci_names[m]);
  }

  // free energies
  _prop_Fi[0] = &getMaterialPropertyByName<Real>(_Fa_name);
  _prop_Fi[1] = &getMaterialPropertyByName<Real>(_Fb_name);

  // declare _fi_copy to be passed to the kernels
  _Fi_copy[0] = &declareProperty<Real>("cp" + _Fa_name);
  _Fi_copy[1] = &declareProperty<Real>("cp" + _Fb_name);

  // derivative of free energy wrt phase concentrations
  for (const auto m : make_range(_num_c))
  {
    _dFidci[m * 2] = &getMaterialPropertyDerivative<Real>(_Fa_name, _ci_names[m * 2]);
    _dFidci[m * 2 + 1] = &getMaterialPropertyDerivative<Real>(_Fb_name, _ci_names[m * 2 + 1]);

    // declare _dFidci_copy to be passed to the kernels
    _dFidci_copy[m * 2] = &declarePropertyDerivative<Real>("cp" + _Fa_name, _ci_names[m * 2]);
    _dFidci_copy[m * 2 + 1] =
        &declarePropertyDerivative<Real>("cp" + _Fb_name, _ci_names[m * 2 + 1]);
  }

  // Second derivative of free energy wrt phase concentrations for use in this material. In
  // _d2Fidcidbi[m][n][l], m is phase index of Fi, n is the species index of ci, l is the species
  // index of bi.
  for (const auto m : make_range(2))
  {
    _d2Fidcidbi[m].resize(_num_c);
    for (const auto n : make_range(_num_c))
    {
      _d2Fidcidbi[m][n].resize(_num_c);
      for (const auto l : make_range(_num_c))
      {
        if (m == 0)
          _d2Fidcidbi[0][n][l] =
              &getMaterialPropertyDerivative<Real>(_Fa_name, _ci_names[n * 2], _ci_names[l * 2]);

        else
          _d2Fidcidbi[1][n][l] = &getMaterialPropertyDerivative<Real>(
              _Fb_name, _ci_names[n * 2 + 1], _ci_names[l * 2 + 1]);
      }
    }
  }

  // _d2Fadc1db1_copy (2D symmetric matrix), to be passed to kernels
  for (const auto m : make_range(_num_c))
  {
    _d2Fadc1db1_copy[m].resize(_num_c);
    for (const auto n : make_range(_num_c))
      _d2Fadc1db1_copy[m][n] =
          &declarePropertyDerivative<Real>("cp" + _Fa_name, _ci_names[m * 2], _ci_names[n * 2]);
  }

  // partial derivative of Fa and Fb wrt coupled variables, to be passed to kernels
  for (const auto m : make_range(_n_args))
  {
    _dFadarg[m] = &getMaterialPropertyDerivative<Real>(_Fa_name, _args_names[m]);
    _dFadarg_copy[m] = &declarePropertyDerivative<Real>("cp" + _Fa_name, _args_names[m]);
    _dFbdarg[m] = &getMaterialPropertyDerivative<Real>(_Fb_name, _args_names[m]);
    _dFbdarg_copy[m] = &declarePropertyDerivative<Real>("cp" + _Fb_name, _args_names[m]);
  }

  // second partial derivatives of Fa wrt ca and another coupled variable, to be passed to
  // kernels
  for (const auto m : make_range(_n_args))
  {
    _d2Fadcadarg[m].resize(_num_c);
    _d2Fadcadarg_copy[m].resize(_num_c);
    for (const auto n : make_range(_num_c))
    {
      _d2Fadcadarg[m][n] =
          &getMaterialPropertyDerivative<Real>(_Fa_name, _ci_names[n * 2], _args_names[m]);
      _d2Fadcadarg_copy[m][n] =
          &declarePropertyDerivative<Real>("cp" + _Fa_name, _ci_names[n * 2], _args_names[m]);
    }
  }

  if (_damped_newton)
    _C = &getMaterialPropertyByName<Real>(_condition_name);
  else
    _C = nullptr;
}

void
KKSPhaseConcentrationMaterial::initQpStatefulProperties()
{
  for (const auto m : make_range(_num_c * 2))
    (*_prop_ci[m])[_qp] = _ci_IC[m];
}

void
KKSPhaseConcentrationMaterial::initialSetup()
{
  _Fa = &getMaterial("fa_name");
  _Fb = &getMaterial("fb_name");
  if (_damped_newton)
    _condition = &getMaterialByName(_condition_name);
}

void
KKSPhaseConcentrationMaterial::computeQpProperties()
{
  // parameters for nested Newton iteration
  NestedSolve::Value<> solution(_num_c * 2);

  for (unsigned int m = 0; m < _num_c * 2; ++m)
    solution(m) = (*_ci_old[m])[_qp];

  _nested_solve.setAbsoluteTolerance(_abs_tol);
  _nested_solve.setRelativeTolerance(_rel_tol);

  auto compute = [&](const NestedSolve::Value<> & guess,
                     NestedSolve::Value<> & residual,
                     NestedSolve::Jacobian<> & jacobian)
  {
    for (const auto m : make_range(_num_c * 2))
      (*_prop_ci[m])[_qp] = guess(m);

    _Fa->computePropertiesAtQp(_qp);
    _Fb->computePropertiesAtQp(_qp);

    // assign residual functions
    for (const auto m : make_range(_num_c))
    {
      residual(m * 2) = (*_dFidci[m * 2])[_qp] - (*_dFidci[m * 2 + 1])[_qp];
      residual(m * 2 + 1) = (1 - _prop_h[_qp]) * (*_prop_ci[m * 2])[_qp] +
                            _prop_h[_qp] * (*_prop_ci[m * 2 + 1])[_qp] - (*_prop_c[m])[_qp];
    }

    jacobian.setZero();

    // fill in the non-zero elements in jacobian
    for (const auto m : make_range(_num_c))
    {
      for (const auto n : make_range(_num_c))
      {
        // equal chemical potential derivative equations
        jacobian(m * 2, n * 2) = (*_d2Fidcidbi[0][m][n])[_qp];
        jacobian(m * 2, n * 2 + 1) = -(*_d2Fidcidbi[1][m][n])[_qp];
      }
      // concentration conservation derivative equations
      jacobian(m * 2 + 1, m * 2) = 1 - _prop_h[_qp];
      jacobian(m * 2 + 1, m * 2 + 1) = _prop_h[_qp];
    }
  };
  auto computeCondition = [&](const NestedSolve::Value<> & guess) -> Real
  {
    for (const auto m : make_range(_num_c * 2))
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
  for (const auto m : make_range(_num_c * 2))
    (*_prop_ci[m])[_qp] = solution[m];

  // assign to the copied parameters to be used in kernels
  for (const auto m : make_range(2))
    (*_Fi_copy[m])[_qp] = (*_prop_Fi[m])[_qp];

  for (const auto m : make_range(_num_c * 2))
    (*_dFidci_copy[m])[_qp] = (*_dFidci[m])[_qp];

  for (const auto m : make_range(_num_c))
  {
    for (const auto n : make_range(_num_c))
      (*_d2Fadc1db1_copy[m][n])[_qp] = (*_d2Fidcidbi[0][m][n])[_qp];
  }

  for (const auto m : make_range(_n_args))
  {
    (*_dFadarg_copy[m])[_qp] = (*_dFadarg[m])[_qp];
    (*_dFbdarg_copy[m])[_qp] = (*_dFbdarg[m])[_qp];
  }

  for (const auto m : make_range(_n_args))
  {
    for (const auto n : make_range(_num_c))
      (*_d2Fadcadarg_copy[m][n])[_qp] = (*_d2Fadcadarg[m][n])[_qp];
  }
}
