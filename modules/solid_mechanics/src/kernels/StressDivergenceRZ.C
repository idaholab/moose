/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "StressDivergenceRZ.h"

#include "Assembly.h"
#include "Material.h"
#include "SymmElasticityTensor.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<StressDivergenceRZ>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for r, "
                                        "1 for z)");
  params.addCoupledVar("disp_r", "The r displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature");

  params.set<bool>("use_displaced_mesh") = true;

  return params;
}

StressDivergenceRZ::StressDivergenceRZ(const InputParameters & parameters)
  : Kernel(parameters),
    _stress(getMaterialProperty<SymmTensor>("stress")),
    _Jacobian_mult(getMaterialProperty<SymmElasticityTensor>("Jacobian_mult")),
    _d_stress_dT(getMaterialProperty<SymmTensor>("d_stress_dT")),
    _component(getParam<unsigned int>("component")),
    _rdisp_coupled(isCoupled("disp_r")),
    _zdisp_coupled(isCoupled("disp_z")),
    _temp_coupled(isCoupled("temp")),
    _rdisp_var(_rdisp_coupled ? coupled("disp_r") : 0),
    _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0),
    _temp_var(_temp_coupled ? coupled("temp") : 0),
    _avg_grad_test(_test.size(), std::vector<Real>(3, 0.0)),
    _avg_grad_phi(_phi.size(), std::vector<Real>(3, 0.0)),
    _volumetric_locking_correction(getParam<bool>("volumetric_locking_correction"))
{
}

void
StressDivergenceRZ::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  if (_volumetric_locking_correction)
  {
    // calculate volume averaged value of shape function derivative
    _avg_grad_test.resize(_test.size());
    for (_i = 0; _i < _test.size(); _i++)
    {
      _avg_grad_test[_i].resize(2);
      _avg_grad_test[_i][_component] = 0.0;
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        if (_component == 0)
          _avg_grad_test[_i][_component] +=
              (_grad_test[_i][_qp](_component) + _test[_i][_qp] / _q_point[_qp](0)) * _JxW[_qp] *
              _coord[_qp];
        else
          _avg_grad_test[_i][_component] +=
              _grad_test[_i][_qp](_component) * _JxW[_qp] * _coord[_qp];
      }
      _avg_grad_test[_i][_component] /= _current_elem_volume;
    }
  }

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _save_in)
      var->sys().solution().add_vector(_local_re, var->dofIndices());
  }
}

Real
StressDivergenceRZ::computeQpResidual()
{
  Real div(0);
  if (_component == 0)
  {
    div = _grad_test[_i][_qp](0) * _stress[_qp].xx() +
          //    0                                 * _stress[_qp].yy() +
          +_test[_i][_qp] / _q_point[_qp](0) * _stress[_qp].zz() +
          +_grad_test[_i][_qp](1) * _stress[_qp].xy();

    // volumetric locking correction
    if (_volumetric_locking_correction)
      div += (_avg_grad_test[_i][0] - _grad_test[_i][_qp](0) - _test[_i][_qp] / _q_point[_qp](0)) *
             (_stress[_qp].trace()) / 3.0;
  }
  else if (_component == 1)
  {
    div =
        //    0                      * _stress[_qp].xx() +
        _grad_test[_i][_qp](1) * _stress[_qp].yy() +
        //    0                      * _stress[_qp].zz() +
        _grad_test[_i][_qp](0) * _stress[_qp].xy();

    // volumetric locking correction
    if (_volumetric_locking_correction)
      div += (_avg_grad_test[_i][1] - _grad_test[_i][_qp](1)) * (_stress[_qp].trace()) / 3.0;
  }
  else
  {
    mooseError("Invalid component");
  }

  return div;
}

void
StressDivergenceRZ::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  if (_volumetric_locking_correction)
  {
    // calculate volume averaged value of shape function derivative
    _avg_grad_test.resize(_test.size());
    for (_i = 0; _i < _test.size(); _i++)
    {
      _avg_grad_test[_i].resize(2);
      _avg_grad_test[_i][_component] = 0.0;
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        if (_component == 0)
          _avg_grad_test[_i][_component] +=
              (_grad_test[_i][_qp](_component) + _test[_i][_qp] / _q_point[_qp](0)) * _JxW[_qp] *
              _coord[_qp];
        else
          _avg_grad_test[_i][_component] +=
              _grad_test[_i][_qp](_component) * _JxW[_qp] * _coord[_qp];
      }
      _avg_grad_test[_i][_component] /= _current_elem_volume;
    }

    _avg_grad_phi.resize(_phi.size());
    for (_i = 0; _i < _phi.size(); _i++)
    {
      _avg_grad_phi[_i].resize(2);
      for (unsigned int component = 0; component < 2; component++)
      {
        _avg_grad_phi[_i][component] = 0.0;
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
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

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
      var->sys().solution().add_vector(diag, var->dofIndices());
  }
}

Real
StressDivergenceRZ::computeQpJacobian()
{
  return calculateJacobian(_component, _component);
}

Real
StressDivergenceRZ::calculateJacobian(unsigned int ivar, unsigned int jvar)
{
  // B^T_i * C * B_j
  SymmTensor test, phi;
  if (ivar == 0)
  {
    test.xx() = _grad_test[_i][_qp](0);
    test.xy() = 0.5 * _grad_test[_i][_qp](1);
    test.zz() = _test[_i][_qp] / _q_point[_qp](0);
  }
  else
  {
    test.xy() = 0.5 * _grad_test[_i][_qp](0);
    test.yy() = _grad_test[_i][_qp](1);
  }
  if (jvar == 0)
  {
    phi.xx() = _grad_phi[_j][_qp](0);
    phi.xy() = 0.5 * _grad_phi[_j][_qp](1);
    phi.zz() = _phi[_j][_qp] / _q_point[_qp](0);
  }
  else
  {
    phi.xy() = 0.5 * _grad_phi[_j][_qp](0);
    phi.yy() = _grad_phi[_j][_qp](1);
  }

  SymmTensor tmp(_Jacobian_mult[_qp] * phi);
  Real first_term(test.doubleContraction(tmp));
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
    val += _Jacobian_mult[_qp].sum_3x3() * (_avg_grad_test[_i][ivar] - new_test(ivar)) *
           (_avg_grad_phi[_j][jvar] - new_phi(jvar)) / 3.0;

    // B^T_i * C * Bvol_j
    RealGradient sum_3x1 = _Jacobian_mult[_qp].sum_3x1();
    if (ivar == 0 && jvar == 0)
      val +=
          (sum_3x1(0) * test.xx() + sum_3x1(2) * test.zz()) * (_avg_grad_phi[_j][0] - new_phi(0));
    else if (ivar == 0 && jvar == 1)
      val +=
          (sum_3x1(0) * test.xx() + sum_3x1(2) * test.zz()) * (_avg_grad_phi[_j][1] - new_phi(1));
    else if (ivar == 1 && jvar == 0)
      val += sum_3x1(1) * test.yy() * (_avg_grad_phi[_j][0] - new_phi(0));
    else
      val += sum_3x1(1) * test.yy() * (_avg_grad_phi[_j][1] - new_phi(1));

    // Bvol^T_i * C * B_j
    // tmp = C * B_j from above
    if (ivar == 0)
      val += (tmp.xx() + tmp.yy() + tmp.zz()) * (_avg_grad_test[_i][0] - new_test(0));
    else
      val += (tmp.xx() + tmp.yy() + tmp.zz()) * (_avg_grad_test[_i][1] - new_test(1));
  }
  return val / 3.0 + first_term;
}

void
StressDivergenceRZ::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    if (_volumetric_locking_correction)
    {
      // calculate volume averaged value of shape function derivative
      _avg_grad_test.resize(_test.size());
      for (_i = 0; _i < _test.size(); _i++)
      {
        _avg_grad_test[_i].resize(2);
        _avg_grad_test[_i][_component] = 0.0;
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        {
          if (_component == 0)
            _avg_grad_test[_i][_component] +=
                (_grad_test[_i][_qp](_component) + _test[_i][_qp] / _q_point[_qp](0)) * _JxW[_qp] *
                _coord[_qp];
          else
            _avg_grad_test[_i][_component] +=
                _grad_test[_i][_qp](_component) * _JxW[_qp] * _coord[_qp];
        }
        _avg_grad_test[_i][_component] /= _current_elem_volume;
      }

      _avg_grad_phi.resize(_phi.size());
      for (_i = 0; _i < _phi.size(); _i++)
      {
        _avg_grad_phi[_i].resize(3);
        for (unsigned int component = 0; component < 2; component++)
        {
          _avg_grad_phi[_i][component] = 0.0;
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          {
            if (component == 0)
              _avg_grad_phi[_i][component] +=
                  (_grad_phi[_i][_qp](component) + _phi[_i][_qp] / _q_point[_qp](0)) * _JxW[_qp] *
                  _coord[_qp];
            else
              _avg_grad_phi[_i][component] +=
                  _grad_phi[_i][_qp](component) * _JxW[_qp] * _coord[_qp];
          }

          _avg_grad_phi[_i][component] /= _current_elem_volume;
        }
      }
    }

    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
  }
}

Real
StressDivergenceRZ::computeQpOffDiagJacobian(unsigned int jvar)
{

  if (_rdisp_coupled && jvar == _rdisp_var)
  {
    return calculateJacobian(_component, 0);
  }
  else if (_zdisp_coupled && jvar == _zdisp_var)
  {
    return calculateJacobian(_component, 1);
  }
  else if (_temp_coupled && jvar == _temp_var)
  {
    SymmTensor test;
    if (_component == 0)
    {
      test.xx() = _grad_test[_i][_qp](0);
      test.xy() = 0.5 * _grad_test[_i][_qp](1);
      test.zz() = _test[_i][_qp] / _q_point[_qp](0);
    }
    else
    {
      test.xy() = 0.5 * _grad_test[_i][_qp](0);
      test.yy() = _grad_test[_i][_qp](1);
    }
    return _d_stress_dT[_qp].doubleContraction(test) * _phi[_j][_qp];
  }

  return 0;
}
