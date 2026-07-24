//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosKernelCurl.h"

namespace Moose::Kokkos
{

InputParameters
KernelCurl::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription(
      "A base class that defines the curl operators for Kokkos vector kernels.");
  return params;
}

KernelCurl::KernelCurl(const InputParameters & parameters)
  : VectorKernel(parameters), _curl_test(), _curl_phi(), _curl_u(_var)
{
}

} // namespace Moose::Kokkos
