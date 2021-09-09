//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AEFVMaterial.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

registerMooseObject("RdgApp", AEFVMaterial);

InputParameters
AEFVMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "A material kernel for the advection equation using a cell-centered finite volume method.");
  params.addRequiredCoupledVar("u", "Cell-averge variable");
  params.addRequiredParam<UserObjectName>("slope_limiting", "Name for slope limiting user object");
  return params;
}

AEFVMaterial::AEFVMaterial(const InputParameters & parameters)
  : Material(parameters),
    _uc(coupledValue("u")),
    _lslope(getUserObject<SlopeLimitingBase>("slope_limiting")),
    _u(declareProperty<Real>("u"))
{
}

AEFVMaterial::~AEFVMaterial() {}

void
AEFVMaterial::computeQpProperties()
{
  // initialize the variable
  _u[_qp] = _uc[_qp];

  // interpolate variable values at face center
  if (_bnd)
  {
    // you should know how many equations you are solving and assign this number
    // e.g. = 1 (for the advection equation)
    unsigned int nvars = 1;
    std::vector<RealGradient> ugrad(nvars, RealGradient(0., 0., 0.));
    ugrad = _lslope.getElementSlope(_current_elem->id());

    // get the directional vector from cell center to face center
    RealGradient dvec = _q_point[_qp] - _current_elem->vertex_average();

    // calculate the variable at face center
    _u[_qp] += ugrad[0] * dvec;

    // clear the temporary vectors
    ugrad.clear();
  }
  // calculations only for elemental output
  else if (!_bnd)
  {
  }
}
