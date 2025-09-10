//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMultiDProperty.h"

registerKokkosMaterial("MooseTestApp", Kokkos1DRealProperty);
registerKokkosMaterial("MooseTestApp", Kokkos2DRealProperty);
registerKokkosMaterial("MooseTestApp", Kokkos3DRealProperty);
registerKokkosMaterial("MooseTestApp", Kokkos4DRealProperty);
registerKokkosMaterial("MooseTestApp", Kokkos1DIntProperty);
registerKokkosMaterial("MooseTestApp", Kokkos2DIntProperty);
registerKokkosMaterial("MooseTestApp", Kokkos3DIntProperty);
registerKokkosMaterial("MooseTestApp", Kokkos4DIntProperty);

InputParameters
Kokkos1DRealProperty::validParams()
{
  InputParameters params = KokkosMultiDProperty<1, Real>::validParams();
  return params;
}

Kokkos1DRealProperty::Kokkos1DRealProperty(const InputParameters & parameters)
  : KokkosMultiDProperty<1, Real>(parameters)
{
}

InputParameters
Kokkos2DRealProperty::validParams()
{
  InputParameters params = KokkosMultiDProperty<2, Real>::validParams();
  return params;
}

Kokkos2DRealProperty::Kokkos2DRealProperty(const InputParameters & parameters)
  : KokkosMultiDProperty<2, Real>(parameters)
{
}

InputParameters
Kokkos3DRealProperty::validParams()
{
  InputParameters params = KokkosMultiDProperty<3, Real>::validParams();
  return params;
}

Kokkos3DRealProperty::Kokkos3DRealProperty(const InputParameters & parameters)
  : KokkosMultiDProperty<3, Real>(parameters)
{
}

InputParameters
Kokkos4DRealProperty::validParams()
{
  InputParameters params = KokkosMultiDProperty<4, Real>::validParams();
  return params;
}

Kokkos4DRealProperty::Kokkos4DRealProperty(const InputParameters & parameters)
  : KokkosMultiDProperty<4, Real>(parameters)
{
}

InputParameters
Kokkos1DIntProperty::validParams()
{
  InputParameters params = KokkosMultiDProperty<1, int>::validParams();
  return params;
}

Kokkos1DIntProperty::Kokkos1DIntProperty(const InputParameters & parameters)
  : KokkosMultiDProperty<1, int>(parameters)
{
}

InputParameters
Kokkos2DIntProperty::validParams()
{
  InputParameters params = KokkosMultiDProperty<2, int>::validParams();
  return params;
}

Kokkos2DIntProperty::Kokkos2DIntProperty(const InputParameters & parameters)
  : KokkosMultiDProperty<2, int>(parameters)
{
}

InputParameters
Kokkos3DIntProperty::validParams()
{
  InputParameters params = KokkosMultiDProperty<3, int>::validParams();
  return params;
}

Kokkos3DIntProperty::Kokkos3DIntProperty(const InputParameters & parameters)
  : KokkosMultiDProperty<3, int>(parameters)
{
}

InputParameters
Kokkos4DIntProperty::validParams()
{
  InputParameters params = KokkosMultiDProperty<4, int>::validParams();
  return params;
}

Kokkos4DIntProperty::Kokkos4DIntProperty(const InputParameters & parameters)
  : KokkosMultiDProperty<4, int>(parameters)
{
}
