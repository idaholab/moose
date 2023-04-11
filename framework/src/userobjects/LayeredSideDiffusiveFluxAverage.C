//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredSideDiffusiveFluxAverage.h"

registerMooseObject("MooseApp", LayeredSideDiffusiveFluxAverage);
registerMooseObjectRenamed("MooseApp",
                           LayeredSideFluxAverage,
                           "06/30/2021 24:00",
                           LayeredSideDiffusiveFluxAverage);

InputParameters
LayeredSideDiffusiveFluxAverage::validParams()
{
  InputParameters params = LayeredSideIntegral::validParams();
  params.addClassDescription("Computes the diffusive flux of a variable on layers alongside "
                             "a boundary.");
  params.addRequiredParam<std::string>(
      "diffusivity",
      "The name of the diffusivity material property that will be used in the flux computation.");
  return params;
}

LayeredSideDiffusiveFluxAverage::LayeredSideDiffusiveFluxAverage(const InputParameters & parameters)
  : LayeredSideAverage(parameters),
    _diffusivity(parameters.get<std::string>("diffusivity")),
    _diffusion_coef(getMaterialProperty<Real>(_diffusivity))
{
}

Real
LayeredSideDiffusiveFluxAverage::computeQpIntegral()
{
  if (_fv)
  {
    // Get the face info
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have a face info");

    // Get the gradient of the variable on the face
    const auto & grad_u = _fv_variable->adGradSln(*fi, determineState());

    // FIXME Get the diffusion coefficient on the face, see #16809
    return -MetaPhysicL::raw_value(_diffusion_coef[_qp] * grad_u * _normals[_qp]);
  }
  else
    return -_diffusion_coef[_qp] * _grad_u[_qp] * _normals[_qp];
}
