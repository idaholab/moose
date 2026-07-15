//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IPHDGBC.h"
#include "IPHDGAssemblyHelper.h"

InputParameters
IPHDGBC::validParams()
{
  return HybridizedDGBC::validParams();
}

IPHDGBC::IPHDGBC(const InputParameters & parameters) : HybridizedDGBC(parameters) {}

HybridizedDGAssemblyHelper &
IPHDGBC::hybridizedDGHelper()
{
  return iphdgHelper();
}
