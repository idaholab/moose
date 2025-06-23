//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUMultiDProperty.h"

registerMooseObject("MooseTestApp", GPU1DRealProperty);
registerMooseObject("MooseTestApp", GPU2DRealProperty);
registerMooseObject("MooseTestApp", GPU3DRealProperty);
registerMooseObject("MooseTestApp", GPU4DRealProperty);
registerMooseObject("MooseTestApp", GPU1DIntProperty);
registerMooseObject("MooseTestApp", GPU2DIntProperty);
registerMooseObject("MooseTestApp", GPU3DIntProperty);
registerMooseObject("MooseTestApp", GPU4DIntProperty);

InputParameters
GPU1DRealProperty::validParams()
{
  InputParameters params = GPUMultiDProperty<1, Real>::validParams();
  return params;
}

GPU1DRealProperty::GPU1DRealProperty(const InputParameters & parameters)
  : GPUMultiDProperty<1, Real>(parameters)
{
}

InputParameters
GPU2DRealProperty::validParams()
{
  InputParameters params = GPUMultiDProperty<2, Real>::validParams();
  return params;
}

GPU2DRealProperty::GPU2DRealProperty(const InputParameters & parameters)
  : GPUMultiDProperty<2, Real>(parameters)
{
}

InputParameters
GPU3DRealProperty::validParams()
{
  InputParameters params = GPUMultiDProperty<3, Real>::validParams();
  return params;
}

GPU3DRealProperty::GPU3DRealProperty(const InputParameters & parameters)
  : GPUMultiDProperty<3, Real>(parameters)
{
}

InputParameters
GPU4DRealProperty::validParams()
{
  InputParameters params = GPUMultiDProperty<4, Real>::validParams();
  return params;
}

GPU4DRealProperty::GPU4DRealProperty(const InputParameters & parameters)
  : GPUMultiDProperty<4, Real>(parameters)
{
}

InputParameters
GPU1DIntProperty::validParams()
{
  InputParameters params = GPUMultiDProperty<1, int>::validParams();
  return params;
}

GPU1DIntProperty::GPU1DIntProperty(const InputParameters & parameters)
  : GPUMultiDProperty<1, int>(parameters)
{
}

InputParameters
GPU2DIntProperty::validParams()
{
  InputParameters params = GPUMultiDProperty<2, int>::validParams();
  return params;
}

GPU2DIntProperty::GPU2DIntProperty(const InputParameters & parameters)
  : GPUMultiDProperty<2, int>(parameters)
{
}

InputParameters
GPU3DIntProperty::validParams()
{
  InputParameters params = GPUMultiDProperty<3, int>::validParams();
  return params;
}

GPU3DIntProperty::GPU3DIntProperty(const InputParameters & parameters)
  : GPUMultiDProperty<3, int>(parameters)
{
}

InputParameters
GPU4DIntProperty::validParams()
{
  InputParameters params = GPUMultiDProperty<4, int>::validParams();
  return params;
}

GPU4DIntProperty::GPU4DIntProperty(const InputParameters & parameters)
  : GPUMultiDProperty<4, int>(parameters)
{
}
