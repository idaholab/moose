//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosAnisoHomogenizedHeatConduction.h"

registerKokkosResidualObject("HeatTransferApp", KokkosAnisoHomogenizedHeatConduction);

InputParameters
KokkosAnisoHomogenizedHeatConduction::validParams()
{
  InputParameters params = KernelGrad::validParams();
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

KokkosAnisoHomogenizedHeatConduction::KokkosAnisoHomogenizedHeatConduction(
    const InputParameters & parameters)
  : KernelGrad(parameters),
    _diffusion_coefficient(
        getKokkosMaterialProperty<Moose::Kokkos::Real33>("diffusion_coefficient")),
    _component(getParam<unsigned int>("component")),
    _dim(_mesh.dimension())
{
  if (_component >= _dim)
    paramError("component", "Value is too large for the mesh dimension: 0, 1, 2 for 1D, 2D, 3D.");
}
