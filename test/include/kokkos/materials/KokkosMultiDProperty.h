//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterial.h"

template <unsigned int dimension, typename T>
class KokkosMultiDProperty : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosMultiDProperty(const InputParameters & parameters);

  KOKKOS_FUNCTION void computeQpProperties(const unsigned int /* qp */, Datum & /* datum */) const
  {
  }

protected:
  Moose::Kokkos::MaterialProperty<T, dimension> _property;
  const Moose::Kokkos::Array<unsigned int> _dims;
};

template <unsigned int dimension, typename T>
InputParameters
KokkosMultiDProperty<dimension, T>::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<MaterialPropertyName>("name", "The name of the property");
  params.addParam<std::vector<unsigned int>>("dims", "The dimensions of the property");
  return params;
}

template <unsigned int dimension, typename T>
KokkosMultiDProperty<dimension, T>::KokkosMultiDProperty(const InputParameters & parameters)
  : Material(parameters), _dims(getParam<std::vector<unsigned int>>("dims"))
{
  if (_dims.size() != dimension)
    paramError("dims", "Should be ", dimension, "-dimension");

  _property =
      declareKokkosProperty<T, dimension>("name", getParam<std::vector<unsigned int>>("dims"));
}

class Kokkos1DRealProperty : public KokkosMultiDProperty<1, Real>
{
public:
  static InputParameters validParams();

  Kokkos1DRealProperty(const InputParameters & parameters);
};

class Kokkos2DRealProperty : public KokkosMultiDProperty<2, Real>
{
public:
  static InputParameters validParams();

  Kokkos2DRealProperty(const InputParameters & parameters);
};

class Kokkos3DRealProperty : public KokkosMultiDProperty<3, Real>
{
public:
  static InputParameters validParams();

  Kokkos3DRealProperty(const InputParameters & parameters);
};

class Kokkos4DRealProperty : public KokkosMultiDProperty<4, Real>
{
public:
  static InputParameters validParams();

  Kokkos4DRealProperty(const InputParameters & parameters);
};

class Kokkos1DIntProperty : public KokkosMultiDProperty<1, int>
{
public:
  static InputParameters validParams();

  Kokkos1DIntProperty(const InputParameters & parameters);
};

class Kokkos2DIntProperty : public KokkosMultiDProperty<2, int>
{
public:
  static InputParameters validParams();

  Kokkos2DIntProperty(const InputParameters & parameters);
};

class Kokkos3DIntProperty : public KokkosMultiDProperty<3, int>
{
public:
  static InputParameters validParams();

  Kokkos3DIntProperty(const InputParameters & parameters);
};

class Kokkos4DIntProperty : public KokkosMultiDProperty<4, int>
{
public:
  static InputParameters validParams();

  Kokkos4DIntProperty(const InputParameters & parameters);
};
