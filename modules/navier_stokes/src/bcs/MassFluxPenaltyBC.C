//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassFluxPenaltyBC.h"
#include "MassFluxPenaltyIPHDGAssemblyHelper.h"

registerMooseObject("NavierStokesApp", MassFluxPenaltyBC);

InputParameters
MassFluxPenaltyBC::validParams()
{
  InputParameters params = IPHDGBC::validParams();
  params += MassFluxPenaltyIPHDGAssemblyHelper::validParams();
  params.addClassDescription("introduces a jump correction on exterior faces for grad-div "
                             "stabilization for discontinuous Galerkin methods.");
  params.addRequiredParam<bool>("dirichlet_boundary",
                                "Whether this is a Dirichlet boundary for the velocity. If it is, "
                                "then we will not compute the trace residuals");
  return params;
}

MassFluxPenaltyBC::MassFluxPenaltyBC(const InputParameters & params)
  : IPHDGBC(params),
    _iphdg_helper(std::make_unique<MassFluxPenaltyIPHDGAssemblyHelper>(
        this, this, this, _sys, _assembly, _tid, std::set<SubdomainID>{}, boundaryIDs())),
    _dirichlet_boundary(getParam<bool>("dirichlet_boundary"))
{
}

void
MassFluxPenaltyBC::compute()
{
  auto & iphdg_helper = iphdgHelper();
  iphdg_helper.resizeResiduals();

  // u, lm_u
  iphdg_helper.scalarFace();
  if (!_dirichlet_boundary)
    iphdg_helper.lmFace();
}

IPHDGAssemblyHelper &
MassFluxPenaltyBC::iphdgHelper()
{
  return *_iphdg_helper;
}
