//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosReporter.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
Reporter::validParams()
{
  InputParameters params = ::Reporter::validParams();

  return params;
}

Reporter::Reporter(const MooseObject * moose_object) : ::Reporter(moose_object) {}

Reporter::Reporter(const Reporter & object) : ::Reporter(object, {}) {}

} // namespace Kokkos
} // namespace Moose
