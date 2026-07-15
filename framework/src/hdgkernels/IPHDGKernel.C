//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IPHDGAssemblyHelper.h"
#include "IPHDGKernel.h"

InputParameters
IPHDGKernel::validParams()
{
  return HybridizedDGKernel::validParams();
}

IPHDGKernel::IPHDGKernel(const InputParameters & params) : HybridizedDGKernel(params) {}

HybridizedDGAssemblyHelper &
IPHDGKernel::hybridizedDGHelper()
{
  return iphdgHelper();
}
