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

#include "ConstantGrainForceAndTorque.h"

template<>
InputParameters validParams<ConstantGrainForceAndTorque>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription("Userobject for calculating force and torque acting on a grain");
  params.addParam<RealGradient>("force", "force acting on grains");
  params.addParam<UserObjectName>("grain_data","center of mass of grains");

  return params;
}

ConstantGrainForceAndTorque::ConstantGrainForceAndTorque(const InputParameters & parameters) :
    ElementUserObject(parameters),
    _dF(getParam<RealGradient>("force")),
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
ConstantGrainForceAndTorque::initialize()
{
  for (unsigned int i = 0; i < _ncomp; ++i)
  {
    _force_torque_store[i] = 0;
    _force_torque_derivative_store[i] = 0;
  }
}

void
ConstantGrainForceAndTorque::execute()
{
    RealGradient compute_torque0 = 0.0;
    RealGradient compute_torque1 = 0.0;

    for (_qp=0; _qp<_qrule->n_points(); ++_qp)
    {
      compute_torque0 += (_q_point[_qp] - _grain_centers[0]).cross(_dF);
      compute_torque1 += (_q_point[_qp] - _grain_centers[1]).cross(- _dF);

      _force_torque_store[0] = _dF(0);
      _force_torque_store[1] = _dF(1);
      _force_torque_store[2] = _dF(2);
      _force_torque_store[3] = compute_torque0(0);
      _force_torque_store[4] = compute_torque0(1);
      _force_torque_store[5] = compute_torque0(2);
      _force_torque_store[6] = - _dF(0);
      _force_torque_store[7] = - _dF(1);
      _force_torque_store[8] = - _dF(2);
      _force_torque_store[9] = compute_torque1(0);
      _force_torque_store[10] = compute_torque1(1);
      _force_torque_store[11] = compute_torque1(2);
    }

      _force_torque_derivative_store[0] = 0.0;
      _force_torque_derivative_store[1] = 0.0;
      _force_torque_derivative_store[2] = 0.0;
      _force_torque_derivative_store[3] = 0.0;
      _force_torque_derivative_store[4] = 0.0;
      _force_torque_derivative_store[5] = 0.0;
      _force_torque_derivative_store[6] = 0.0;
      _force_torque_derivative_store[7] = 0.0;
      _force_torque_derivative_store[8] = 0.0;
      _force_torque_derivative_store[9] = 0.0;
      _force_torque_derivative_store[10] = 0.0;
      _force_torque_derivative_store[11] = 0.0;
}

void
ConstantGrainForceAndTorque::finalize()
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
ConstantGrainForceAndTorque::threadJoin(const UserObject & y)
{
  const ConstantGrainForceAndTorque & pps = static_cast<const ConstantGrainForceAndTorque &>(y);
  for (unsigned int i = 0; i < _ncomp; ++i)
  {
    _force_torque_store[i] += pps._force_torque_store[i];
    _force_torque_derivative_store[i] += pps._force_torque_derivative_store[i];
  }
}

const std::vector<RealGradient> &
ConstantGrainForceAndTorque::getForceValues() const
{
  return _force_values;
}

const std::vector<RealGradient> &
ConstantGrainForceAndTorque::getTorqueValues() const
{
  return _torque_values;
}

const std::vector<RealGradient> &
ConstantGrainForceAndTorque::getForceDerivatives() const
{
  return _force_derivatives;
}

const std::vector<RealGradient> &
ConstantGrainForceAndTorque::getTorqueDerivatives() const
{
  return _torque_derivatives;
}
