//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionIPHDGAssemblyHelper.h"
#include "AdvectionIPHDGOutflowBC.h"

registerMooseObject("MooseApp", AdvectionIPHDGOutflowBC);

InputParameters
AdvectionIPHDGOutflowBC::validParams()
{
  auto params = IPHDGBC::validParams();
  params += AdvectionIPHDGAssemblyHelper::validParams();
  params.addClassDescription("Implements an outflow boundary condition for use with a hybridized "
                             "discretization of the advection equation");
  params.addRequiredParam<bool>("constrain_lm",
                                "Whether to constrain the Lagrange multiplier to weakly match the "
                                "interior solution on this boundary. This should be set to true "
                                "for pure advection problems and likely false otherwise.");
  return params;
}

AdvectionIPHDGOutflowBC::AdvectionIPHDGOutflowBC(const InputParameters & parameters)
  : IPHDGBC(parameters),
    _iphdg_helper(std::make_unique<AdvectionIPHDGAssemblyHelper>(
        this, this, this, _sys, _assembly, _tid, std::set<SubdomainID>{}, boundaryIDs())),
    _constrain_lm(getParam<bool>("constrain_lm"))
{
}

void
AdvectionIPHDGOutflowBC::compute()
{
  auto & iphdg_helper = iphdgHelper();
  iphdg_helper.resizeResiduals();

  // u, lm_u
  iphdg_helper.scalarFace();
  if (_constrain_lm)
    iphdg_helper.lmOutflow();
}

AdvectionIPHDGAssemblyHelper &
AdvectionIPHDGOutflowBC::iphdgHelper()
{
  return *_iphdg_helper;
}
