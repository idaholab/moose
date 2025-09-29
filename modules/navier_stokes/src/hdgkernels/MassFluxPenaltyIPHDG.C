//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassFluxPenaltyIPHDG.h"
#include "MassFluxPenaltyIPHDGAssemblyHelper.h"

registerMooseObject("NavierStokesApp", MassFluxPenaltyIPHDG);

InputParameters
MassFluxPenaltyIPHDG::validParams()
{
  InputParameters params = IPHDGKernel::validParams();
  params += MassFluxPenaltyIPHDGAssemblyHelper::validParams();
  params.addClassDescription("introduces a jump correction on internal faces for grad-div "
                             "stabilization for discontinuous Galerkin methods.");
  return params;
}

MassFluxPenaltyIPHDG::MassFluxPenaltyIPHDG(const InputParameters & params)
  : IPHDGKernel(params),
    _iphdg_helper(std::make_unique<MassFluxPenaltyIPHDGAssemblyHelper>(
        this, this, this, _sys, _assembly, _tid, blockIDs(), std::set<BoundaryID>{}))
{
}

IPHDGAssemblyHelper &
MassFluxPenaltyIPHDG::iphdgHelper()
{
  return *_iphdg_helper;
}
