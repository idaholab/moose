//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeExternalGrainForceAndTorque.h"
#include "GrainTrackerInterface.h"

#include "libmesh/quadrature.h"

registerMooseObject("PhaseFieldApp", ComputeExternalGrainForceAndTorque);

InputParameters
ComputeExternalGrainForceAndTorque::validParams()
{
  InputParameters params = ShapeElementUserObject::validParams();
  params.addClassDescription("Userobject for calculating force and torque acting on a grain");
  params.addParam<MaterialPropertyName>("force_density", "force_density", "Force density material");
  params.addParam<UserObjectName>("grain_data", "center of mass of grains");
  params.addCoupledVar("c", "Concentration field");
  params.addCoupledVar("etas", "Array of coupled order parameters");
  return params;
}

ComputeExternalGrainForceAndTorque::ComputeExternalGrainForceAndTorque(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ShapeElementUserObject>(parameters),
    GrainForceAndTorqueInterface(),
    _c_name(coupledName("c", 0)),
    _c_var(coupled("c")),
    _dF_name(getParam<MaterialPropertyName>("force_density")),
    _dF(getMaterialPropertyByName<std::vector<RealGradient>>(_dF_name)),
    _dFdc(getMaterialPropertyByName<std::vector<RealGradient>>(
        derivativePropertyNameFirst(_dF_name, _c_name))),
    _op_num(coupledComponents("etas")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_data")),
    _vals_var(_op_num),
    _vals_name(_op_num),
    _dFdeta(_op_num)
{
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _vals_var[i] = coupled("etas", i);
    _vals_name[i] = coupledName("etas", i);
    _dFdeta[i] = &getMaterialPropertyByName<std::vector<RealGradient>>(
        derivativePropertyNameFirst(_dF_name, _vals_name[i]));
  }
}

void
ComputeExternalGrainForceAndTorque::initialize()
{
  _grain_num = _grain_tracker.getTotalFeatureCount();
  _ncomp = 6 * _grain_num;

  _force_values.resize(_grain_num);
  _torque_values.resize(_grain_num);
  _force_torque_store.assign(_ncomp, 0.0);

  if (_fe_problem.currentlyComputingJacobian())
  {
    _total_dofs = _subproblem.es().n_dofs();
    _force_torque_c_jacobian_store.assign(_ncomp * _total_dofs, 0.0);
    _force_torque_eta_jacobian_store.resize(_op_num);

    for (unsigned int i = 0; i < _op_num; ++i)
      _force_torque_eta_jacobian_store[i].assign(_ncomp * _total_dofs, 0.0);
  }
}

void
ComputeExternalGrainForceAndTorque::execute()
{
  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  for (unsigned int i = 0; i < _grain_num; ++i)
    for (unsigned int j = 0; j < _op_num; ++j)
      if (i == op_to_grains[j])
      {
        const auto centroid = _grain_tracker.getGrainCentroid(i);
        for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
          if (_dF[_qp][j](0) != 0.0 || _dF[_qp][j](1) != 0.0 || _dF[_qp][j](2) != 0.0)
          {
            const RealGradient compute_torque =
                _JxW[_qp] * _coord[_qp] *
                (_current_elem->vertex_average() - centroid).cross(_dF[_qp][j]);
            _force_torque_store[6 * i + 0] += _JxW[_qp] * _coord[_qp] * _dF[_qp][j](0);
            _force_torque_store[6 * i + 1] += _JxW[_qp] * _coord[_qp] * _dF[_qp][j](1);
            _force_torque_store[6 * i + 2] += _JxW[_qp] * _coord[_qp] * _dF[_qp][j](2);
            _force_torque_store[6 * i + 3] += compute_torque(0);
            _force_torque_store[6 * i + 4] += compute_torque(1);
            _force_torque_store[6 * i + 5] += compute_torque(2);
          }
      }
}

void
ComputeExternalGrainForceAndTorque::executeJacobian(unsigned int jvar)
{
  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  if (jvar == _c_var)
    for (unsigned int i = 0; i < _grain_num; ++i)
      for (unsigned int j = 0; j < _op_num; ++j)
        if (i == op_to_grains[j])
        {
          const auto centroid = _grain_tracker.getGrainCentroid(i);
          for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
            if (_dFdc[_qp][j](0) != 0.0 || _dFdc[_qp][j](1) != 0.0 || _dFdc[_qp][j](2) != 0.0)
            {
              const Real factor = _JxW[_qp] * _coord[_qp] * _phi[_j][_qp];
              const RealGradient compute_torque_jacobian_c =
                  factor * (_current_elem->vertex_average() - centroid).cross(_dFdc[_qp][j]);
              _force_torque_c_jacobian_store[(6 * i + 0) * _total_dofs + _j_global] +=
                  factor * _dFdc[_qp][j](0);
              _force_torque_c_jacobian_store[(6 * i + 1) * _total_dofs + _j_global] +=
                  factor * _dFdc[_qp][j](1);
              _force_torque_c_jacobian_store[(6 * i + 2) * _total_dofs + _j_global] +=
                  factor * _dFdc[_qp][j](2);
              _force_torque_c_jacobian_store[(6 * i + 3) * _total_dofs + _j_global] +=
                  compute_torque_jacobian_c(0);
              _force_torque_c_jacobian_store[(6 * i + 4) * _total_dofs + _j_global] +=
                  compute_torque_jacobian_c(1);
              _force_torque_c_jacobian_store[(6 * i + 5) * _total_dofs + _j_global] +=
                  compute_torque_jacobian_c(2);
            }
        }

  for (unsigned int i = 0; i < _op_num; ++i)
    if (jvar == _vals_var[i])
      for (unsigned int j = 0; j < _grain_num; ++j)
        for (unsigned int k = 0; k < _op_num; ++k)
          if (j == op_to_grains[k])
          {
            const auto centroid = _grain_tracker.getGrainCentroid(j);
            for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
              if ((*_dFdeta[i])[_qp][j](0) != 0.0 || (*_dFdeta[i])[_qp][j](1) != 0.0 ||
                  (*_dFdeta[i])[_qp][j](2) != 0.0)
              {
                const Real factor = _JxW[_qp] * _coord[_qp] * _phi[_j][_qp];
                const RealGradient compute_torque_jacobian_eta =
                    factor *
                    (_current_elem->vertex_average() - centroid).cross((*_dFdeta[i])[_qp][k]);
                _force_torque_eta_jacobian_store[i][(6 * j + 0) * _total_dofs + _j_global] +=
                    factor * (*_dFdeta[i])[_qp][k](0);
                _force_torque_eta_jacobian_store[i][(6 * j + 1) * _total_dofs + _j_global] +=
                    factor * (*_dFdeta[i])[_qp][k](1);
                _force_torque_eta_jacobian_store[i][(6 * j + 2) * _total_dofs + _j_global] +=
                    factor * (*_dFdeta[i])[_qp][k](2);
                _force_torque_eta_jacobian_store[i][(6 * j + 3) * _total_dofs + _j_global] +=
                    compute_torque_jacobian_eta(0);
                _force_torque_eta_jacobian_store[i][(6 * j + 4) * _total_dofs + _j_global] +=
                    compute_torque_jacobian_eta(1);
                _force_torque_eta_jacobian_store[i][(6 * j + 5) * _total_dofs + _j_global] +=
                    compute_torque_jacobian_eta(2);
              }
          }
}

void
ComputeExternalGrainForceAndTorque::finalize()
{
  gatherSum(_force_torque_store);
  for (unsigned int i = 0; i < _grain_num; ++i)
  {
    _force_values[i](0) = _force_torque_store[6 * i + 0];
    _force_values[i](1) = _force_torque_store[6 * i + 1];
    _force_values[i](2) = _force_torque_store[6 * i + 2];
    _torque_values[i](0) = _force_torque_store[6 * i + 3];
    _torque_values[i](1) = _force_torque_store[6 * i + 4];
    _torque_values[i](2) = _force_torque_store[6 * i + 5];
  }

  if (_fe_problem.currentlyComputingJacobian())
  {
    gatherSum(_force_torque_c_jacobian_store);
    for (unsigned int i = 0; i < _op_num; ++i)
      gatherSum(_force_torque_eta_jacobian_store[i]);
  }
}

void
ComputeExternalGrainForceAndTorque::threadJoin(const UserObject & y)
{
  const ComputeExternalGrainForceAndTorque & pps =
      static_cast<const ComputeExternalGrainForceAndTorque &>(y);
  for (unsigned int i = 0; i < _ncomp; ++i)
    _force_torque_store[i] += pps._force_torque_store[i];
  if (_fe_problem.currentlyComputingJacobian())
  {
    for (unsigned int i = 0; i < _ncomp * _total_dofs; ++i)
      _force_torque_c_jacobian_store[i] += pps._force_torque_c_jacobian_store[i];
    for (unsigned int i = 0; i < _op_num; ++i)
      for (unsigned int j = 0; j < _ncomp * _total_dofs; ++j)
        _force_torque_eta_jacobian_store[i][j] += pps._force_torque_eta_jacobian_store[i][j];
  }
}

const std::vector<RealGradient> &
ComputeExternalGrainForceAndTorque::getForceValues() const
{
  return _force_values;
}

const std::vector<RealGradient> &
ComputeExternalGrainForceAndTorque::getTorqueValues() const
{
  return _torque_values;
}

const std::vector<Real> &
ComputeExternalGrainForceAndTorque::getForceCJacobians() const
{
  return _force_torque_c_jacobian_store;
}
const std::vector<std::vector<Real>> &
ComputeExternalGrainForceAndTorque::getForceEtaJacobians() const
{
  return _force_torque_eta_jacobian_store;
}
