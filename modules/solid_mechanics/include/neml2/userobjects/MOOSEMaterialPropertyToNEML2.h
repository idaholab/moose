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
#include "MOOSEToNEML2.h"

#ifndef NEML2_ENABLED
NEML2ObjectStubHeader(MOOSERealMaterialPropertyToNEML2, ElementUserObject);
NEML2ObjectStubHeader(MOOSERankTwoTensorMaterialPropertyToNEML2, ElementUserObject);
NEML2ObjectStubHeader(MOOSESymmetricRankTwoTensorMaterialPropertyToNEML2, ElementUserObject);
NEML2ObjectStubHeader(MOOSEStdVectorRealMaterialPropertyToNEML2, ElementUserObject);

NEML2ObjectStubHeader(MOOSEOldRealMaterialPropertyToNEML2, ElementUserObject);
NEML2ObjectStubHeader(MOOSEOldRankTwoTensorMaterialPropertyToNEML2, ElementUserObject);
NEML2ObjectStubHeader(MOOSEOldSymmetricRankTwoTensorMaterialPropertyToNEML2, ElementUserObject);
NEML2ObjectStubHeader(MOOSEOldStdVectorRealMaterialPropertyToNEML2, ElementUserObject);
#else

/**
 * Gather a MOOSE material property for insertion into the specified input of a NEML2 model.
 */
template <typename T, unsigned int state>
class MOOSEMaterialPropertyToNEML2 : public MOOSEToNEML2
{
public:
  static InputParameters validParams();

  MOOSEMaterialPropertyToNEML2(const InputParameters & params);

protected:
  virtual torch::Tensor convertQpMOOSEData() const override;

  /// MOOSE material property to read data from
  const MaterialProperty<T> & _mat_prop;
};

typedef MOOSEMaterialPropertyToNEML2<Real, 0> MOOSERealMaterialPropertyToNEML2;
typedef MOOSEMaterialPropertyToNEML2<RankTwoTensor, 0> MOOSERankTwoTensorMaterialPropertyToNEML2;
typedef MOOSEMaterialPropertyToNEML2<SymmetricRankTwoTensor, 0>
    MOOSESymmetricRankTwoTensorMaterialPropertyToNEML2;
typedef MOOSEMaterialPropertyToNEML2<std::vector<Real>, 0>
    MOOSEStdVectorRealMaterialPropertyToNEML2;

typedef MOOSEMaterialPropertyToNEML2<Real, 1> MOOSEOldRealMaterialPropertyToNEML2;
typedef MOOSEMaterialPropertyToNEML2<RankTwoTensor, 1> MOOSEOldRankTwoTensorMaterialPropertyToNEML2;
typedef MOOSEMaterialPropertyToNEML2<SymmetricRankTwoTensor, 1>
    MOOSEOldSymmetricRankTwoTensorMaterialPropertyToNEML2;
typedef MOOSEMaterialPropertyToNEML2<std::vector<Real>, 1>
    MOOSEOldStdVectorRealMaterialPropertyToNEML2;

#endif
