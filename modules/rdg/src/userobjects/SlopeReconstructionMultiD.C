//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SlopeReconstructionMultiD.h"

InputParameters
SlopeReconstructionMultiD::validParams()
{
  InputParameters params = SlopeReconstructionBase::validParams();

  params.addRequiredParam<std::vector<BoundaryName>>("boundary_list", "List of boundary IDs");

  params.addRequiredParam<std::vector<UserObjectName>>(
      "boundary_condition_user_object_list", "List of boundary condition user object names");

  return params;
}

SlopeReconstructionMultiD::SlopeReconstructionMultiD(const InputParameters & parameters)
  : SlopeReconstructionBase(parameters)
{
  const std::vector<BoundaryName> & bnd_name = getParam<std::vector<BoundaryName>>("boundary_list");

  const std::vector<UserObjectName> & bc_uo_name =
      getParam<std::vector<UserObjectName>>("boundary_condition_user_object_list");

  if (bnd_name.size() != bc_uo_name.size())
    mooseError("Number of boundaries NOT equal to number of BCUserObject names:",
               "\nNumber of boundaries is ",
               bnd_name.size(),
               "\nNumber of BCUserObject is ",
               bc_uo_name.size());

  for (unsigned int i = 0; i < bnd_name.size(); i++)
  {
    BoundaryID bnd_id = _mesh.getBoundaryID(bnd_name[i]);

    _bnd_uo_name_map.insert(std::pair<BoundaryID, UserObjectName>(bnd_id, bc_uo_name[i]));
  }
}
