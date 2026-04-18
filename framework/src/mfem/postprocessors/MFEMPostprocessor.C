//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPostprocessor.h"

namespace Moose::MFEM
{
InputParameters
Postprocessor::validParams()
{
  InputParameters params = ExecutedObject::validParams();
  params += ::Postprocessor::validParams();
  params.registerSystemAttributeName("Moose::MFEM::ExecutedObject");
  return params;
}

Postprocessor::Postprocessor(const InputParameters & parameters)
  : ExecutedObject(parameters), ::Postprocessor(this)
{
}

std::optional<std::string>
Postprocessor::suppliedPostprocessorName() const
{
  return name();
}

} // namespace Moose::MFEM
#endif
