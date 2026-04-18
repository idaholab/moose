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

namespace Moose::MFEM
{
/**
 * Declares material properties based on names and functions prescribed by input parameters.
 *
 * This is identical in function to the GenericFunctionVectorMaterial in Moose.
 */
class GenericFunctorVectorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  GenericFunctorVectorMaterial(const InputParameters & parameters);
  virtual ~GenericFunctorVectorMaterial();

protected:
  const std::vector<std::string> & _prop_names;
  const std::vector<Moose::MFEM::VectorCoefficientName> _prop_values;
};

} // namespace Moose::MFEM
#endif
