//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ObtainContactAngle.h"

#include "libmesh/quadrature.h"

registerMooseObject("PhaseFieldApp", ObtainContactAngle);

InputParameters
ObtainContactAngle::validParams()
{
  InputParameters params = SidePostprocessor::validParams();
  params.addClassDescription("Obtain contact angle");
  params.addRequiredCoupledVar("pf", "phase field variable");
  return params;
}

ObtainContactAngle::ObtainContactAngle(const InputParameters & parameters)
  : SidePostprocessor(parameters), _grad_pf(coupledGradient("pf"))
{
}

void
ObtainContactAngle::initialize()
{
  _cos_theta_val = 0.0;
  _total_weight = 0.0;
}

void
ObtainContactAngle::execute()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    const Real weight = _grad_pf[qp].norm();
    _cos_theta_val +=
        _grad_pf[qp] * _normals[qp]; // weight * (_grad_pf[qp]/_grad_pf[qp].norm()) * _normals[qp]
    _total_weight += weight;
  }
}

Real
ObtainContactAngle::getValue() const
{
  return _contact_angle;
}

void
ObtainContactAngle::threadJoin(const UserObject & y)
{
  const ObtainContactAngle & pps = static_cast<const ObtainContactAngle &>(y);
  _cos_theta_val += pps._cos_theta_val;
  _total_weight += pps._total_weight;
}

void
ObtainContactAngle::finalize()
{
  gatherSum(_cos_theta_val);
  gatherSum(_total_weight);
  _contact_angle = std::acos(_cos_theta_val / _total_weight) * 180 /libMesh::pi;
}
