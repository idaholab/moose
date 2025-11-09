//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSideIntegralMaterialProperty.h"

registerKokkosPostprocessor("MooseApp", KokkosSideIntegralMaterialProperty);

InputParameters
KokkosSideIntegralMaterialProperty::validParams()
{
  InputParameters params =
      KokkosIntegralMaterialProperty<KokkosSideIntegralPostprocessor>::validParams();
  params.renameParam("mat_prop", "property", "The name of the material property");
  params.addClassDescription("Compute the integral of the material property over the domain");
  return params;
}

KokkosSideIntegralMaterialProperty::KokkosSideIntegralMaterialProperty(
    const InputParameters & parameters)
  : KokkosIntegralMaterialProperty<KokkosSideIntegralPostprocessor>(parameters)
{
}
