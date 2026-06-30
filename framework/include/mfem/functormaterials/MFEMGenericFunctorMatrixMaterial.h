//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMFunctorMaterial.h"
#include "MFEMContainers.h"

/**
 * Declares constant matrix material properties based on names and values prescribed by input
 * parameters.
 *
 * This is the matrix counterpart of MFEMGenericFunctorMaterial and
 * MFEMGenericFunctorVectorMaterial.
 */
class MFEMGenericFunctorMatrixMaterial : public MFEMFunctorMaterial
{
public:
  static InputParameters validParams();

  MFEMGenericFunctorMatrixMaterial(const InputParameters & parameters);
  virtual ~MFEMGenericFunctorMatrixMaterial();

protected:
  const std::vector<std::string> & _prop_names;
  const std::vector<MFEMMatrixCoefficientName> _prop_values;
};

#endif
