//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ZeroBondStatusUserObjectPD.h"
#include "AuxiliarySystem.h"

registerMooseObject("PeridynamicsTestApp", ZeroBondStatusUserObjectPD);

InputParameters
ZeroBondStatusUserObjectPD::validParams()
{
  InputParameters params = GeneralUserObjectBasePD::validParams();
  params.addClassDescription("Class to assign value of zero to auxilary variable bond status for "
                             "user specified bonds list");

  params.addRequiredParam<std::vector<unsigned int>>(
      "bond_ids_list", "List of bond IDs to be assigned with value 0");

  params.set<ExecFlagEnum>("execute_on", true) = EXEC_INITIAL;

  return params;
}

ZeroBondStatusUserObjectPD::ZeroBondStatusUserObjectPD(const InputParameters & parameters)
  : GeneralUserObjectBasePD(parameters), _list(getParam<std::vector<unsigned int>>("bond_ids_list"))
{
}

void
ZeroBondStatusUserObjectPD::initialize()
{
  _aux.solution().close();
}

void
ZeroBondStatusUserObjectPD::execute()
{
  for (const auto & bid : _list)
  {
    const Elem * current_bond = _pdmesh.elemPtr(bid);
    if (current_bond->processor_id() == _tid)
    {
      dof_id_type dof = current_bond->dof_number(_aux.number(), _bond_status_var->number(), 0);
      _aux.solution().set(dof, 0); // set the status of bonds in the given list to 0
    }
  }
}

void
ZeroBondStatusUserObjectPD::finalize()
{
  _aux.solution().close();
}
