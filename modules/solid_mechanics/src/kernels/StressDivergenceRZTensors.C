//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressDivergenceRZTensors.h"
#include "Assembly.h"
#include "ElasticityTensorTools.h"
#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", StressDivergenceRZTensors);

InputParameters
StressDivergenceRZTensors::validParams()
{
  InputParameters params = StressDivergenceTensors::validParams();
  params.addClassDescription(
      "Calculate stress divergence for an axisymmetric problem in cylindrical coordinates.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component < 2",
      "An integer corresponding to the direction the variable this kernel acts in. (0 "
      "refers to the radial and 1 to the axial displacement.)");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

StressDivergenceRZTensors::StressDivergenceRZTensors(const InputParameters & parameters)
  : StressDivergenceTensors(parameters)
{
}

void
StressDivergenceRZTensors::initialSetup()
{
  // check if any of the eigenstrains provide derivatives wrt variables that are not coupled
  for (auto eigenstrain_name : getParam<std::vector<MaterialPropertyName>>("eigenstrain_names"))
    validateNonlinearCoupling<RankTwoTensor>(eigenstrain_name);

  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system in the Problem block must be set to RZ for axisymmetric "
               "geometries.");

  if (getBlockCoordSystem() == Moose::COORD_RZ && _fe_problem.getAxisymmetricRadialCoord() != 0)
    mooseError("rz_coord_axis=Y is the only supported option for StressDivergenceRZTensors");
}

Real
StressDivergenceRZTensors::computeQpResidual()
{
  Real div = 0.0;
  if (_component == 0)
  {
    div = _grad_test[_i][_qp](0) * _stress[_qp](0, 0) +
          +(_test[_i][_qp] / _q_point[_qp](0)) * _stress[_qp](2, 2) +
          +_grad_test[_i][_qp](1) * _stress[_qp](0, 1); // stress_{rz}

    // volumetric locking correction
    if (_volumetric_locking_correction)
      div += (_avg_grad_test[_i][0] - _grad_test[_i][_qp](0) - _test[_i][_qp] / _q_point[_qp](0)) *
             (_stress[_qp].trace()) / 3.0;
  }
  else if (_component == 1)
  {
    div = _grad_test[_i][_qp](1) * _stress[_qp](1, 1) +
          +_grad_test[_i][_qp](0) * _stress[_qp](1, 0); // stress_{zr}

    // volumetric locking correction
    if (_volumetric_locking_correction)
      div += (_avg_grad_test[_i][1] - _grad_test[_i][_qp](1)) * (_stress[_qp].trace()) / 3.0;
  }
  else
    mooseError("Invalid component for this AxisymmetricRZ problem.");

  return div;
}

Real
StressDivergenceRZTensors::computeQpJacobian()
{
  return calculateJacobian(_component, _component);
}

Real
StressDivergenceRZTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    if (jvar == _disp_var[i])
      return calculateJacobian(_component, i);

  // bail out if jvar is not coupled
  if (getJvarMap()[jvar] < 0)
    return 0.0;

  // off-diagonal Jacobian with respect to any other coupled variable
  const unsigned int cvar = mapJvarToCvar(jvar);
  RankTwoTensor total_deigenstrain;
  for (const auto deigenstrain_darg : _deigenstrain_dargs[cvar])
    total_deigenstrain += (*deigenstrain_darg)[_qp];

  Real jac = 0.0;
  if (_component == 0)
  {
    for (unsigned k = 0; k < LIBMESH_DIM; ++k)
      for (unsigned l = 0; l < LIBMESH_DIM; ++l)
        jac -= (_grad_test[_i][_qp](0) * _Jacobian_mult[_qp](0, 0, k, l) +
                _test[_i][_qp] / _q_point[_qp](0) * _Jacobian_mult[_qp](2, 2, k, l) +
                _grad_test[_i][_qp](1) * _Jacobian_mult[_qp](0, 1, k, l)) *
               total_deigenstrain(k, l);
    return jac * _phi[_j][_qp];
  }
  else if (_component == 1)
  {
    for (unsigned k = 0; k < LIBMESH_DIM; ++k)
      for (unsigned l = 0; l < LIBMESH_DIM; ++l)
        jac -= (_grad_test[_i][_qp](1) * _Jacobian_mult[_qp](1, 1, k, l) +
                _grad_test[_i][_qp](0) * _Jacobian_mult[_qp](1, 0, k, l)) *
               total_deigenstrain(k, l);
    return jac * _phi[_j][_qp];
  }

  return 0.0;
}

Real
StressDivergenceRZTensors::calculateJacobian(unsigned int ivar, unsigned int jvar)
{
  // B^T_i * C * B_j
  RealGradient test, test_z, phi, phi_z;
  Real first_term = 0.0;
  if (ivar == 0) // Case grad_test for x, requires contributions from stress_xx, stress_xy, and
                 // stress_zz
  {
    test(0) = _grad_test[_i][_qp](0);
    test(1) = _grad_test[_i][_qp](1);
    test_z(2) = _test[_i][_qp] / _q_point[_qp](0);
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
    phi_z(2) = _phi[_j][_qp] / _q_point[_qp](0);
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

    new_test(0) = _grad_test[_i][_qp](0) + _test[_i][_qp] / _q_point[_qp](0);
    new_test(1) = _grad_test[_i][_qp](1);
    new_phi(0) = _grad_phi[_j][_qp](0) + _phi[_j][_qp] / _q_point[_qp](0);
    new_phi(1) = _grad_phi[_j][_qp](1);

    // Bvol^T_i * C * Bvol_j
    val += _Jacobian_mult[_qp].sum3x3() * (_avg_grad_test[_i][ivar] - new_test(ivar)) *
           (_avg_grad_phi[_j][jvar] - new_phi(jvar)) / 3.0;

    // B^T_i * C * Bvol_j
    RealGradient sum_3x1 = _Jacobian_mult[_qp].sum3x1();
    if (ivar == 0 && jvar == 0)
      val += (sum_3x1(0) * test(0) + sum_3x1(2) * test_z(2)) * (_avg_grad_phi[_j][0] - new_phi(0));
    else if (ivar == 0 && jvar == 1)
      val += (sum_3x1(0) * test(0) + sum_3x1(2) * test_z(2)) * (_avg_grad_phi[_j][1] - new_phi(1));
    else if (ivar == 1 && jvar == 0)
      val += sum_3x1(1) * test(1) * (_avg_grad_phi[_j][0] - new_phi(0));
    else
      val += sum_3x1(1) * test(1) * (_avg_grad_phi[_j][1] - new_phi(1));

    // Bvol^T_i * C * B_j
    // val = trace (C * B_j) *(avg_grad_test[_i][ivar] - new_test(ivar))
    if (jvar == 0)
      for (unsigned int i = 0; i < 3; ++i)
        val +=
            (_Jacobian_mult[_qp](i, i, 0, 0) * phi(0) + _Jacobian_mult[_qp](i, i, 0, 1) * phi(1) +
             _Jacobian_mult[_qp](i, i, 2, 2) * phi_z(2)) *
            (_avg_grad_test[_i][ivar] - new_test(ivar));
    else if (jvar == 1)
      for (unsigned int i = 0; i < 3; ++i)
        val +=
            (_Jacobian_mult[_qp](i, i, 0, 1) * phi(0) + _Jacobian_mult[_qp](i, i, 1, 1) * phi(1)) *
            (_avg_grad_test[_i][ivar] - new_test(ivar));
  }

  return val / 3.0 + first_term;
}

void
StressDivergenceRZTensors::computeAverageGradientTest()
{
  // calculate volume averaged value of shape function derivative
  _avg_grad_test.resize(_test.size());
  for (_i = 0; _i < _test.size(); ++_i)
  {
    _avg_grad_test[_i].resize(2);
    _avg_grad_test[_i][_component] = 0.0;
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      if (_component == 0)
        _avg_grad_test[_i][_component] +=
            (_grad_test[_i][_qp](_component) + _test[_i][_qp] / _q_point[_qp](0)) * _JxW[_qp] *
            _coord[_qp];
      else
        _avg_grad_test[_i][_component] += _grad_test[_i][_qp](_component) * _JxW[_qp] * _coord[_qp];
    }
    _avg_grad_test[_i][_component] /= _current_elem_volume;
  }
}

void
StressDivergenceRZTensors::computeAverageGradientPhi()
{
  _avg_grad_phi.resize(_phi.size());
  for (_i = 0; _i < _phi.size(); ++_i)
  {
    _avg_grad_phi[_i].resize(2);
    for (unsigned int component = 0; component < 2; ++component)
    {
      _avg_grad_phi[_i][component] = 0.0;
      for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      {
        if (component == 0)
          _avg_grad_phi[_i][component] +=
              (_grad_phi[_i][_qp](component) + _phi[_i][_qp] / _q_point[_qp](0)) * _JxW[_qp] *
              _coord[_qp];
        else
          _avg_grad_phi[_i][component] += _grad_phi[_i][_qp](component) * _JxW[_qp] * _coord[_qp];
      }
      _avg_grad_phi[_i][component] /= _current_elem_volume;
    }
  }
}
