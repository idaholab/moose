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
NEML2ObjectStubHeader(NEML2ParameterDerivativeToRealMOOSEMaterialProperty, Material);
NEML2ObjectStubHeader(NEML2ParameterDerivativeToStdVectorRealMOOSEMaterialProperty, Material);
NEML2ObjectStubHeader(NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty,
                      Material);
NEML2ObjectStubHeader(NEML2ParameterDerivativeToSymmetricRankFourTensorMOOSEMaterialProperty,
                      Material);
#else

#include "neml2/tensors/TensorBase.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

class ExecuteNEML2Model;

template <typename T>
class NEML2ParameterDerivativeToMOOSEMaterialProperty : public Material
{
public:
  static InputParameters validParams();

  NEML2ParameterDerivativeToMOOSEMaterialProperty(const InputParameters & params);

  virtual void computeProperties() override;

protected:
  virtual void initQpStatefulProperties() override{};

  /// User object managing the execution of the NEML2 model
  const ExecuteNEML2Model & _execute_neml2_model;

  /// Emitted material property
  MaterialProperty<T> & _prop;

  /// the requested parameter derivative
  const neml2::Tensor & _output_view;
};

typedef NEML2ParameterDerivativeToMOOSEMaterialProperty<Real>
    NEML2ParameterDerivativeToRealMOOSEMaterialProperty;
typedef NEML2ParameterDerivativeToMOOSEMaterialProperty<std::vector<Real>>
    NEML2ParameterDerivativeToStdVectorRealMOOSEMaterialProperty;
typedef NEML2ParameterDerivativeToMOOSEMaterialProperty<SymmetricRankTwoTensor>
    NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty;
typedef NEML2ParameterDerivativeToMOOSEMaterialProperty<SymmetricRankFourTensor>
    NEML2ParameterDerivativeToSymmetricRankFourTensorMOOSEMaterialProperty;

#endif
