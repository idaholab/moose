//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Standard includes
#include <sstream>
#include <stdexcept>

// MOOSE includes
#include "AddBlockVariableInfoAction.h"
#include "AddVariableAction.h"
#include "SetupMeshAction.h"

#include "FEProblem.h"
// #include "Factory.h"
#include "MooseEnum.h"
#include "MooseMesh.h"

#include "libmesh/string_to_enum.h"
#include "libmesh/fe_interface.h"

registerMooseAction("MooseApp", AddBlockVariableInfoAction, "add_block_variable_info");

template <>
InputParameters
validParams<AddBlockVariableInfoAction>()
{
  // Get MooseEnums for the possible order/family options for this variable
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());

  // Define the general input options
  InputParameters params = validParams<Action>();
  params.addParam<MooseEnum>(
      "family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE shape function to use "
                             "for this variable (additional orders not listed are "
                             "allowed)");
  params.addParam<std::vector<SubdomainName>>("block", "The block id where this variable lives");
  return params;
}

AddBlockVariableInfoAction::AddBlockVariableInfoAction(InputParameters params)
  : Action(params),
    _fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
             Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))),
    _scalar_var(_fe_type.family == SCALAR)
{
  _all_block_ids.clear();
}

void
AddBlockVariableInfoAction::act()
{
  if (_current_task == "add_block_variable_info")
  {
    std::set<SubdomainID> blocks = getSubdomainIDs();

    // Scalar variable
    if (_scalar_var)
      return;

    for (auto & block : blocks)
    {
      unsigned int ndof = 0;
      // find the element type, look for the first element of the right block
      for (const auto & elem : _mesh->getMesh().active_local_element_ptr_range())
        if (elem->subdomain_id() == block)
          ndof = FEInterface::n_dofs(_mesh->getMesh().mesh_dimension(),
                                     _fe_type, elem->type());
      std::cout << name() << " " << block << " ndof " << ndof << std::endl;
      _app.addBlockDoFs(block, ndof);
    }
  }
}

std::set<SubdomainID>
AddBlockVariableInfoAction::getSubdomainIDs()
{
  if (!isParamValid("block"))
  {
    if (_all_block_ids.size() == 0)
      _mesh->getMesh().subdomain_ids(_all_block_ids);

    return _all_block_ids;
  }

  std::set<SubdomainID> blocks;
  for (const auto & subdomain_name : getParam<std::vector<SubdomainName>>("block"))
    blocks.insert(_mesh->getSubdomainID(subdomain_name));
  return blocks;
}
