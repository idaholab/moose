//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ObtainAvgContactAngle.h"

#include "libmesh/quadrature.h"

registerMooseObject("PhaseFieldApp", ObtainAvgContactAngle);

InputParameters
ObtainAvgContactAngle::validParams()
{
  InputParameters params = SidePostprocessor::validParams();
  params.addClassDescription("Obtain contact angle");
  params.addRequiredCoupledVar("pf", "phase field variable");
  return params;
}

ObtainAvgContactAngle::ObtainAvgContactAngle(const InputParameters & parameters)
  : SidePostprocessor(parameters), _pf(coupledValue("pf")), _grad_pf(coupledGradient("pf"))
{
}

void
ObtainAvgContactAngle::initialize()
{
  _cos_theta_val = 0.0;
  _total_weight = 0.0;
}

void
ObtainAvgContactAngle::execute()
{

  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    if (std::abs(_pf[qp]) < 1.0) //Operating only within the interface
    {
      Real tol_val = libMesh::TOLERANCE * libMesh::TOLERANCE;
      const Real weight = _grad_pf[qp].norm();
      _cos_theta_val +=
          _grad_pf[qp] * _normals[qp]; // weight * (_grad_pf[qp]/_grad_pf[qp].norm()) * _normals[qp]
      _total_weight += weight;
    }
  }
}

Real
ObtainAvgContactAngle::getValue() const
{
  return _contact_angle;
}

void
ObtainAvgContactAngle::threadJoin(const UserObject & y)
{
  const ObtainAvgContactAngle & pps = static_cast<const ObtainAvgContactAngle &>(y);
  _cos_theta_val += pps._cos_theta_val;
  _total_weight += pps._total_weight;
}

void
ObtainAvgContactAngle::finalize()
{
  gatherSum(_cos_theta_val);
  gatherSum(_total_weight);
  _contact_angle = std::acos(_cos_theta_val / _total_weight) * 180 / libMesh::pi;
}
