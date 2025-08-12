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
 * Declares material properties based on names and functions prescribed by input parameters.
 *
 * This is identical in function to the GenericFunctionMaterial in Moose.
 */
class MFEMGenericFunctorMaterial : public MFEMFunctorMaterial
{
public:
  static InputParameters validParams();

  MFEMGenericFunctorMaterial(const InputParameters & parameters);
  virtual ~MFEMGenericFunctorMaterial();

protected:
  const std::vector<std::string> & _prop_names;
  const std::vector<MFEMScalarCoefficientName> & _prop_values;
};

#endif
