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
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Userobject for calculating force and torque acting on a grain");
  params.addParam<std::vector<Real> >("force", "force acting on grains");
  params.addParam<std::vector<Real> >("torque", "torque acting on grains");

  return params;
}

ConstantGrainForceAndTorque::ConstantGrainForceAndTorque(const InputParameters & parameters) :
    GeneralUserObject(parameters),
    _F(getParam<std::vector<Real> >("force")),
    _M(getParam<std::vector<Real> >("torque")),
    _ncrys(_F.size()/3),
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
    for (unsigned int i = 0; i < _ncrys; ++i)
    {
      _force_torque_store[6*i+0] = _F[3*i+0];
      _force_torque_store[6*i+1] = _F[3*i+1];
      _force_torque_store[6*i+2] = _F[3*i+2];
      _force_torque_store[6*i+3] = _M[3*i+0];
      _force_torque_store[6*i+4] = _M[3*i+1];
      _force_torque_store[6*i+5] = _M[3*i+2];

      _force_torque_derivative_store[6*i+0] = 0.0;
      _force_torque_derivative_store[6*i+1] = 0.0;
      _force_torque_derivative_store[6*i+2] = 0.0;
      _force_torque_derivative_store[6*i+3] = 0.0;
      _force_torque_derivative_store[6*i+4] = 0.0;
      _force_torque_derivative_store[6*i+5] = 0.0;
    }
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
