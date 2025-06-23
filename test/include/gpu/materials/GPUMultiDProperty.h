//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUMaterial.h"

template <unsigned int dimension, typename T>
class GPUMultiDProperty : public GPUMaterial<GPUMultiDProperty<dimension, T>>
{
public:
  static InputParameters validParams()
  {
    InputParameters params = GPUMaterial<GPUMultiDProperty<dimension, T>>::validParams();
    params.addParam<MaterialPropertyName>("name", "The name of the property");
    params.addParam<std::vector<unsigned int>>("dims", "The dimensions of the property");
    return params;
  }

  GPUMultiDProperty(const InputParameters & parameters)
    : GPUMaterial<GPUMultiDProperty<dimension, T>>(parameters),
      _dims(this->template getParam<std::vector<unsigned int>>("dims"))
  {
    if (_dims.size() != dimension)
      this->paramError("dims", "Should be ", dimension, "-dimension");

    _property = this->template declareGPUProperty<T, dimension>(
        "name", this->template getParam<std::vector<unsigned int>>("dims"));
  }

  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const {}

protected:
  GPUMaterialProperty<T, dimension> _property;
  GPUArray<unsigned int> _dims;
};

class GPU1DRealProperty final : public GPUMultiDProperty<1, Real>
{
public:
  static InputParameters validParams();

  GPU1DRealProperty(const InputParameters & parameters);
};

class GPU2DRealProperty final : public GPUMultiDProperty<2, Real>
{
public:
  static InputParameters validParams();

  GPU2DRealProperty(const InputParameters & parameters);
};

class GPU3DRealProperty final : public GPUMultiDProperty<3, Real>
{
public:
  static InputParameters validParams();

  GPU3DRealProperty(const InputParameters & parameters);
};

class GPU4DRealProperty final : public GPUMultiDProperty<4, Real>
{
public:
  static InputParameters validParams();

  GPU4DRealProperty(const InputParameters & parameters);
};

class GPU1DIntProperty final : public GPUMultiDProperty<1, int>
{
public:
  static InputParameters validParams();

  GPU1DIntProperty(const InputParameters & parameters);
};

class GPU2DIntProperty final : public GPUMultiDProperty<2, int>
{
public:
  static InputParameters validParams();

  GPU2DIntProperty(const InputParameters & parameters);
};

class GPU3DIntProperty final : public GPUMultiDProperty<3, int>
{
public:
  static InputParameters validParams();

  GPU3DIntProperty(const InputParameters & parameters);
};

class GPU4DIntProperty final : public GPUMultiDProperty<4, int>
{
public:
  static InputParameters validParams();

  GPU4DIntProperty(const InputParameters & parameters);
};
