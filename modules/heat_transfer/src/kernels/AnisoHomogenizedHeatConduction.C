//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnisoHomogenizedHeatConduction.h"

registerMooseObject("HeatConductionApp", AnisoHomogenizedHeatConduction);

InputParameters
AnisoHomogenizedHeatConduction::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Kernel for asymptotic expansion homogenization for thermal "
                             "conductivity when anisotropic thermal conductivities are used");
  params.addParam<MaterialPropertyName>("diffusion_coefficient",
                                        "thermal_conductivity",
                                        "The diffusion coefficient for the temperature gradient");
  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the direction the variable this "
      "kernel acts in. (0 for x, 1 for y, 2 for z)");
  return params;
}

AnisoHomogenizedHeatConduction::AnisoHomogenizedHeatConduction(const InputParameters & parameters)
  : Kernel(parameters),
    _diffusion_coefficient(getMaterialProperty<RankTwoTensor>("diffusion_coefficient")),
    _component(getParam<unsigned int>("component"))
{
  if (_component >= _mesh.dimension())
    paramError("Value is too large for the mesh dimension: 0, 1, 2 for 1D, 2D, 3D.");
}

Real
AnisoHomogenizedHeatConduction::computeQpResidual()
{
  // This is the matrix-vector product of the diffusion coefficient tensor with
  // the j-th unit vector
  RealVectorValue d_times_ej(0, 0, 0);
  for (unsigned int j = 0; j < _mesh.dimension(); ++j)
    d_times_ej(j) = _diffusion_coefficient[_qp](j, _component);
  return d_times_ej * _grad_test[_i][_qp];
}
