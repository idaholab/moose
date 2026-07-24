//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionLHDGAssemblyHelper.h"
#include "AdvectionLHDGOutflowBC.h"

registerMooseObject("NavierStokesApp", AdvectionLHDGOutflowBC);

InputParameters
AdvectionLHDGOutflowBC::validParams()
{
  auto params = TwoFieldScalarHDGBC::validParams();
  params += AdvectionLHDGAssemblyHelper::validParams();
  params.addRequiredParam<bool>("constrain_lm",
                                "Whether to constrain the facet scalar to weakly match the cell "
                                "scalar. Set false when diffusion supplies the trace equation.");
  params.addClassDescription(
      "Implements an advective outflow boundary condition using the L-HDG hybrid velocity.");
  return params;
}

AdvectionLHDGOutflowBC::AdvectionLHDGOutflowBC(const InputParameters & parameters)
  : TwoFieldScalarHDGBC(parameters),
    _lhdg_helper(std::make_unique<AdvectionLHDGAssemblyHelper>(
        this, this, this, _sys, _assembly, _tid, std::set<SubdomainID>{}, boundaryIDs())),
    _constrain_lm(getParam<bool>("constrain_lm"))
{
}

void
AdvectionLHDGOutflowBC::compute(TwoFieldScalarHDGAssemblyHelper &)
{
  _lhdg_helper->resizeResiduals();
  _lhdg_helper->scalarFace();
  if (_constrain_lm)
    _lhdg_helper->lmOutflow();
}

TwoFieldScalarHDGAssemblyHelper &
AdvectionLHDGOutflowBC::hdgHelper()
{
  return *_lhdg_helper;
}
