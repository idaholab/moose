//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * This material converts functor to regular (AD or not) material properties
 */
template <typename T>
class MaterialFunctorConverterTempl : public Material
{
public:
  static InputParameters validParams();

  MaterialFunctorConverterTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  void initQpStatefulProperties() override;

  /// Number of material properties to create
  const std::size_t _num_functors_to_convert;

  /// Incoming functors to convert. We up-convert non-AD functors to AD functors to ease implementation
  std::vector<const Moose::Functor<Moose::GenericType<T, true>> *> _functors_in;
  /// Regular material properties to create
  std::vector<MaterialProperty<T> *> _reg_props_out;
  /// AD material properties to create
  std::vector<ADMaterialProperty<T> *> _ad_props_out;
};

typedef MaterialFunctorConverterTempl<Real> MaterialFunctorConverter;
typedef MaterialFunctorConverterTempl<RealVectorValue> VectorMaterialFunctorConverter;
