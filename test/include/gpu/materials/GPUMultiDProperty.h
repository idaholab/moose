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
class KokkosMultiDProperty : public Moose::Kokkos::Material<KokkosMultiDProperty<dimension, T>>
{
public:
  static InputParameters validParams()
  {
    InputParameters params =
        Moose::Kokkos::Material<KokkosMultiDProperty<dimension, T>>::validParams();
    params.addParam<MaterialPropertyName>("name", "The name of the property");
    params.addParam<std::vector<unsigned int>>("dims", "The dimensions of the property");
    return params;
  }

  KokkosMultiDProperty(const InputParameters & parameters)
    : Moose::Kokkos::Material<KokkosMultiDProperty<dimension, T>>(parameters),
      _dims(this->template getParam<std::vector<unsigned int>>("dims"))
  {
    if (_dims.size() != dimension)
      this->paramError("dims", "Should be ", dimension, "-dimension");

    _property = this->template declareKokkosProperty<T, dimension>(
        "name", this->template getParam<std::vector<unsigned int>>("dims"));
  }

  KOKKOS_FUNCTION void computeQpProperties(const unsigned int /* qp */, Datum & /* datum */) const
  {
  }

protected:
  Moose::Kokkos::MaterialProperty<T, dimension> _property;
  Moose::Kokkos::Array<unsigned int> _dims;
};

class Kokkos1DRealProperty final : public KokkosMultiDProperty<1, Real>
{
public:
  static InputParameters validParams();

  Kokkos1DRealProperty(const InputParameters & parameters);
};

class Kokkos2DRealProperty final : public KokkosMultiDProperty<2, Real>
{
public:
  static InputParameters validParams();

  Kokkos2DRealProperty(const InputParameters & parameters);
};

class Kokkos3DRealProperty final : public KokkosMultiDProperty<3, Real>
{
public:
  static InputParameters validParams();

  Kokkos3DRealProperty(const InputParameters & parameters);
};

class Kokkos4DRealProperty final : public KokkosMultiDProperty<4, Real>
{
public:
  static InputParameters validParams();

  Kokkos4DRealProperty(const InputParameters & parameters);
};

class Kokkos1DIntProperty final : public KokkosMultiDProperty<1, int>
{
public:
  static InputParameters validParams();

  Kokkos1DIntProperty(const InputParameters & parameters);
};

class Kokkos2DIntProperty final : public KokkosMultiDProperty<2, int>
{
public:
  static InputParameters validParams();

  Kokkos2DIntProperty(const InputParameters & parameters);
};

class Kokkos3DIntProperty final : public KokkosMultiDProperty<3, int>
{
public:
  static InputParameters validParams();

  Kokkos3DIntProperty(const InputParameters & parameters);
};

class Kokkos4DIntProperty final : public KokkosMultiDProperty<4, int>
{
public:
  static InputParameters validParams();

  Kokkos4DIntProperty(const InputParameters & parameters);
};
