//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseFunctorArguments.h"
#include "MooseTypes.h"

namespace Moose
{
const std::set<SubdomainID> NodeArg::undefined_subdomain_connection = {Moose::INVALID_BLOCK_ID};
}
