//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSPhaseConcentrationDerivatives.h"
#include "MatrixTools.h"

registerMooseObject("PhaseFieldApp", KKSPhaseConcentrationDerivatives);

InputParameters
KKSPhaseConcentrationDerivatives::validParams()
{
  InputParameters params = DerivativeMaterialInterface<Material>::validParams();
  params.addClassDescription(
      "Computes the KKS phase concentration derivatives wrt global concentrations and order "
      "parameters, which are used in the chain rules in the KKS kernels. This class is intended to "
      "be used with KKSPhaseConcentrationMaterial.");
  params.addRequiredCoupledVar("global_cs", "The interpolated concentrations c, b, etc");
  params.addRequiredCoupledVar("eta", "Order parameter.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "ci_names",
      "Phase concentrations. The order must match Fa, Fb, and global_cs, for example, c1, "
      "c2, b1, b2, etc");
  params.addRequiredParam<MaterialName>("fa_name", "Fa material object.");
  params.addRequiredParam<MaterialName>("fb_name", "Fb material object.");
  params.addParam<MaterialPropertyName>("h_name", "h", "Switching function h(eta).");
  return params;
}

KKSPhaseConcentrationDerivatives::KKSPhaseConcentrationDerivatives(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _num_c(coupledComponents("global_cs")),
    _c_names(coupledNames("global_cs")),
    _eta_name(getVar("eta", 0)->name()),
    _ci_names(getParam<std::vector<MaterialPropertyName>>("ci_names")),
    _prop_ci(_num_c * 2),
    _dcidb(_num_c),
    _dcideta(_num_c),
    _Fa_name(getParam<MaterialName>("fa_name")),
    _Fb_name(getParam<MaterialName>("fb_name")),
    _d2Fidcidbi(2),
    _prop_h(getMaterialProperty<Real>("h_name")),
    _prop_dh(getMaterialPropertyDerivative<Real>("h_name", _eta_name))
{
  for (const auto m : make_range(_num_c * 2))
    _prop_ci[m] = &getMaterialPropertyByName<Real>(_ci_names[m]);

  for (const auto m : make_range(_num_c))
  {
    _dcideta[m].resize(2);
    _dcidb[m].resize(2);
    for (const auto n : make_range(2))
    {
      // Derivative of phase concentration wrt eta. In _dcideta[m][n], m is the species index of
      // ci, n is the phase index of ci
      _dcideta[m][n] = &declarePropertyDerivative<Real>(_ci_names[m * 2 + n], _eta_name);
      _dcidb[m][n].resize(_num_c);

      // Derivative of phase concentration wrt global concentration. In _dcidb[m][n][l], m is the
      //  species index of ci, n is the phase index of ci, l is the species index of b
      for (const auto l : make_range(_num_c))
        _dcidb[m][n][l] = &declarePropertyDerivative<Real>(_ci_names[m * 2 + n], _c_names[l]);
    }
  }

  /** Second derivative of free energy wrt phase concentrations for use in this material. In
      _d2Fidcidbi[m][n][l], m is phase index of Fi, n is the species index of ci, l is the species
      index of bi.
  */
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
}

void
KKSPhaseConcentrationDerivatives::computeQpProperties()
{
  // declare Jacobian matrix A
  Eigen::MatrixXd A(_num_c * 2, _num_c * 2);

  A.setZero();

  // fill in the non-zero elements in A
  for (const auto m : make_range(_num_c))
  {
    for (const auto n : make_range(_num_c))
    {
      // equal chemical potential derivative equations
      A(m * 2, n * 2) = (*_d2Fidcidbi[0][m][n])[_qp];
      A(m * 2, n * 2 + 1) = -(*_d2Fidcidbi[1][m][n])[_qp];
    }

    // concentration conservation derivative equations
    A(m * 2 + 1, m * 2) = 1 - _prop_h[_qp];
    A(m * 2 + 1, m * 2 + 1) = _prop_h[_qp];
  }

  A = A.inverse();

  // solve linear system of constraint derivatives wrt b for computing dcidb loop through
  // derivatives wrt the ith component; they have the same A, but different k_c
  for (const auto i : make_range(_num_c))
  {
    std::vector<Real> k_c(_num_c * 2);
    std::vector<Real> x_c(_num_c * 2);

    // assign the non-zero elements in k_c
    k_c[i * 2 + 1] = 1;

    // compute x_c
    for (const auto m : make_range(_num_c * 2))
    {
      for (const auto n : make_range(_num_c * 2))
        x_c[m] += A(m, n) * k_c[n];
    }

    // assign the values in x_c to _dcidb
    for (const auto m : make_range(_num_c))
    {
      for (const auto n : make_range(2))
        (*_dcidb[m][n][i])[_qp] = x_c[m * 2 + n];
    }
  }

  // solve linear system of constraint derivatives wrt eta for computing dcideta use the same
  // linear matrix as computing dcidb
  std::vector<Real> k_eta(_num_c * 2);
  std::vector<Real> x_eta(_num_c * 2);

  // fill in k_eta
  for (const auto m : make_range(_num_c))
  {
    k_eta[m * 2] = 0;
    k_eta[m * 2 + 1] = _prop_dh[_qp] * ((*_prop_ci[m * 2])[_qp] - (*_prop_ci[m * 2 + 1])[_qp]);
  }

  // compute x_eta
  for (const auto m : make_range(_num_c * 2))
  {
    for (const auto n : make_range(_num_c * 2))
      x_eta[m] += A(m, n) * k_eta[n];
  }

  // assign the values in x_eta to _dcideta
  for (const auto m : make_range(_num_c))
  {
    for (const auto n : make_range(2))
      (*_dcideta[m][n])[_qp] = x_eta[m * 2 + n];
  }
}
