//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassContinuityAssemblyHelper.h"
#include "MassContinuityIPHDGKernel.h"

registerMooseObject("NavierStokesApp", MassContinuityIPHDGKernel);

InputParameters
MassContinuityIPHDGKernel::validParams()
{
  auto params = IPHDGKernel::validParams();
  params += MassContinuityAssemblyHelper::validParams();
  params.addClassDescription("Adds element and interior face integrals for a hybridized interior "
                             "penalty discontinuous Galerkin discretization of a conservation of "
                             "mass term for incompressible Navier-Stokes.");
  return params;
}

MassContinuityIPHDGKernel::MassContinuityIPHDGKernel(const InputParameters & params)
  : IPHDGKernel(params),
    _iphdg_helper(std::make_unique<MassContinuityAssemblyHelper>(
        this, this, this, _mesh, _sys, _assembly, _tid, blockIDs(), std::set<BoundaryID>{}))
{
}

IPHDGAssemblyHelper &
MassContinuityIPHDGKernel::iphdgHelper()
{
  return *_iphdg_helper;
}
