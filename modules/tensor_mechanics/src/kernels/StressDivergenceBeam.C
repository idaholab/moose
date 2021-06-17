//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressDivergenceBeam.h"

// MOOSE includes
#include "Assembly.h"
#include "Material.h"
#include "MooseVariable.h"
#include "SystemBase.h"
#include "RankTwoTensor.h"
#include "NonlinearSystem.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", StressDivergenceBeam);

InputParameters
StressDivergenceBeam::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Quasi-static and dynamic stress divergence kernel for Beam element");
  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the direction "
      "the variable this kernel acts in. (0 for disp_x, "
      "1 for disp_y, 2 for disp_z, 3 for rot_x, 4 for rot_y and 5 for rot_z)");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addRequiredCoupledVar(
      "rotations", "The rotations appropriate for the simulation geometry and coordinate system");
  params.addParam<MaterialPropertyName>(
      "zeta",
      0.0,
      "Name of material property or a constant real number defining the zeta parameter for the "
      "Rayleigh damping.");
  params.addRangeCheckedParam<Real>(
      "alpha", 0.0, "alpha >= -0.3333 & alpha <= 0.0", "alpha parameter for HHT time integration");

  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

StressDivergenceBeam::StressDivergenceBeam(const InputParameters & parameters)
  : Kernel(parameters),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _nrot(coupledComponents("rotations")),
    _rot_var(_nrot),
    _force(getMaterialPropertyByName<RealVectorValue>("forces")),
    _moment(getMaterialPropertyByName<RealVectorValue>("moments")),
    _K11(getMaterialPropertyByName<RankTwoTensor>("Jacobian_11")),
    _K22(getMaterialPropertyByName<RankTwoTensor>("Jacobian_22")),
    _K22_cross(getMaterialPropertyByName<RankTwoTensor>("Jacobian_22_cross")),
    _K21_cross(getMaterialPropertyByName<RankTwoTensor>("Jacobian_12")),
    _K21(getMaterialPropertyByName<RankTwoTensor>("Jacobian_21")),
    _original_length(getMaterialPropertyByName<Real>("original_length")),
    _total_rotation(getMaterialPropertyByName<RankTwoTensor>("total_rotation")),
    _zeta(getMaterialProperty<Real>("zeta")),
    _alpha(getParam<Real>("alpha")),
    _isDamped(getParam<MaterialPropertyName>("zeta") != "0.0" || std::abs(_alpha) > 0.0),
    _force_old(_isDamped ? &getMaterialPropertyOld<RealVectorValue>("forces") : nullptr),
    _moment_old(_isDamped ? &getMaterialPropertyOld<RealVectorValue>("moments") : nullptr),
    _total_rotation_old(_isDamped ? &getMaterialPropertyOld<RankTwoTensor>("total_rotation")
                                  : nullptr),
    _force_older(std::abs(_alpha) > 0.0 ? &getMaterialPropertyOlder<RealVectorValue>("forces")
                                        : nullptr),
    _moment_older(std::abs(_alpha) > 0.0 ? &getMaterialPropertyOlder<RealVectorValue>("moments")
                                         : nullptr),
    _total_rotation_older(std::abs(_alpha) > 0.0
                              ? &getMaterialPropertyOlder<RankTwoTensor>("total_rotation")
                              : nullptr),
    _global_force_res(0),
    _global_moment_res(0),
    _force_local_t(0),
    _moment_local_t(0),
    _local_force_res(0),
    _local_moment_res(0)
{
  if (_ndisp != _nrot)
    mooseError(
        "StressDivergenceBeam: The number of displacement and rotation variables should be same.");

  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);

  for (unsigned int i = 0; i < _nrot; ++i)
    _rot_var[i] = coupled("rotations", i);
}

void
StressDivergenceBeam::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  mooseAssert(_local_re.size() == 2,
              "StressDivergenceBeam: Beam element must have two nodes only.");

  _global_force_res.resize(_test.size());
  _global_moment_res.resize(_test.size());

  computeGlobalResidual(&_force, &_moment, &_total_rotation, _global_force_res, _global_moment_res);

  // add contributions from stiffness proportional damping (non-zero _zeta) or HHT time integration
  // (non-zero _alpha)
  if (_isDamped && _dt > 0.0)
    computeDynamicTerms(_global_force_res, _global_moment_res);

  for (_i = 0; _i < _test.size(); ++_i)
  {
    if (_component < 3)
      _local_re(_i) = _global_force_res[_i](_component);
    else
      _local_re(_i) = _global_moment_res[_i](_component - 3);
  }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (_i = 0; _i < _save_in.size(); ++_i)
      _save_in[_i]->sys().solution().add_vector(_local_re, _save_in[_i]->dofIndices());
  }
}

void
StressDivergenceBeam::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  for (unsigned int i = 0; i < _test.size(); ++i)
  {
    for (unsigned int j = 0; j < _phi.size(); ++j)
    {
      if (_component < 3)
        _local_ke(i, j) = (i == j ? 1 : -1) * _K11[0](_component, _component);
      else
      {
        if (i == j)
          _local_ke(i, j) = _K22[0](_component - 3, _component - 3);
        else
          _local_ke(i, j) = _K22_cross[0](_component - 3, _component - 3);
      }
    }
  }

  // scaling factor for Rayliegh damping and HHT time integration
  if (_isDamped && _dt > 0.0)
    _local_ke *= (1.0 + _alpha + (1.0 + _alpha) * _zeta[0] / _dt);

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; ++i)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

void
StressDivergenceBeam::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    unsigned int coupled_component = 0;
    bool disp_coupled = false;
    bool rot_coupled = false;

    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      if (jvar_num == _disp_var[i])
      {
        coupled_component = i;
        disp_coupled = true;
        break;
      }
    }

    for (unsigned int i = 0; i < _nrot; ++i)
    {
      if (jvar_num == _rot_var[i])
      {
        coupled_component = i + 3;
        rot_coupled = true;
        break;
      }
    }

    prepareMatrixTag(_assembly, _var.number(), jvar_num);

    if (disp_coupled || rot_coupled)
    {
      for (unsigned int i = 0; i < _test.size(); ++i)
      {
        for (unsigned int j = 0; j < _phi.size(); ++j)
        {
          if (_component < 3 && coupled_component < 3)
            _local_ke(i, j) += (i == j ? 1 : -1) * _K11[0](_component, coupled_component);
          else if (_component < 3 && coupled_component > 2)
          {
            if (i == 0)
              _local_ke(i, j) += _K21[0](coupled_component - 3, _component);
            else
              _local_ke(i, j) += _K21_cross[0](coupled_component - 3, _component);
          }
          else if (_component > 2 && coupled_component < 3)
          {
            if (j == 0)
              _local_ke(i, j) += _K21[0](_component - 3, coupled_component);
            else
              _local_ke(i, j) += _K21_cross[0](_component - 3, coupled_component);
          }
          else
          {
            if (i == j)
              _local_ke(i, j) += _K22[0](_component - 3, coupled_component - 3);
            else
              _local_ke(i, j) += _K22_cross[0](_component - 3, coupled_component - 3);
          }
        }
      }
    }

    // scaling factor for Rayleigh damping and HHT time integration
    if (_isDamped && _dt > 0.0)
      _local_ke *= (1.0 + _alpha + (1.0 + _alpha) * _zeta[0] / _dt);

    accumulateTaggedLocalMatrix();
  }
}

void
StressDivergenceBeam::computeDynamicTerms(std::vector<RealVectorValue> & global_force_res,
                                          std::vector<RealVectorValue> & global_moment_res)
{
  mooseAssert(_zeta[0] >= 0.0, "StressDivergenceBeam: Zeta parameter should be non-negative.");
  std::vector<RealVectorValue> global_force_res_old(_test.size());
  std::vector<RealVectorValue> global_moment_res_old(_test.size());
  computeGlobalResidual(
      _force_old, _moment_old, _total_rotation_old, global_force_res_old, global_moment_res_old);

  // For HHT calculation, the global force and moment residual from t_older is required
  std::vector<RealVectorValue> global_force_res_older(_test.size());
  std::vector<RealVectorValue> global_moment_res_older(_test.size());

  if (std::abs(_alpha) > 0.0)
    computeGlobalResidual(_force_older,
                          _moment_older,
                          _total_rotation_older,
                          global_force_res_older,
                          global_moment_res_older);

  // Update the global_force_res and global_moment_res to include HHT and Rayleigh damping
  // contributions
  for (_i = 0; _i < _test.size(); ++_i)
  {
    global_force_res[_i] =
        global_force_res[_i] * (1.0 + _alpha + (1.0 + _alpha) * _zeta[0] / _dt) -
        global_force_res_old[_i] * (_alpha + (1.0 + 2.0 * _alpha) * _zeta[0] / _dt) +
        global_force_res_older[_i] * (_alpha * _zeta[0] / _dt);
    global_moment_res[_i] =
        global_moment_res[_i] * (1.0 + _alpha + (1.0 + _alpha) * _zeta[0] / _dt) -
        global_moment_res_old[_i] * (_alpha + (1.0 + 2.0 * _alpha) * _zeta[0] / _dt) +
        global_moment_res_older[_i] * (_alpha * _zeta[0] / _dt);
  }
}

void
StressDivergenceBeam::computeGlobalResidual(const MaterialProperty<RealVectorValue> * force,
                                            const MaterialProperty<RealVectorValue> * moment,
                                            const MaterialProperty<RankTwoTensor> * total_rotation,
                                            std::vector<RealVectorValue> & global_force_res,
                                            std::vector<RealVectorValue> & global_moment_res)
{
  RealVectorValue a;
  _force_local_t.resize(_qrule->n_points());
  _moment_local_t.resize(_qrule->n_points());
  _local_force_res.resize(_test.size());
  _local_moment_res.resize(_test.size());

  // convert forces/moments from global coordinate system to current beam local configuration
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _force_local_t[_qp] = (*total_rotation)[0] * (*force)[_qp];
    _moment_local_t[_qp] = (*total_rotation)[0] * (*moment)[_qp];
  }

  // residual for displacement variables
  for (_i = 0; _i < _test.size(); ++_i)
  {
    _local_force_res[_i] = a;
    for (unsigned int component = 0; component < 3; ++component)
    {
      for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
        _local_force_res[_i](component) +=
            (_i == 0 ? -1 : 1) * _force_local_t[_qp](component) * 0.5;
    }
  }

  // residual for rotation variables
  for (_i = 0; _i < _test.size(); ++_i)
  {
    _local_moment_res[_i] = a;
    for (unsigned int component = 3; component < 6; ++component)
    {
      for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      {
        if (component == 3)
          _local_moment_res[_i](component - 3) +=
              (_i == 0 ? -1 : 1) * _moment_local_t[_qp](0) * 0.5;
        else if (component == 4)
          _local_moment_res[_i](component - 3) +=
              (_i == 0 ? -1 : 1) * _moment_local_t[_qp](1) * 0.5 +
              _force_local_t[_qp](2) * 0.25 * _original_length[0];
        else
          _local_moment_res[_i](component - 3) +=
              (_i == 0 ? -1 : 1) * _moment_local_t[_qp](2) * 0.5 -
              _force_local_t[_qp](1) * 0.25 * _original_length[0];
      }
    }
  }

  // convert residual for each variable from current beam local configuration to global
  // configuration
  for (_i = 0; _i < _test.size(); ++_i)
  {
    global_force_res[_i] = (*total_rotation)[0].transpose() * _local_force_res[_i];
    global_moment_res[_i] = (*total_rotation)[0].transpose() * _local_moment_res[_i];
  }
}
