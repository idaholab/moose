//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KernelCurl.h"
#include "Assembly.h"

InputParameters
KernelCurl::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription("A base class that defines the curl operators.");
  return params;
}

KernelCurl::KernelCurl(const InputParameters & parameters)
  : VectorKernel(parameters),
    _curl_test(_var.curlPhi()),
    _curl_phi(_assembly.curlPhi(_var)),
    _curl_u(_is_implicit ? _var.curlSln() : _var.curlSlnOld())
{
}
