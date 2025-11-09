//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosIntegralMaterialProperty.h"

template <typename Base>
InputParameters
KokkosIntegralMaterialProperty<Base>::validParams()
{
  InputParameters params = Base::validParams();
  params.addRequiredParam<MaterialPropertyName>("mat_prop", "The name of the material property");
  params.addClassDescription("Compute the integral of the material property over the domain");
  return params;
}

template <typename Base>
KokkosIntegralMaterialProperty<Base>::KokkosIntegralMaterialProperty(
    const InputParameters & parameters)
  : Base(parameters), _scalar(Base::template getKokkosMaterialProperty<Real>("mat_prop"))
{
}

template class KokkosIntegralMaterialProperty<KokkosElementIntegralPostprocessor>;
template class KokkosIntegralMaterialProperty<KokkosSideIntegralPostprocessor>;
