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
NEML2ObjectStubHeader(MOOSEStdVectorRealMaterialPropertyToNEML2, ElementUserObject);
#else

/**
 * Gather a MOOSE material property for insertion into the specified input of a NEML2 model.
 */
template <typename T>
class MOOSEMaterialPropertyToNEML2 : public MOOSEToNEML2
{
public:
  static InputParameters validParams();

  MOOSEMaterialPropertyToNEML2(const InputParameters & params);

protected:
  virtual void execute() override;

  /// MOOSE material property to read data from
  const MaterialProperty<T> & _mat_prop;
};

typedef MOOSEMaterialPropertyToNEML2<Real> MOOSERealMaterialPropertyToNEML2;
typedef MOOSEMaterialPropertyToNEML2<RankTwoTensor> MOOSERankTwoTensorMaterialPropertyToNEML2;
typedef MOOSEMaterialPropertyToNEML2<std::vector<Real>> MOOSEStdVectorRealMaterialPropertyToNEML2;

#endif
