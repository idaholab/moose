//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSPhaseConcentrationMultiPhaseDerivatives.h"
#include "MatrixTools.h"

registerMooseObject("PhaseFieldApp", KKSPhaseConcentrationMultiPhaseDerivatives);

InputParameters
KKSPhaseConcentrationMultiPhaseDerivatives::validParams()
{
  InputParameters params = DerivativeMaterialInterface<Material>::validParams();
  params.addClassDescription(
      "Computes the KKS phase concentration derivatives wrt global concentrations and order "
      "parameters, which are used for the chain rule in the KKS kernels. This class is intended to "
      "be used with KKSPhaseConcentrationMultiPhaseMaterial.");
  params.addRequiredCoupledVar("global_cs", "The interpolated concentrations c, b, etc");
  params.addRequiredCoupledVar("all_etas", "Order parameters.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "ci_names",
      "Phase concentrations. They must have the same order as Fj_names and global_cs, for "
      "example, c1, c2, b1, b2.");
  params.addRequiredParam<std::vector<MaterialName>>(
      "Fj_material", "Free energy material objects in the same order as all_etas.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "witching functions in the same order as all_etas.");
  return params;
}

KKSPhaseConcentrationMultiPhaseDerivatives::KKSPhaseConcentrationMultiPhaseDerivatives(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _num_c(coupledComponents("global_cs")),
    _c_names(coupledNames("global_cs")),
    _eta_names(coupledNames("all_etas")),
    _num_j(coupledComponents("all_etas")),
    _prop_ci(_num_c * _num_j),
    _ci_names(getParam<std::vector<MaterialPropertyName>>("ci_names")),
    _dcidetaj(_num_c),
    _dcidb(_num_c),
    _Fj_names(getParam<std::vector<MaterialName>>("Fj_material")),
    _d2Fidcidbi(_num_j),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_hj(_num_j),
    _dhjdetai(_num_j)

{
  for (unsigned int m = 0; m < _num_c * _num_j; ++m)
    _prop_ci[m] = &getMaterialPropertyByName<Real>(_ci_names[m]);

  for (unsigned int m = 0; m < _num_c; ++m)
  {
    _dcidb[m].resize(_num_j);
    _dcidetaj[m].resize(_num_j);

    for (unsigned int n = 0; n < _num_j; ++n)
    {
      _dcidb[m][n].resize(_num_c);
      _dcidetaj[m][n].resize(_num_j);

      // Derivative of phase concentration wrt global concentration. In _dcidb[m][n][l],
      // m is the species index of ci, n is the phase index of ci, and l is the species index of b
      for (unsigned int l = 0; l < _num_c; ++l)
        _dcidb[m][n][l] = &declarePropertyDerivative<Real>(_ci_names[n + m * _num_j], _c_names[l]);

      // Derivative of phase concentration wrt eta. In _dcidetaj[m][n][l], m is the species
      // index of ci, n is the phase index of ci, and l is the phase of etaj
      for (unsigned int l = 0; l < _num_j; ++l)
        _dcidetaj[m][n][l] =
            &declarePropertyDerivative<Real>(_ci_names[n + m * _num_j], _eta_names[l]);
    }
  }

  // Second derivative of free energy wrt phase concentrations for use in this material. In
  // _d2Fidcidbi[m][n][l], m is phase index of Fi, n is the species index of ci, l is the species
  // index of bi.
  for (unsigned int m = 0; m < _num_j; ++m)
  {
    _d2Fidcidbi[m].resize(_num_c);

    for (unsigned int n = 0; n < _num_c; ++n)
    {
      _d2Fidcidbi[m][n].resize(_num_c);

      for (unsigned int l = 0; l < _num_c; ++l)
        _d2Fidcidbi[m][n][l] = &getMaterialPropertyDerivative<Real>(
            _Fj_names[m], _ci_names[m + n * _num_j], _ci_names[m + l * _num_j]);
    }
  }

  for (unsigned int m = 0; m < _num_j; ++m)
  {
    _prop_hj[m] = &getMaterialPropertyByName<Real>(_hj_names[m]);

    _dhjdetai[m].resize(_num_j);

    for (unsigned int n = 0; n < _num_j; ++n)
      _dhjdetai[m][n] = &getMaterialPropertyDerivative<Real>(_hj_names[m], _eta_names[n]);
  }
}

void
KKSPhaseConcentrationMultiPhaseDerivatives::computeQpProperties()
{
  // declare Jacobian matrix A
  Eigen::MatrixXd A(_num_c * _num_j, _num_c * _num_j);

  // initialize all elements in A to be zero
  for (unsigned int m = 0; m < _num_j * _num_c; ++m)
  {
    for (unsigned int n = 0; n < _num_j * _num_c; ++n)
      A(m, n) = 0;
  }

  // fill in the non-zero elements in A
  for (unsigned int m = 0; m < _num_c; ++m)
  {
    // equal chemical potential derivative equations
    for (unsigned int n = 0; n < (_num_j - 1); ++n)
    {
      for (unsigned int l = 0; l < _num_c; ++l)
      {
        A(m * _num_j + n, n + l * _num_j) = (*_d2Fidcidbi[n][m][l])[_qp];
        A(m * _num_j + n, n + l * _num_j + 1) = -(*_d2Fidcidbi[n + 1][m][l])[_qp];
      }
    }

    // concentration conservation derivative equations
    for (unsigned int n = 0; n < _num_j; ++n)
      A((m + 1) * _num_j - 1, m * _num_j + n) = (*_prop_hj[n])[_qp];
  }

  A = A.inverse();

  // solve linear system of constraint derivatives wrt b for computing dcidb
  // loop through derivatives wrt the ith component; they have the same A, but different k_c
  for (unsigned int i = 0; i < _num_c; ++i)
  {
    std::vector<Real> k_c(_num_j * _num_c);
    std::vector<Real> x_c(_num_j * _num_c);

    // assign non-zero elements in k_c
    k_c[i * _num_j + _num_j - 1] = 1;

    // compute x_c
    for (unsigned int m = 0; m < (_num_j * _num_c); ++m)
    {
      for (unsigned int n = 0; n < (_num_j * _num_c); ++n)
        x_c[m] += A(m, n) * k_c[n];
    }

    // assign the values in x_c to _dcidb
    for (unsigned int m = 0; m < _num_c; ++m)
    {
      for (unsigned int n = 0; n < _num_j; ++n)
        (*_dcidb[m][n][i])[_qp] = x_c[m * _num_j + n];
    }
  }

  // solve linear system of constraint derivatives wrt eta for computing dcidetaj
  // use the same linear matrix as computing dcidb
  for (unsigned int i = 0; i < _num_j; ++i)
  {
    std::vector<Real> k_eta(_num_j * _num_c);
    std::vector<Real> x_eta(_num_j * _num_c);

    // assign non-zero elements in k_eta
    for (unsigned int m = 0; m < _num_c; ++m)
    {
      Real sum = 0.0;

      for (unsigned int n = 0; n < _num_j; ++n)
        sum += (*_dhjdetai[n][i])[_qp] * (*_prop_ci[m * _num_j + n])[_qp];

      k_eta[m * _num_j + _num_j - 1] = -sum;
    }

    // compute x_eta
    for (unsigned int m = 0; m < (_num_j * _num_c); ++m)
    {
      for (unsigned int n = 0; n < (_num_j * _num_c); ++n)
        x_eta[m] += A(m, n) * k_eta[n];
    }

    // assign the values in x_eta to _dcidetaj
    for (unsigned int m = 0; m < _num_c; ++m)
    {
      for (unsigned int n = 0; n < _num_j; ++n)
        (*_dcidetaj[m][n][i])[_qp] = x_eta[m * _num_j + n];
    }
  }
}
