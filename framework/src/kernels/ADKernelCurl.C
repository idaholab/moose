//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKernelCurl.h"
#include "Assembly.h"

InputParameters
ADKernelCurl::validParams()
{
  InputParameters params = ADVectorKernel::validParams();
  params.addClassDescription("A base class that defines the AD curl operators.");
  return params;
}

ADKernelCurl::ADKernelCurl(const InputParameters & parameters)
  : ADVectorKernel(parameters),
    _curl_test(_var.curlPhi()),
    _curl_phi(_assembly.curlPhi(_var)),
    _curl_u(_var.adCurlSln())
{
}
