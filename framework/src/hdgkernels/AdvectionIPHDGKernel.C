//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionIPHDGAssemblyHelper.h"
#include "AdvectionIPHDGKernel.h"

registerMooseObject("MooseApp", AdvectionIPHDGKernel);

InputParameters
AdvectionIPHDGKernel::validParams()
{
  auto params = TwoFieldScalarHDGKernel::validParams();
  params += AdvectionIPHDGAssemblyHelper::validParams();
  params.addClassDescription("Adds element and interior face integrals for a hybridized interior "
                             "penalty discontinuous Galerkin discretization of an advection term.");
  return params;
}

AdvectionIPHDGKernel::AdvectionIPHDGKernel(const InputParameters & params)
  : TwoFieldScalarHDGKernel(params),
    _iphdg_helper(std::make_unique<AdvectionIPHDGAssemblyHelper>(
        this, this, this, _sys, _assembly, _tid, blockIDs(), std::set<BoundaryID>{}))
{
}

TwoFieldScalarHDGAssemblyHelper &
AdvectionIPHDGKernel::hdgHelper()
{
  return *_iphdg_helper;
}
