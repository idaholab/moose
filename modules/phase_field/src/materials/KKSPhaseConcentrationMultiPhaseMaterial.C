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
      "to solve the equal chemical potential and concentration conservation equations");
  params.addRequiredCoupledVar("global_cs", "Global concentrations, for example, c, b.");
  params.addRequiredCoupledVar("all_etas", "Order parameters for all phases.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "Names of the switching functions in the same order of the all_etas");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "ci_names",
      "Phase concentrations. These must have the same order as Fj_names and global_cs, for "
      "example, c1, c2, b1, b2.");
  params.addRequiredParam<std::vector<Real>>("ci_IC",
                                             "Initial values of ci in the same order of ci_names");
  params.addRequiredParam<std::vector<MaterialName>>("Fj_material",
                                                     "Free energy material objects.");
  params.addParam<MaterialPropertyName>(
      "nested_iterations",
      "The output number of nested Newton iterations at each quadrature point");
  params.addCoupledVar("args", "The coupled variables of free energy.");
  params += NestedSolve::validParams();
  return params;
}

KKSPhaseConcentrationMultiPhaseMaterial::KKSPhaseConcentrationMultiPhaseMaterial(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _prop_c(coupledValues("global_cs")),
    _num_c(coupledComponents("global_cs")),
    _num_j(coupledComponents("all_etas")),
    _eta_names(coupledNames("all_etas")),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_hj(_num_j),
    _ci_names(getParam<std::vector<MaterialPropertyName>>("ci_names")),
    _prop_ci(_num_c * _num_j),
    _ci_old(_num_c * _num_j),
    _ci_IC(getParam<std::vector<Real>>("ci_IC")),

    _Fj_names(getParam<std::vector<MaterialName>>("Fj_material")),

    _prop_Fi(_num_j),
    _Fi_copy(_num_j),
    _dFidci(_num_j),
    _dFidci_copy(_num_j),
    _d2Fidcidbi(_num_j),
    _d2Fidcidbi_copy(_num_j),
    _args_names(coupledNames("args")),
    _n_args(coupledComponents("args")),
    _dFidarg(_num_j),
    _dFidarg_copy(_num_j),
    _d2F1dc1darg(_num_c),
    _d2F1dc1darg_copy(_num_c),
    _iter(declareProperty<Real>("nested_iterations")),
    _abs_tol(getParam<Real>("absolute_tolerance")),
    _rel_tol(getParam<Real>("relative_tolerance")),
    _nested_solve(NestedSolve(parameters))

{
  for (unsigned int m = 0; m < _num_c * _num_j; ++m)
  {
    _ci_old[m] = &getMaterialPropertyOld<Real>(_ci_names[m]);
    _prop_ci[m] = &declareProperty<Real>(_ci_names[m]);
  }

  for (unsigned int m = 0; m < _num_j; ++m)
  {
    _prop_Fi[m] = &getMaterialPropertyByName<Real>(_Fj_names[m]);
    _Fi_copy[m] = &declareProperty<Real>("cp" + _Fj_names[m]);
  }

  for (unsigned int m = 0; m < _num_j; ++m)
  {
    _prop_hj[m] = &getMaterialPropertyByName<Real>(_hj_names[m]);

    _dFidci[m].resize(_num_c);
    _dFidci_copy[m].resize(_num_c);
    _d2Fidcidbi[m].resize(_num_c);
    _d2Fidcidbi_copy[m].resize(_num_c);

    for (unsigned int n = 0; n < _num_c; ++n)
    {
      _dFidci[m][n] = &getMaterialPropertyDerivative<Real>(_Fj_names[m], _ci_names[m + n * _num_j]);
      _dFidci_copy[m][n] =
          &declarePropertyDerivative<Real>("cp" + _Fj_names[m], _ci_names[m + n * _num_j]);

      _d2Fidcidbi[m][n].resize(_num_c);
      _d2Fidcidbi_copy[m][n].resize(_num_c);

      for (unsigned int l = 0; l < _num_c; ++l)
      {
        _d2Fidcidbi[m][n][l] = &getMaterialPropertyDerivative<Real>(
            _Fj_names[m], _ci_names[m + n * _num_j], _ci_names[m + l * _num_j]);
        _d2Fidcidbi_copy[m][n][l] = &declarePropertyDerivative<Real>(
            "cp" + _Fj_names[m], _ci_names[m + n * _num_j], _ci_names[m + l * _num_j]);
      }
    }
  }

  // partial derivative of Fi wrt coupled variables, to be passed to kernels
  for (unsigned int m = 0; m < _num_j; ++m)
  {
    _dFidarg[m].resize(_n_args);
    _dFidarg_copy[m].resize(_n_args);

    for (unsigned int n = 0; n < _n_args; ++n)
    {
      _dFidarg[m][n] = &getMaterialPropertyDerivative<Real>(_Fj_names[m], _args_names[n]);
      _dFidarg_copy[m][n] = &declarePropertyDerivative<Real>("cp" + _Fj_names[m], _args_names[n]);
    }
  }

  // second partial derivatives of F1 wrt c1 and another coupled variable, to be passed to kernels
  for (unsigned int m = 0; m < _num_c; ++m)
  {
    _d2F1dc1darg[m].resize(_n_args);
    _d2F1dc1darg_copy[m].resize(_n_args);

    for (unsigned int n = 0; n < _n_args; ++n)
    {
      _d2F1dc1darg[m][n] =
          &getMaterialPropertyDerivative<Real>(_Fj_names[0], _ci_names[m * _num_j], _args_names[n]);
      _d2F1dc1darg_copy[m][n] = &declarePropertyDerivative<Real>(
          "cp" + _Fj_names[0], _ci_names[m * _num_j], _args_names[n]);
    }
  }
}

void
KKSPhaseConcentrationMultiPhaseMaterial::initQpStatefulProperties()
{
  for (unsigned int m = 0; m < _num_c * _num_j; ++m)
    (*_prop_ci[m])[_qp] = _ci_IC[m];
}

void
KKSPhaseConcentrationMultiPhaseMaterial::initialSetup()
{
  for (unsigned int m = 0; m < _num_j; ++m)
    _Fj_mat[m] = &getMaterialByName(_Fj_names[m]);
}

void
KKSPhaseConcentrationMultiPhaseMaterial::computeQpProperties()
{
  // parameters for nested Newton iteration
  NestedSolve::Value<> solution(_num_c * _num_j);

  for (unsigned int m = 0; m < _num_c * _num_j; ++m)
    solution(m) = (*_ci_old[m])[_qp];

  _nested_solve.setAbsoluteTolerance(_abs_tol);
  _nested_solve.setRelativeTolerance(_rel_tol);

  auto compute = [&](const NestedSolve::Value<> & guess,
                     NestedSolve::Value<> & residual,
                     NestedSolve::Jacobian<> & jacobian) {
    for (unsigned int m = 0; m < _num_c * _num_j; ++m)
      (*_prop_ci[m])[_qp] = guess(m);

    for (unsigned int m = 0; m < _num_j; ++m)
      _Fj_mat[m]->computePropertiesAtQp(_qp);

    // assign residual functions
    for (unsigned int m = 0; m < _num_c; ++m)
    {
      for (unsigned int n = 0; n < _num_j - 1; ++n)
        residual(m * _num_j + n) = (*_dFidci[n][m])[_qp] - (*_dFidci[n + 1][m])[_qp];

      residual((m + 1) * _num_j - 1) = -(*_prop_c[m])[_qp];

      for (unsigned int l = 0; l < _num_j; ++l)
        residual((m + 1) * _num_j - 1) += (*_prop_hj[l])[_qp] * (*_prop_ci[m * _num_j + l])[_qp];
    }

    // initialize all terms in jacobian to be zero
    for (unsigned int m = 0; m < _num_j * _num_c; ++m)
    {
      for (unsigned int n = 0; n < _num_j * _num_c; ++n)
        jacobian(m, n) = 0;
    }

    // fill in the non-zero terms in jacobian
    // first assign the terms in jacobian that come from the mu equality derivative equations
    // loop through the constraint equation sets of the mth component
    for (unsigned int m = 0; m < _num_c; ++m)
    {
      // loop through the nth constraint equation in the constraint set of one component
      for (unsigned int n = 0; n < (_num_j - 1); ++n)
      {
        // loop through the lth chain rule terms in one constrain equation
        for (unsigned int l = 0; l < _num_c; ++l)
        {
          jacobian(m * _num_j + n, n + l * _num_j) = (*_d2Fidcidbi[n][m][l])[_qp];
          jacobian(m * _num_j + n, n + l * _num_j + 1) = -(*_d2Fidcidbi[n + 1][m][l])[_qp];
        }
      }
    }

    // then assign the terms in jacobian that come from the concentration conservation equations
    for (unsigned int m = 0; m < _num_c; ++m)
    {
      for (unsigned int n = 0; n < _num_j; ++n)
        jacobian((m + 1) * _num_j - 1, m * _num_j + n) = (*_prop_hj[n])[_qp];
    }
  };

  _nested_solve.nonlinear(solution, compute);
  _iter[_qp] = _nested_solve.getIterations();

  if (_nested_solve.getState() == NestedSolve::State::NOT_CONVERGED)
    mooseError("Nested Newton iteration did not converge.");

  // assign solution to ci
  for (unsigned int m = 0; m < _num_c * _num_j; ++m)
    (*_prop_ci[m])[_qp] = solution[m];

  // assign to the copied parameters to be used in kernels
  for (unsigned int m = 0; m < _num_j; ++m)
    (*_Fi_copy[m])[_qp] = (*_prop_Fi[m])[_qp];

  for (unsigned int m = 0; m < _num_j; ++m)
  {
    for (unsigned int n = 0; n < _num_c; ++n)
    {
      (*_dFidci_copy[m][n])[_qp] = (*_dFidci[m][n])[_qp];

      for (unsigned int l = 0; l < _num_c; ++l)
        (*_d2Fidcidbi_copy[m][n][l])[_qp] = (*_d2Fidcidbi[m][n][l])[_qp];
    }
  }

  for (unsigned int m = 0; m < _num_j; ++m)
  {
    for (unsigned int n = 0; n < _n_args; ++n)
      (*_dFidarg_copy[m][n])[_qp] = (*_dFidarg[m][n])[_qp];
  }

  for (unsigned int m = 0; m < _num_c; ++m)
  {
    for (unsigned int n = 0; n < _num_j; ++n)
      (*_d2F1dc1darg_copy[m][n])[_qp] = (*_d2F1dc1darg[m][n])[_qp];
  }
}
