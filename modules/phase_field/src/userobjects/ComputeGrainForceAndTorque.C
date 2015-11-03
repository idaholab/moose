/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeGrainForceAndTorque.h"
#include "ComputeGrainCenterUserObject.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<ComputeGrainForceAndTorque>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription("Userobject for calculating force and torque acting on a grain");
  params.addParam<MaterialPropertyName>("force_density", "force_density", "Force density material");
  params.addParam<UserObjectName>("grain_data", "center of mass of grains");
  params.addCoupledVar("c", "Concentration field");
  return params;
}

ComputeGrainForceAndTorque::ComputeGrainForceAndTorque(const InputParameters & parameters) :
    ElementUserObject(parameters),
    GrainForceAndTorqueInterface(),
    _c_name(getVar("c", 0)->name()),
    _dF(getMaterialProperty<std::vector<RealGradient> >("force_density")),
    _dF_name(getParam<MaterialPropertyName>("force_density")),
    _dFdc(getMaterialPropertyByName<std::vector<RealGradient> >(propertyNameFirst(_dF_name, _c_name))),
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
  for (unsigned int i = 0; i < _ncrys; ++i)
    for (_qp=0; _qp<_qrule->n_points(); ++_qp)
    {
      const RealGradient compute_torque =_JxW[_qp] * _coord[_qp] * (_q_point[_qp] - _grain_centers[i]).cross(_dF[_qp][i]);
      _force_torque_store[6*i+0] += _JxW[_qp] * _coord[_qp] * _dF[_qp][i](0);
      _force_torque_store[6*i+1] += _JxW[_qp] * _coord[_qp] * _dF[_qp][i](1);
      _force_torque_store[6*i+2] += _JxW[_qp] * _coord[_qp] * _dF[_qp][i](2);
      _force_torque_store[6*i+3] += compute_torque(0);
      _force_torque_store[6*i+4] += compute_torque(1);
      _force_torque_store[6*i+5] += compute_torque(2);

      const RealGradient compute_torque_derivative_c =_JxW[_qp] * _coord[_qp] * (_q_point[_qp] - _grain_centers[i]).cross(_dFdc[_qp][i]);
      _force_torque_derivative_store[6*i+0] += _JxW[_qp] * _coord[_qp] * _dFdc[_qp][i](0);
      _force_torque_derivative_store[6*i+1] += _JxW[_qp] * _coord[_qp] * _dFdc[_qp][i](1);
      _force_torque_derivative_store[6*i+2] += _JxW[_qp] * _coord[_qp] * _dFdc[_qp][i](2);
      _force_torque_derivative_store[6*i+3] += compute_torque_derivative_c(0);
      _force_torque_derivative_store[6*i+4] += compute_torque_derivative_c(1);
      _force_torque_derivative_store[6*i+5] += compute_torque_derivative_c(2);
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
