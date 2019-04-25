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

class ComputeSurfaceTensionKKS;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;

template <>
InputParameters validParams<ComputeSurfaceTensionKKS>();

class ComputeSurfaceTensionKKS : public Material
{
public:
  ComputeSurfaceTensionKKS(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Order parameter
  const VariableValue & _v;
  /// Gradient of order parameter
  const VariableGradient & _grad_v;

  /// Material property for gradient energy coefficient
  const MaterialProperty<Real> & _kappa;
  /// Material property for barrier function
  const MaterialProperty<Real> & _g;
  /// double well height parameter
  Real _w;

  MaterialProperty<RankTwoTensor> & _planar_stress;
};

