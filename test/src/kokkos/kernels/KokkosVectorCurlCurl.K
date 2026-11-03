//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorCurlCurl.h"

registerKokkosResidualObject("MooseTestApp", KokkosVectorCurlCurl);

InputParameters
KokkosVectorCurlCurl::validParams()
{
  InputParameters params = Moose::Kokkos::VectorKernel::validParams();
  params.addClassDescription("Curl-curl operator with a reaction term, for testing.");
  return params;
}

KokkosVectorCurlCurl::KokkosVectorCurlCurl(const InputParameters & parameters)
  : Moose::Kokkos::VectorKernel(parameters)
{
}
