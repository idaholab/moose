/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AEFVMaterial.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<AEFVMaterial>()
{
  InputParameters params = validParams<Material>();
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
    RealGradient dvec = _q_point[_qp] - _current_elem->centroid();

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
