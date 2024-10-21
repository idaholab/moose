//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NEML2Utils.h"
#include "Material.h"

#ifndef NEML2_ENABLED
NEML2ObjectStubHeader(NEML2ToRealMOOSEMaterialProperty, Material);
NEML2ObjectStubHeader(NEML2ToStdVectorRealMOOSEMaterialProperty, Material);
NEML2ObjectStubHeader(NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty, Material);
NEML2ObjectStubHeader(NEML2ToSymmetricRankFourTensorMOOSEMaterialProperty, Material);
#else

#include "neml2/tensors/TensorBase.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

class ExecuteNEML2Model;

template <typename T>
class NEML2ToMOOSEMaterialProperty : public Material
{
public:
  static InputParameters validParams();

  NEML2ToMOOSEMaterialProperty(const InputParameters & params);

  virtual void computeProperties() override;

protected:
  virtual void initQpStatefulProperties() override;

  /// User object managing the execution of the NEML2 model
  const ExecuteNEML2Model & _execute_neml2_model;

  /// Emitted material property
  MaterialProperty<T> & _prop;

  /// Initial condition
  const MaterialProperty<T> * _prop0;

  /// labled view to the requested output
  const neml2::Tensor & _output_view;
};

typedef NEML2ToMOOSEMaterialProperty<Real> NEML2ToRealMOOSEMaterialProperty;
typedef NEML2ToMOOSEMaterialProperty<std::vector<Real>> NEML2ToStdVectorRealMOOSEMaterialProperty;
typedef NEML2ToMOOSEMaterialProperty<SymmetricRankTwoTensor>
    NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty;
typedef NEML2ToMOOSEMaterialProperty<SymmetricRankFourTensor>
    NEML2ToSymmetricRankFourTensorMOOSEMaterialProperty;

#endif
