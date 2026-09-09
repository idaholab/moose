//* This file is part of the MOOSE framework
//* https://www.mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ObtainAvgContactAngle.h"

#include "libmesh/quadrature.h"

#include <algorithm>

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
  : SidePostprocessor(parameters),
    _pf(coupledValue("pf")),
    _grad_pf(coupledGradient("pf")),
    _contact_angle(0.0),
    _cos_theta_val(0.0),
    _total_weight(0.0)
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
  // The pointwise angle cos(theta) = grad(pf).n/|grad(pf)| is only defined where the interface
  // meets the boundary; in the bulk phases grad(pf) vanishes and the interface normal with it.
  // Weighting by |grad(pf)| removes the pointwise division, and the double well factor (1 - pf^2)
  // -- the same interface localization the contact angle boundary condition uses -- suppresses the
  // bulk, whose share of the boundary otherwise biases the average toward 90 degrees. Both factors
  // are smooth in pf, so the reported angle varies smoothly with the solution.
  for (const auto qp : make_range(_qrule->n_points()))
  {
    // pf can overshoot |pf| = 1 slightly; holding the weight at zero there keeps it non-negative,
    // which is what bounds the averaged cosine below by -1 and above by 1.
    const Real localization = std::max(0.0, 1.0 - _pf[qp] * _pf[qp]);
    const Real w = _JxW[qp] * _coord[qp] * localization;
    _cos_theta_val += w * (_grad_pf[qp] * _normals[qp]);
    _total_weight += w * _grad_pf[qp].norm();
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
  const ObtainAvgContactAngle & pps = cast_ref<const ObtainAvgContactAngle &>(y);
  _cos_theta_val += pps._cos_theta_val;
  _total_weight += pps._total_weight;
}

void
ObtainAvgContactAngle::finalize()
{
  gatherSum(_cos_theta_val);
  gatherSum(_total_weight);

  // With the interface fully detached from the boundary there is no angle to report, so hold the
  // last value instead of dividing by zero.
  if (_total_weight == 0.0)
    return;

  // |grad(pf).n| <= |grad(pf)| holds pointwise, so the ratio of the integrals lies in [-1, 1] up
  // to roundoff; clamp it so that a ratio a few epsilon outside the range cannot produce a NaN.
  const Real cos_theta = std::clamp(_cos_theta_val / _total_weight, -1.0, 1.0);
  _contact_angle = std::acos(cos_theta) * 180 / libMesh::pi;
}
