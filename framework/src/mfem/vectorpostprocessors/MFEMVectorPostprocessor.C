//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorPostprocessor.h"

namespace Moose::MFEM
{
InputParameters
VectorPostprocessor::validParams()
{
  InputParameters params = ExecutedObject::validParams();
  params += ::VectorPostprocessor::validParams();
  params.registerSystemAttributeName("Moose::MFEM::ExecutedObject");
  return params;
}

VectorPostprocessor::VectorPostprocessor(const InputParameters & parameters)
  : ExecutedObject(parameters), ::VectorPostprocessor(this)
{
}

std::optional<std::string>
VectorPostprocessor::suppliedVectorPostprocessorName() const
{
  return name();
}

} // namespace Moose::MFEM
#endif // MOOSE_MFEM_ENABLED
