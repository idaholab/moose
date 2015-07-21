/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ComputeGrainForceAndTorque.h"

template<>
InputParameters validParams<ComputeGrainForceAndTorque>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription("Userobject for calculating force and torque acting on a grain");
  params.addRequiredParam<MaterialPropertyName>("force_density", "Force density material");
  params.addParam<UserObjectName>("grain_data","center of mass of grains");
  return params;
}

ComputeGrainForceAndTorque::ComputeGrainForceAndTorque(const std::string & name, InputParameters parameters) :
    ElementUserObject(name, parameters),
    _dF(getMaterialProperty<std::vector<RealGradient> >("force_density")),
    _dFdc(getMaterialProperty<std::vector<RealGradient> >("dFdc")),
    _grain_data(getUserObject<ComputeGrainCenterUserObject>("grain_data")),
    _grain_volumes(_grain_data.getGrainVolumes()),
    _grain_centers(_grain_data.getGrainCenters()),
    _ncrys(_grain_volumes.size()),
    _ncomp(6*_ncrys),
    _force_values(_ncrys),
    _torque_values(_ncrys),
    _force_derivatives(_ncrys),
    _torque_derivatives(_ncrys),
    _force_torque_store(_ncomp),
    _force_torque_derivative_store(_ncomp)
{
}

void
ComputeGrainForceAndTorque::initialize()
{
  for (unsigned int i = 0; i < _ncomp; ++i)
  {
    _force_torque_store[i] = 0;
    _force_torque_derivative_store[i] = 0;
  }
}

void
ComputeGrainForceAndTorque::execute()
{
  RealGradient _compute_torque = 0.0;
  RealGradient _compute_torque_derivative = 0.0;

  for (unsigned int i = 0; i < _ncrys; ++i)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
    {
      _compute_torque =_JxW[_qp] * _coord[_qp] * (_q_point[_qp] - _grain_centers[i]).cross(_dF[_qp][i]);
      _force_torque_store[6*i+0] += _JxW[_qp] * _coord[_qp] * _dF[_qp][i](0);
      _force_torque_store[6*i+1] += _JxW[_qp] * _coord[_qp] * _dF[_qp][i](1);
      _force_torque_store[6*i+2] += _JxW[_qp] * _coord[_qp] * _dF[_qp][i](2);
      _force_torque_store[6*i+3] += _compute_torque(0);
      _force_torque_store[6*i+4] += _compute_torque(1);
      _force_torque_store[6*i+5] += _compute_torque(2);

      _compute_torque_derivative =_JxW[_qp] * _coord[_qp] * (_q_point[_qp] - _grain_centers[i]).cross(_dFdc[_qp][i]);
      _force_torque_derivative_store[6*i+0] += _JxW[_qp] * _coord[_qp] * _dFdc[_qp][i](0);
      _force_torque_derivative_store[6*i+1] += _JxW[_qp] * _coord[_qp] * _dFdc[_qp][i](1);
      _force_torque_derivative_store[6*i+2] += _JxW[_qp] * _coord[_qp] * _dFdc[_qp][i](2);
      _force_torque_derivative_store[6*i+3] += _compute_torque_derivative(0);
      _force_torque_derivative_store[6*i+4] += _compute_torque_derivative(1);
      _force_torque_derivative_store[6*i+5] += _compute_torque_derivative(2);
    }
}

void
ComputeGrainForceAndTorque::finalize()
{
  gatherSum(_force_torque_store);
  gatherSum(_force_torque_derivative_store);

  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _force_values[i](0) = _force_torque_store[6*i+0];
    _force_values[i](1) = _force_torque_store[6*i+1];
    _force_values[i](2) = _force_torque_store[6*i+2];
    _torque_values[i](0) = _force_torque_store[6*i+3];
    _torque_values[i](1) = _force_torque_store[6*i+4];
    _torque_values[i](2) = _force_torque_store[6*i+5];

    _force_derivatives[i](0) = _force_torque_derivative_store[6*i+0];
    _force_derivatives[i](1) = _force_torque_derivative_store[6*i+1];
    _force_derivatives[i](2) = _force_torque_derivative_store[6*i+2];
    _torque_derivatives[i](0) = _force_torque_derivative_store[6*i+3];
    _torque_derivatives[i](1) = _force_torque_derivative_store[6*i+4];
    _torque_derivatives[i](2) = _force_torque_derivative_store[6*i+5];
  }
}

void
ComputeGrainForceAndTorque::threadJoin(const UserObject & y)
{
  const ComputeGrainForceAndTorque & pps = static_cast<const ComputeGrainForceAndTorque &>(y);
  for (unsigned int i = 0; i < _ncomp; ++i)
  {
    _force_torque_store[i] += pps._force_torque_store[i];
    _force_torque_derivative_store[i] += pps._force_torque_derivative_store[i];
  }
}

const std::vector<RealGradient> &
ComputeGrainForceAndTorque::getForceValues() const
{
  return _force_values;
}

const std::vector<RealGradient> &
ComputeGrainForceAndTorque::getTorqueValues() const
{
  return _torque_values;
}

const std::vector<RealGradient> &
ComputeGrainForceAndTorque::getForceDerivatives() const
{
  return _force_derivatives;
}

const std::vector<RealGradient> &
ComputeGrainForceAndTorque::getTorqueDerivatives() const
{
  return _torque_derivatives;
}
