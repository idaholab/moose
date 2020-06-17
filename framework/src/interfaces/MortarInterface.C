//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE Includes
#include "MortarInterface.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MortarData.h"

#include <algorithm>

InputParameters
MortarInterface::validParams()
{
  // Create InputParameters object that will be appended to the parameters for the inheriting object
  InputParameters params = emptyInputParameters();

  params.addRequiredParam<BoundaryName>("primary_boundary",
                                        "The name of the primary boundary sideset.");
  params.addRequiredParam<BoundaryName>("secondary_boundary",
                                        "The name of the secondary boundary sideset.");
  params.addRequiredParam<SubdomainName>("primary_subdomain", "The name of the primary subdomain.");
  params.addRequiredParam<SubdomainName>("secondary_subdomain", "The name of the secondary subdomain.");
  params.addParam<bool>(
      "periodic",
      false,
      "Whether this constraint is going to be used to enforce a periodic condition. This has the "
      "effect of changing the normals vector for projection from outward to inward facing");

  return params;
}

// Standard constructor
MortarInterface::MortarInterface(const MooseObject * moose_object)
  : _moi_problem(*moose_object->getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _moi_mesh(_moi_problem.mesh()),
    _mortar_data(_moi_problem.mortarData()),
    _secondary_id(_moi_mesh.getBoundaryID(moose_object->getParam<BoundaryName>("secondary_boundary"))),
    _primary_id(_moi_mesh.getBoundaryID(moose_object->getParam<BoundaryName>("primary_boundary"))),
    _secondary_subdomain_id(
        _moi_mesh.getSubdomainID(moose_object->getParam<SubdomainName>("secondary_subdomain"))),
    _primary_subdomain_id(
        _moi_mesh.getSubdomainID(moose_object->getParam<SubdomainName>("primary_subdomain")))
{
  if (_moi_mesh.dimension() == 3)
    mooseError("Mortar cannot currently be run in three dimensions. It's on the to-do list!");

  // Create the mortar interface if it hasn't already been created
  _moi_problem.createMortarInterface(std::make_pair(_primary_id, _secondary_id),
                                     std::make_pair(_primary_subdomain_id, _secondary_subdomain_id),
                                     moose_object->isParamValid("use_displaced_mesh")
                                         ? moose_object->getParam<bool>("use_displaced_mesh")
                                         : false,
                                     moose_object->getParam<bool>("periodic"));

  const auto & secondary_set = _mortar_data.getHigherDimSubdomainIDs(_secondary_subdomain_id);
  const auto & primary_set = _mortar_data.getHigherDimSubdomainIDs(_primary_subdomain_id);

  std::set_union(secondary_set.begin(),
                 secondary_set.end(),
                 primary_set.begin(),
                 primary_set.end(),
                 std::inserter(_higher_dim_subdomain_ids, _higher_dim_subdomain_ids.begin()));
  _boundary_ids = {_secondary_id, _primary_id};
}
