/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StressDivergence2DTensors.h"
#include "Assembly.h"
#include "ElasticityTensorTools.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<StressDivergence2DTensors>()
{
  InputParameters params = validParams<StressDivergenceTensors>();
  params.addClassDescription("Calculate stress divergence for a 2D problem.");
  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts in. (0 "
      "for x (or r, for axisymmetric), 1 for y (or z, for axisymmetric))");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

StressDivergence2DTensors::StressDivergence2DTensors(const InputParameters & parameters)
  : StressDivergenceTensors(parameters),
    _avg_grad_zz_test(_test.size(), 0.0),
    _avg_grad_zz_phi(_phi.size(), 0.0)
{
}

void
StressDivergence2DTensors::computeResidual()
{
  computeAverageGradientZZTest();
  StressDivergenceTensors::computeResidual();
}

Real
StressDivergence2DTensors::computeQpResidual()
{
  Real div = 0.0;
  if (_component == 0)
  {
    div = _grad_test[_i][_qp](0) * _stress[_qp](0, 0) + getGradientZZTest() * _stress[_qp](2, 2) +
          +_grad_test[_i][_qp](1) * _stress[_qp](0, 1); // stress_{rz}

    // volumetric locking correction
    if (_volumetric_locking_correction)
      div += (_avg_grad_test[_i][0] - _grad_test[_i][_qp](0)) *
             (_stress[_qp](0, 0) + _stress[_qp](1, 1)) / 2.0;
  }
  else if (_component == 1)
  {
    div = _grad_test[_i][_qp](1) * _stress[_qp](1, 1) +
          +_grad_test[_i][_qp](0) * _stress[_qp](1, 0); // stress_{zr}

    // volumetric locking correction
    if (_volumetric_locking_correction)
      div += (_avg_grad_test[_i][1] - _grad_test[_i][_qp](1)) *
             (_stress[_qp](0, 0) + _stress[_qp](1, 1)) / 2.0;
  }
  else
    mooseError("Invalid component for this 2D problem.");

  return div;
}

void
StressDivergence2DTensors::computeJacobian()
{
  computeAverageGradientZZTest();
  computeAverageGradientZZPhi();
  StressDivergenceTensors::computeJacobian();
}

Real
StressDivergence2DTensors::computeQpJacobian()
{
  return calculateJacobian(_component, _component);
}

void
StressDivergence2DTensors::computeOffDiagJacobian(unsigned int jvar)
{
  computeAverageGradientZZTest();
  computeAverageGradientZZPhi();
  StressDivergenceTensors::computeOffDiagJacobian(jvar);
}

Real
StressDivergence2DTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    if (jvar == _disp_var[i])
      return calculateJacobian(_component, i);
  }

  if (_temp_coupled && jvar == _temp_var)
    return 0.0;

  return 0.0;
}

Real
StressDivergence2DTensors::calculateJacobian(unsigned int ivar, unsigned int jvar)
{
  // B^T_i * C * B_j
  RealGradient test, test_z, phi, phi_z;
  Real first_term = 0.0;
  if (ivar == 0)
  // Case grad_test for x, requires contributions from stress_xx, stress_xy, and stress_zz
  {
    test(0) = _grad_test[_i][_qp](0);
    test(1) = _grad_test[_i][_qp](1);
    test_z(2) = getGradientZZTest();
  }
  else // Case grad_test for y
  {
    test(0) = _grad_test[_i][_qp](0);
    test(1) = _grad_test[_i][_qp](1);
  }

  if (jvar == 0)
  {
    phi(0) = _grad_phi[_j][_qp](0);
    phi(1) = _grad_phi[_j][_qp](1);
    phi_z(2) = getGradientZZPhi();
  }
  else
  {
    phi(0) = _grad_phi[_j][_qp](0);
    phi(1) = _grad_phi[_j][_qp](1);
  }

  if (ivar == 0 &&
      jvar == 0) // Case when both phi and test are functions of x and z; requires four terms
  {
    const Real first_sum = ElasticityTensorTools::elasticJacobian(
        _Jacobian_mult[_qp], ivar, jvar, test, phi); // test_x and phi_x
    const Real second_sum = ElasticityTensorTools::elasticJacobian(
        _Jacobian_mult[_qp], 2, 2, test_z, phi_z); // test_z and phi_z
    const Real mixed_sum1 = ElasticityTensorTools::elasticJacobian(
        _Jacobian_mult[_qp], ivar, 2, test, phi_z); // test_x and phi_z
    const Real mixed_sum2 = ElasticityTensorTools::elasticJacobian(
        _Jacobian_mult[_qp], 2, jvar, test_z, phi); // test_z and phi_x

    first_term = first_sum + second_sum + mixed_sum1 + mixed_sum2;
  }
  else if (ivar == 0 && jvar == 1)
  {
    const Real first_sum = ElasticityTensorTools::elasticJacobian(
        _Jacobian_mult[_qp], ivar, jvar, test, phi); // test_x and phi_y
    const Real mixed_sum2 = ElasticityTensorTools::elasticJacobian(
        _Jacobian_mult[_qp], 2, jvar, test_z, phi); // test_z and phi_y

    first_term = first_sum + mixed_sum2;
  }
  else if (ivar == 1 && jvar == 0)
  {
    const Real second_sum = ElasticityTensorTools::elasticJacobian(
        _Jacobian_mult[_qp], ivar, jvar, test, phi); // test_y and phi_x
    const Real mixed_sum1 = ElasticityTensorTools::elasticJacobian(
        _Jacobian_mult[_qp], ivar, 2, test, phi_z); // test_y and phi_z

    first_term = second_sum + mixed_sum1;
  }
  else if (ivar == 1 && jvar == 1)
    first_term = ElasticityTensorTools::elasticJacobian(
        _Jacobian_mult[_qp], ivar, jvar, test, phi); // test_y and phi_y
  else
    mooseError("Invalid component in Jacobian Calculation");

  Real val = 0.0;
  // volumetric locking correction
  // K = Bbar^T_i * C * Bbar^T_j where Bbar = B + Bvol
  // K = B^T_i * C * B_j + Bvol^T_i * C * Bvol_j + B^T_i * C * Bvol_j + Bvol^T_i * C * B_j
  if (_volumetric_locking_correction)
  {
    RealGradient new_test(2, 0.0);
    RealGradient new_phi(2, 0.0);

    new_test(0) = _grad_test[_i][_qp](0);
    new_test(1) = _grad_test[_i][_qp](1);
    new_phi(0) = _grad_phi[_j][_qp](0);
    new_phi(1) = _grad_phi[_j][_qp](1);

    // Bvol^T_i * C * Bvol_j
    Real sum = 0.0;
    for (unsigned i = 0; i < 2; ++i)
      for (unsigned j = 0; j < 2; ++j)
        sum += _Jacobian_mult[_qp](i, i, j, j);
    val += sum * (_avg_grad_test[_i][ivar] - new_test(ivar)) *
           (_avg_grad_phi[_j][jvar] - new_phi(jvar)) / 2.0;

    // B^T_i * C * Bvol_j
    RealGradient sum_2x1;
    sum_2x1(0) = _Jacobian_mult[_qp](0, 0, 0, 0) + _Jacobian_mult[_qp](0, 0, 1, 1);
    sum_2x1(1) = _Jacobian_mult[_qp](1, 1, 0, 0) + _Jacobian_mult[_qp](1, 1, 1, 1);
    if (ivar == 0 && jvar == 0)
      val += sum_2x1(0) * test(0) * (_avg_grad_phi[_j][0] - new_phi(0));
    else if (ivar == 0 && jvar == 1)
      val += sum_2x1(0) * test(0) * (_avg_grad_phi[_j][1] - new_phi(1));
    else if (ivar == 1 && jvar == 0)
      val += sum_2x1(1) * test(1) * (_avg_grad_phi[_j][0] - new_phi(0));
    else
      val += sum_2x1(1) * test(1) * (_avg_grad_phi[_j][1] - new_phi(1));

    // Bvol^T_i * C * B_j
    // val = trace (C * B_j) *(avg_grad_test[_i][ivar] - new_test(ivar))
    if (jvar == 0)
      for (unsigned int i = 0; i < 2; ++i)
        val +=
            (_Jacobian_mult[_qp](i, i, 0, 0) * phi(0) + _Jacobian_mult[_qp](i, i, 0, 1) * phi(1)) *
            (_avg_grad_test[_i][ivar] - new_test(ivar));
    else if (jvar == 1)
      for (unsigned int i = 0; i < 2; ++i)
        val +=
            (_Jacobian_mult[_qp](i, i, 0, 1) * phi(0) + _Jacobian_mult[_qp](i, i, 1, 1) * phi(1)) *
            (_avg_grad_test[_i][ivar] - new_test(ivar));
  }

  return val / 2.0 + first_term;
}

void
StressDivergence2DTensors::computeAverageGradientTest()
{
  // calculate volume averaged value of shape function derivative
  for (_i = 0; _i < _test.size(); ++_i)
  {
    // Two coordinate directions
    _avg_grad_test[_i].resize(2);
    _avg_grad_test[_i][0] = 0.0;
    _avg_grad_test[_i][1] = 0.0;
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      _avg_grad_test[_i][_component] += _grad_test[_i][_qp](_component) * _JxW[_qp] * _coord[_qp];

    _avg_grad_test[_i][_component] /= _current_elem_volume;
  }
}

void
StressDivergence2DTensors::computeAverageGradientPhi()
{
  for (_i = 0; _i < _phi.size(); ++_i)
  {
    // Two coordinate directions
    _avg_grad_phi[_i].resize(2);
    _avg_grad_phi[_i][0] = 0.0;
    _avg_grad_phi[_i][1] = 0.0;
    for (unsigned int component = 0; component < 2; ++component)
    {
      _avg_grad_phi[_i][component] = 0.0;
      for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
        _avg_grad_phi[_i][component] += _grad_phi[_i][_qp](component) * _JxW[_qp] * _coord[_qp];

      _avg_grad_phi[_i][component] /= _current_elem_volume;
    }
  }
}

void
StressDivergence2DTensors::computeAverageGradientZZTest()
{
}

void
StressDivergence2DTensors::computeAverageGradientZZPhi()
{
}

Real
StressDivergence2DTensors::getGradientZZTest()
{
  return 0.0;
}

Real
StressDivergence2DTensors::getGradientZZPhi()
{
  return 0.0;
}

void
StressDivergence2DTensors::precalculateResidual()
{
  _avg_grad_test.resize(_test.size());
  _avg_grad_phi.resize(_phi.size());
  StressDivergenceTensors::precalculateResidual();
}

void
StressDivergence2DTensors::precalculateJacobian()
{
  _avg_grad_test.resize(_test.size());
  _avg_grad_phi.resize(_phi.size());
  StressDivergenceTensors::precalculateJacobian();
}

void
StressDivergence2DTensors::precalculateOffDiagJacobian(unsigned int jvar)
{
  _avg_grad_test.resize(_test.size());
  _avg_grad_phi.resize(_phi.size());
  StressDivergenceTensors::precalculateOffDiagJacobian(jvar);
}
