//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NEML2Utils.h"
#include "Material.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

#ifdef NEML2_ENABLED
#include "neml2/tensors/TensorBase.h"
#endif

class NEML2ModelExecutor;

template <typename T>
class NEML2ToMOOSEMaterialProperty : public Material
{
public:
  static InputParameters validParams();

  NEML2ToMOOSEMaterialProperty(const InputParameters & params);

#ifdef NEML2_ENABLED
  void computeProperties() override;

protected:
  void initQpStatefulProperties() override {}
  void computeQpProperties() override;

  /// User object managing the execution of the NEML2 model
  const NEML2ModelExecutor & _execute_neml2_model;

  /// Emitted material property
  MaterialProperty<T> & _prop;

  /// Initial condition
  const MaterialProperty<T> * _prop0;

  /// Reference to the requested output (or its derivative) value
  const neml2::Tensor & _value;

  /// Batch index offset
  std::size_t _batch_idx;
#endif
};

#define DefineNEML2ToMOOSEMaterialProperty(T)                                                      \
  using NEML2ToMOOSE##T##MaterialProperty = NEML2ToMOOSEMaterialProperty<T>

DefineNEML2ToMOOSEMaterialProperty(Real);
DefineNEML2ToMOOSEMaterialProperty(SymmetricRankTwoTensor);
DefineNEML2ToMOOSEMaterialProperty(SymmetricRankFourTensor);
DefineNEML2ToMOOSEMaterialProperty(RealVectorValue);
DefineNEML2ToMOOSEMaterialProperty(RankTwoTensor);
DefineNEML2ToMOOSEMaterialProperty(RankFourTensor);

#undef DefineNEML2ToMOOSEMaterialProperty
