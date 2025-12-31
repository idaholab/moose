//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPostprocessor.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
Postprocessor::validParams()
{
  InputParameters params = ::Postprocessor::validParams();

  return params;
}

Postprocessor::Postprocessor(const MooseObject * moose_object) : ::Postprocessor(moose_object) {}

Postprocessor::Postprocessor(const Postprocessor & object) : ::Postprocessor(object, {}) {}

} // namespace Kokkos
} // namespace Moose
