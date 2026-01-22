//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWERDGReconstruction.h"

registerMooseObject("ShallowWaterApp", SWERDGReconstruction);

InputParameters
SWERDGReconstruction::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Face-extrapolated values for SWE variables h, hu, hv (stub: pass-through).");
  params.addRequiredCoupledVar("h", "Conserved variable: h");
  params.addRequiredCoupledVar("hu", "Conserved variable: h*u");
  params.addRequiredCoupledVar("hv", "Conserved variable: h*v");
  params.addParam<UserObjectName>("slope_limiting", "Slope limiting user object (optional)");
  return params;
}

SWERDGReconstruction::SWERDGReconstruction(const InputParameters & parameters)
  : Material(parameters),
    _h(coupledValue("h")),
    _hu(coupledValue("hu")),
    _hv(coupledValue("hv")),
    _hf(declareProperty<Real>("h")),
    _huf(declareProperty<Real>("hu")),
    _hvf(declareProperty<Real>("hv")),
    _lslope(nullptr)
{
  if (isParamValid("slope_limiting"))
    _lslope = &getUserObject<SlopeLimitingBase>("slope_limiting");
}

SWERDGReconstruction::~SWERDGReconstruction() {}

void
SWERDGReconstruction::computeQpProperties()
{
  // Reconstruct linearly to face centers with limited slopes
  if (_bnd && _lslope)
  {
    const unsigned int nvars = 3;
    std::vector<RealGradient> grads = _lslope->getElementSlope(_current_elem->id());
    if (grads.size() >= nvars)
    {
      const RealGradient dvec = _q_point[_qp] - _current_elem->vertex_average();
      _hf[_qp] = _h[_qp] + grads[0] * dvec;
      _huf[_qp] = _hu[_qp] + grads[1] * dvec;
      _hvf[_qp] = _hv[_qp] + grads[2] * dvec;
    }
    else
    {
      _hf[_qp] = _h[_qp];
      _huf[_qp] = _hu[_qp];
      _hvf[_qp] = _hv[_qp];
    }
  }
  else
  {
    // Elemental qp: keep cell averages
    _hf[_qp] = _h[_qp];
    _huf[_qp] = _hu[_qp];
    _hvf[_qp] = _hv[_qp];
  }
}
