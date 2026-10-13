//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosAnisoHeatConduction.h"

registerKokkosResidualObject("HeatTransferApp", KokkosAnisoHeatConduction);

InputParameters
KokkosAnisoHeatConduction::validParams()
{
  InputParameters params = KernelGrad::validParams();
  params.addClassDescription("Anisotropic diffusive heat conduction term $\\nabla \\cdot "
                             "-\\mathbf{k} \\nabla T$ of the thermal energy conservation equation");
  params.addParam<MaterialPropertyName>(
      "thermal_conductivity",
      "thermal_conductivity",
      "Material property providing thermal conductivity of the material.");
  return params;
}

KokkosAnisoHeatConduction::KokkosAnisoHeatConduction(const InputParameters & parameters)
  : DerivativeMaterialInterface<KernelGrad>(parameters),
    _k(getKokkosMaterialProperty<Real33>("thermal_conductivity")),
    _dk_dT(getKokkosMaterialPropertyDerivative<Real33>("thermal_conductivity", _var.name()))
{
}
