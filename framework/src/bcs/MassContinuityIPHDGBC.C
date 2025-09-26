//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassContinuityIPHDGBC.h"
#include "MassContinuityAssemblyHelper.h"

registerMooseObject("MooseApp", MassContinuityIPHDGBC);

InputParameters
MassContinuityIPHDGBC::validParams()
{
  auto params = IPHDGBC::validParams();
  params.addClassDescription("Adds to mass conservation terms on boundary faces");
  params += MassContinuityAssemblyHelper::validParams();
  return params;
}

MassContinuityIPHDGBC::MassContinuityIPHDGBC(const InputParameters & parameters)
  : IPHDGBC(parameters),
    _iphdg_helper(std::make_unique<MassContinuityAssemblyHelper>(
        this, this, this, _mesh, _sys, _assembly, _tid, std::set<SubdomainID>{}, boundaryIDs()))
{
}

void
MassContinuityIPHDGBC::compute()
{
  _iphdg_helper->resizeResiduals();
  _iphdg_helper->scalarFace();
  _iphdg_helper->lmFace();
}
