//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressDivergenceTruss.h"

// MOOSE includes
#include "Assembly.h"
#include "Material.h"
#include "MooseVariable.h"
#include "SystemBase.h"
#include "RankTwoTensor.h"
#include "NonlinearSystem.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", StressDivergenceTruss);

InputParameters
StressDivergenceTruss::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Quasi-static and dynamic stress divergence kernel for truss element");
  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the direction the variable this kernel acts in. (0 for disp_x, "
      "1 for disp_y, 2 for disp_z)");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
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

StressDivergenceTruss::StressDivergenceTruss(const InputParameters & parameters)
  : Kernel(parameters),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _K11(getMaterialPropertyByName<Real>("Jacobian_11")),
    _original_length(getMaterialPropertyByName<Real>("original_length")),
    _zeta(getMaterialProperty<Real>("zeta")),
    _alpha(getParam<Real>("alpha")),
    _isDamped(getParam<MaterialPropertyName>("zeta") != "0.0" || std::abs(_alpha) > 0.0),
    // _force(getMaterialPropertyByName<RealVectorValue>("forces")),
    // _force_old(_isDamped ? &getMaterialPropertyOld<RealVectorValue>("forces") : nullptr),
    // _force_older(std::abs(_alpha) > 0.0 ? &getMaterialPropertyOlder<RealVectorValue>("forces")
    //                                     : nullptr),
    _force(getMaterialPropertyByName<Real>("forces")),
    _force_old(_isDamped ? &getMaterialPropertyOld<Real>("forces") : nullptr),
    _force_older(std::abs(_alpha) > 0.0 ? &getMaterialPropertyOlder<Real>("forces")
                                        : nullptr),
    _total_rotation(getMaterialPropertyByName<RankTwoTensor>("total_rotation")),
    _total_rotation_old(_isDamped ? &getMaterialPropertyOld<RankTwoTensor>("total_rotation")
                                  : nullptr),
    _orientation(NULL),

    _global_force_res(0),
    _force_local_t(0),
    _local_force_res(0)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);
}

void
StressDivergenceTruss::initialSetup()
{
  _orientation = &_subproblem.assembly(_tid).getFE(FEType(), 1)->get_dxyzdxi();
}

void
StressDivergenceTruss::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  mooseAssert(_local_re.size() == 2, "Truss element must have two nodes only.");

  RealGradient orientation((*_orientation)[0]);
  orientation /= orientation.norm();

  VectorValue<Real> force_local = _force[_qp] * orientation;

  _local_re(0) = -force_local(_component);
  _local_re(1) = -_local_re(0);

  // _global_force_res.resize(_test.size());
  //
  // computeGlobalResidual(&_force, &_total_rotation, _global_force_res);
  //
  // // add contributions from stiffness proportional damping (non-zero _zeta) or HHT time integration
  // // (non-zero _alpha)
  // // if (_isDamped && _dt > 0.0)
  // //   computeDynamicTerms(_global_force_res);
  //
  // for (_i = 0; _i < _test.size(); ++_i)
  //   if (_component < 3)
  //     _local_re(_i) = _global_force_res[_i](_component);

  out <<  " force " << _force[_qp] << " _local_re(0) "<<_local_re(0)<< " _local_re(1) " << _local_re(1) << std::endl;

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); ++i)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

Real
StressDivergenceTruss::computeStiffness(unsigned int i, unsigned int j)
{
  RealGradient orientation((*_orientation)[0]);
  orientation /= orientation.norm();
  return orientation(i) * orientation(j) * _K11[_qp];
}

void
StressDivergenceTruss::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  for (unsigned int i = 0; i < _test.size(); ++i)
    for (unsigned int j = 0; j < _phi.size(); ++j)
      if (_component < 3)
        _local_ke(i, j) = (i == j ? 1 : -1) * computeStiffness(_component, _component);
        // _local_ke(i, j) = (i == j ? 1 : -1) * _K11[0](_component, _component);

  // scaling factor for Rayliegh damping and HHT time integration
  // if (_isDamped && _dt > 0.0)
  //   _local_ke *= (1.0 + _alpha + (1.0 + _alpha) * _zeta[0] / _dt);

  // out <<" _component " << _component << " _local_ke " << _local_ke << std::endl;

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
StressDivergenceTruss::computeOffDiagJacobian(MooseVariableFEBase & jvar)
{
  size_t jvar_num = jvar.number();
  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    // This (undisplaced) jvar could potentially yield the wrong phi size if this object is acting
    // on the displaced mesh
    auto phi_size = _sys.getVariable(_tid, jvar.number()).dofIndices().size();

    unsigned int coupled_component = 0;
    bool disp_coupled = false;

    for (unsigned int i = 0; i < _ndisp; ++i)
      if (jvar_num == _disp_var[i])
      {
        coupled_component = i;
        disp_coupled = true;
        break;
      }

    if (disp_coupled)
    {
      prepareMatrixTag(_assembly, _var.number(), jvar_num);

      for (unsigned int i = 0; i < _test.size(); ++i)
        for (unsigned int j = 0; j < phi_size; ++j)
          _local_ke(i, j) += (i == j ? 1 : -1) * computeStiffness(_component, coupled_component);

      accumulateTaggedLocalMatrix();
    }
    else if (false) // Need some code here for coupling with temperature
    {
    }
  }
}

// void
// StressDivergenceTruss::computeDynamicTerms(std::vector<RealVectorValue> & global_force_res)
// {
//   mooseAssert(_zeta[0] >= 0.0, "StressDivergenceBeam: Zeta parameter should be non-negative.");
//   std::vector<RealVectorValue> global_force_res_old(_test.size());
//   computeGlobalResidual(_force_old, &_total_rotation, global_force_res_old);
//
//   // For HHT calculation, the global force and moment residual from t_older is required
//   std::vector<RealVectorValue> global_force_res_older(_test.size());
//
//   if (std::abs(_alpha) > 0.0)
//     computeGlobalResidual(_force_older, &_total_rotation, global_force_res_older);
//
//   // Update the global_force_res and global_moment_res to include HHT and Rayleigh damping
//   // contributions
//   for (_i = 0; _i < _test.size(); ++_i)
//   {
//     global_force_res[_i] =
//         global_force_res[_i] * (1.0 + _alpha + (1.0 + _alpha) * _zeta[0] / _dt) -
//         global_force_res_old[_i] * (_alpha + (1.0 + 2.0 * _alpha) * _zeta[0] / _dt) +
//         global_force_res_older[_i] * (_alpha * _zeta[0] / _dt);
//   }
// }
//
// void
// StressDivergenceTruss::computeGlobalResidual(const MaterialProperty<RealVectorValue> * force,
//                                              const MaterialProperty<RankTwoTensor> * total_rotation,
//                                              std::vector<RealVectorValue> & global_force_res)
// {
//   // RealGradient orientation((*_orientation)[0]);
//   // orientation /= orientation.norm();
//
//   RealVectorValue a;
//   _force_local_t.resize(_qrule->n_points());
//   _local_force_res.resize(_test.size());
//
//   // convert forces from global coordinate system to current truss local configuration
//   for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
//     _force_local_t[_qp] = (*total_rotation)[0] * (*force)[_qp];
//
//   // residual for displacement variables
//   for (_i = 0; _i < _test.size(); ++_i)
//   {
//     _local_force_res[_i] = a;
//     for (unsigned int component = 0; component < 3; ++component)
//     {
//       for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
//         _local_force_res[_i](component) +=
//             (_i == 0 ? -1 : 1) * _force_local_t[_qp](component) * 0.5;
//     }
//   }
//
//   // convert residual for each variable from current beam local configuration to global
//   // configuration
//   for (_i = 0; _i < _test.size(); ++_i)
//     global_force_res[_i] = (*total_rotation)[0].transpose() * _local_force_res[_i];
//     // global_force_res[_i] = orientation * _local_force_res[_i];
//
//     // out<< " _force_local_t " << _force_local_t[_qp] << " _local_force_res" << _local_force_res[0] << " "<<_local_force_res[1] << " global_force_res " << global_force_res[0] << " " << global_force_res[1] << std::endl;
// }
