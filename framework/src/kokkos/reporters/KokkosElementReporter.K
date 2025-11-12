//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosElementReporter.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
ElementReporter::validParams()
{
  InputParameters params = ElementReducer::validParams();
  params += Reporter::validParams();
  return params;
}

ElementReporter::ElementReporter(const InputParameters & parameters)
  : ElementReducer(parameters), Reporter(this)
{
}

} // namespace Kokkos
} // namespace Moose
