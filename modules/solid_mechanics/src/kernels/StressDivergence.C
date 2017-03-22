/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "StressDivergence.h"

#include "Assembly.h"
#include "Material.h"
#include "MooseMesh.h"
#include "SymmElasticityTensor.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<StressDivergence>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for x, "
                                        "1 for y, 2 for z)");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature");
  params.addParam<Real>("zeta", 0.0, "Stiffness dependent Rayleigh damping coefficient");
  params.addParam<Real>("alpha", 0.0, "alpha parameter required for HHT time integration");
  params.addParam<std::string>(
      "appended_property_name", "", "Name appended to material properties to make them unique");
  params.addParam<bool>("volumetric_locking_correction",
                        true,
                        "Set to false to turn off volumetric locking correction");

  params.set<bool>("use_displaced_mesh") = true;

  return params;
}

StressDivergence::StressDivergence(const InputParameters & parameters)
  : Kernel(parameters),
    _stress_older(getMaterialPropertyOlder<SymmTensor>(
        "stress" + getParam<std::string>("appended_property_name"))),
    _stress_old(getMaterialPropertyOld<SymmTensor>(
        "stress" + getParam<std::string>("appended_property_name"))),
    _stress(getMaterialProperty<SymmTensor>("stress" +
                                            getParam<std::string>("appended_property_name"))),
    _Jacobian_mult(getMaterialProperty<SymmElasticityTensor>(
        "Jacobian_mult" + getParam<std::string>("appended_property_name"))),
    _d_stress_dT(getMaterialProperty<SymmTensor>("d_stress_dT" +
                                                 getParam<std::string>("appended_property_name"))),
    _component(getParam<unsigned int>("component")),
    _xdisp_coupled(isCoupled("disp_x")),
    _ydisp_coupled(isCoupled("disp_y")),
    _zdisp_coupled(isCoupled("disp_z")),
    _temp_coupled(isCoupled("temp")),
    _xdisp_var(_xdisp_coupled ? coupled("disp_x") : 0),
    _ydisp_var(_ydisp_coupled ? coupled("disp_y") : 0),
    _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0),
    _temp_var(_temp_coupled ? coupled("temp") : 0),
    _zeta(getParam<Real>("zeta")),
    _alpha(getParam<Real>("alpha")),
    _avg_grad_test(_test.size(), std::vector<Real>(3, 0.0)),
    _avg_grad_phi(_phi.size(), std::vector<Real>(3, 0.0)),
    _volumetric_locking_correction(getParam<bool>("volumetric_locking_correction"))
{
}

void
StressDivergence::computeResidual()
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
      _avg_grad_test[_i].resize(3);
      _avg_grad_test[_i][_component] = 0.0;
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _avg_grad_test[_i][_component] += _grad_test[_i][_qp](_component) * _JxW[_qp] * _coord[_qp];

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
StressDivergence::computeQpResidual()
{
  Real residual = 0.0;
  if ((_dt > 0) && ((_zeta != 0) || (_alpha != 0)))
  {
    residual = _stress[_qp].rowDot(_component, _grad_test[_i][_qp]) *
                   (1 + _alpha + (1 + _alpha) * _zeta / _dt) -
               (_alpha + (1 + 2 * _alpha) * _zeta / _dt) *
                   _stress_old[_qp].rowDot(_component, _grad_test[_i][_qp]) +
               (_alpha * _zeta / _dt) * _stress_older[_qp].rowDot(_component, _grad_test[_i][_qp]);

    // volumetric locking correction
    if (_volumetric_locking_correction)
      residual += (_stress[_qp].trace() * (1 + _alpha + (1 + _alpha) * _zeta / _dt) -
                   (_alpha + (1 + 2 * _alpha) * _zeta / _dt) * _stress_old[_qp].trace() +
                   (_alpha * _zeta / _dt) * _stress_older[_qp].trace()) /
                  3.0 * (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component));
  }
  else
  {
    residual += _stress[_qp].rowDot(_component, _grad_test[_i][_qp]);

    // volumetric locking correction
    if (_volumetric_locking_correction)
      residual += _stress[_qp].trace() / 3.0 *
                  (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component));
  }
  return residual;
}

void
StressDivergence::computeJacobian()
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
      _avg_grad_test[_i].resize(3);
      _avg_grad_test[_i][_component] = 0.0;
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _avg_grad_test[_i][_component] += _grad_test[_i][_qp](_component) * _JxW[_qp] * _coord[_qp];

      _avg_grad_test[_i][_component] /= _current_elem_volume;
    }

    _avg_grad_phi.resize(_phi.size());
    for (_i = 0; _i < _phi.size(); _i++)
    {
      _avg_grad_phi[_i].resize(3);
      for (unsigned int component = 0; component < _mesh.dimension(); component++)
      {
        _avg_grad_phi[_i][component] = 0.0;
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          _avg_grad_phi[_i][component] += _grad_phi[_i][_qp](component) * _JxW[_qp] * _coord[_qp];

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
StressDivergence::computeQpJacobian()
{
  Real sum_C3x3 = _Jacobian_mult[_qp].sum_3x3();
  RealGradient sum_C3x1 = _Jacobian_mult[_qp].sum_3x1();

  Real jacobian = 0.0;
  // B^T_i * C * B_j
  jacobian += _Jacobian_mult[_qp].stiffness(
      _component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp]); // B^T_i * C *B_j

  if (_volumetric_locking_correction)
  {
    // jacobian = Bbar^T_i * C * Bbar_j where Bbar = B + Bvol
    // jacobian = B^T_i * C * B_j + Bvol^T_i * C * Bvol_j +  Bvol^T_i * C * B_j + B^T_i * C * Bvol_j

    // Bvol^T_i * C * Bvol_j
    jacobian += sum_C3x3 * (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component)) *
                (_avg_grad_phi[_j][_component] - _grad_phi[_j][_qp](_component)) / 9.0;

    // B^T_i * C * Bvol_j
    jacobian += sum_C3x1(_component) * _grad_test[_i][_qp](_component) *
                (_avg_grad_phi[_j][_component] - _grad_phi[_j][_qp](_component)) / 3.0;

    // Bvol^T_i * C * B_j
    SymmTensor phi;
    if (_component == 0)
    {
      phi.xx() = _grad_phi[_j][_qp](0);
      phi.xy() = _grad_phi[_j][_qp](1);
      phi.xz() = _grad_phi[_j][_qp](2);
    }
    else if (_component == 1)
    {
      phi.yy() = _grad_phi[_j][_qp](1);
      phi.xy() = _grad_phi[_j][_qp](0);
      phi.yz() = _grad_phi[_j][_qp](2);
    }
    else if (_component == 2)
    {
      phi.zz() = _grad_phi[_j][_qp](2);
      phi.xz() = _grad_phi[_j][_qp](0);
      phi.yz() = _grad_phi[_j][_qp](1);
    }

    SymmTensor tmp(_Jacobian_mult[_qp] * phi);

    jacobian += (tmp.xx() + tmp.yy() + tmp.zz()) *
                (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component)) / 3.0;
  }

  if (_dt > 0)
    return jacobian * (1 + _alpha + _zeta / _dt);
  else
    return jacobian;
}

void
StressDivergence::computeOffDiagJacobian(unsigned int jvar)
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
        _avg_grad_test[_i].resize(3);
        _avg_grad_test[_i][_component] = 0.0;
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          _avg_grad_test[_i][_component] +=
              _grad_test[_i][_qp](_component) * _JxW[_qp] * _coord[_qp];

        _avg_grad_test[_i][_component] /= _current_elem_volume;
      }

      _avg_grad_phi.resize(_phi.size());
      for (_i = 0; _i < _phi.size(); _i++)
      {
        _avg_grad_phi[_i].resize(3);
        for (unsigned int component = 0; component < _mesh.dimension(); component++)
        {
          _avg_grad_phi[_i][component] = 0.0;
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
            _avg_grad_phi[_i][component] += _grad_phi[_i][_qp](component) * _JxW[_qp] * _coord[_qp];

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
StressDivergence::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int coupled_component = 0;

  bool active = false;

  if (_xdisp_coupled && jvar == _xdisp_var)
  {
    coupled_component = 0;
    active = true;
  }
  else if (_ydisp_coupled && jvar == _ydisp_var)
  {
    coupled_component = 1;
    active = true;
  }
  else if (_zdisp_coupled && jvar == _zdisp_var)
  {
    coupled_component = 2;
    active = true;
  }

  if (active)
  {
    Real sum_C3x3 = _Jacobian_mult[_qp].sum_3x3();
    RealGradient sum_C3x1 = _Jacobian_mult[_qp].sum_3x1();
    Real jacobian = 0.0;
    jacobian += _Jacobian_mult[_qp].stiffness(
        _component, coupled_component, _grad_test[_i][_qp], _grad_phi[_j][_qp]); // B^T_i * C *B_j

    if (_volumetric_locking_correction)
    {
      // jacobian = Bbar^T_i * C * Bbar_j where Bbar = B + Bvol
      // jacobian = B^T_i * C * B_j + Bvol^T_i * C * Bvol_j +  Bvol^T_i * C * B_j + B^T_i * C *
      // Bvol_j

      // Bvol^T_i * C * Bvol_j
      jacobian += sum_C3x3 * (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component)) *
                  (_avg_grad_phi[_j][coupled_component] - _grad_phi[_j][_qp](coupled_component)) /
                  9.0;

      // B^T_i * C * Bvol_j
      jacobian += sum_C3x1(_component) * _grad_test[_i][_qp](_component) *
                  (_avg_grad_phi[_j][coupled_component] - _grad_phi[_j][_qp](coupled_component)) /
                  3.0;

      // Bvol^T_i * C * B_i
      SymmTensor phi;
      if (coupled_component == 0)
      {
        phi.xx() = _grad_phi[_j][_qp](0);
        phi.xy() = _grad_phi[_j][_qp](1);
        phi.xz() = _grad_phi[_j][_qp](2);
      }
      else if (coupled_component == 1)
      {
        phi.yy() = _grad_phi[_j][_qp](1);
        phi.xy() = _grad_phi[_j][_qp](0);
        phi.yz() = _grad_phi[_j][_qp](2);
      }
      else if (coupled_component == 2)
      {
        phi.zz() = _grad_phi[_j][_qp](2);
        phi.xz() = _grad_phi[_j][_qp](0);
        phi.yz() = _grad_phi[_j][_qp](1);
      }

      SymmTensor tmp(_Jacobian_mult[_qp] * phi);

      jacobian += (tmp.xx() + tmp.yy() + tmp.zz()) *
                  (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component)) / 3.0;
    }

    if (_dt > 0)
      return jacobian * (1 + _alpha + _zeta / _dt);
    else
      return jacobian;
  }

  if (_temp_coupled && jvar == _temp_var)
    return _d_stress_dT[_qp].rowDot(_component, _grad_test[_i][_qp]) * _phi[_j][_qp];

  return 0;
}
