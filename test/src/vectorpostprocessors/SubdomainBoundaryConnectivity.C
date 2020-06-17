//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainBoundaryConnectivity.h"

registerMooseObject("MooseTestApp", SubdomainBoundaryConnectivity);

InputParameters
SubdomainBoundaryConnectivity::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addParam<bool>(
      "interface_boundary", false, "True to include primary and neighbor subdomains.");
  params.addParam<BoundaryName>("boundary", "Boundary to find connected subdomains.");
  params.addParam<SubdomainName>("block", "Subdomain to find connected boundaries.");
  return params;
}

SubdomainBoundaryConnectivity::SubdomainBoundaryConnectivity(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _connected_ids(isParamValid("block") ? declareVector("connected_boundary_ids")
                                         : declareVector("connected_subdomain_ids"))
{
  if (isParamValid("block") && isParamValid("boundary"))
    paramError("block", "Cannon specify both boundary and block names.");
  else if (!isParamValid("block") && !isParamValid("boundary"))
    paramError("boundary", "Must specify either boundary or block name.");
}

void
SubdomainBoundaryConnectivity::execute()
{
  if (isParamValid("boundary"))
  {
    BoundaryID bnd_id = _mesh.getBoundaryID(getParam<BoundaryName>("boundary"));
    std::set<SubdomainID> blk_ids;
    if (getParam<bool>("interface_boundary"))
      blk_ids = _mesh.getInterfaceConnectedBlocks(bnd_id);
    else
      blk_ids = _mesh.getBoundaryConnectedBlocks(bnd_id);
    for (const auto & it : blk_ids)
      _connected_ids.push_back(it);
  }
  else if (isParamValid("block"))
  {
    SubdomainID blk_id = _mesh.getSubdomainID(getParam<SubdomainName>("block"));
    if (getParam<bool>("interface_boundary"))
    {
      std::set<BoundaryID> bnd_ids = _mesh.getSubdomainInterfaceBoundaryIds(blk_id);
      for (const auto & it : bnd_ids)
        _connected_ids.push_back(it);
    }
    else
    {
      const std::set<BoundaryID> & bnd_ids = _mesh.getSubdomainBoundaryIds(blk_id);
      for (const auto & it : bnd_ids)
        _connected_ids.push_back(it);
    }
  }
}
