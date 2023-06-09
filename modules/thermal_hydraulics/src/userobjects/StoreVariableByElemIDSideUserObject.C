//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StoreVariableByElemIDSideUserObject.h"
#include "THMUtils.h"

registerMooseObject("ThermalHydraulicsApp", StoreVariableByElemIDSideUserObject);

InputParameters
StoreVariableByElemIDSideUserObject::validParams()
{
  InputParameters params = SideUserObject::validParams();

  params.addRequiredCoupledVar("variable", "Variable to store");
  params.addClassDescription(
      "Stores variable values at each quadrature point on a side by element ID.");

  return params;
}

StoreVariableByElemIDSideUserObject::StoreVariableByElemIDSideUserObject(
    const InputParameters & parameters)
  : SideUserObject(parameters),

    _u(adCoupledValue("variable"))
{
}

void
StoreVariableByElemIDSideUserObject::initialize()
{
  _elem_id_to_var_values.clear();
}

void
StoreVariableByElemIDSideUserObject::execute()
{
  unsigned int n_qp = _qrule->n_points();
  const auto elem_id = _current_elem->id();

  _elem_id_to_var_values[elem_id].resize(n_qp);
  for (unsigned int qp = 0; qp < n_qp; qp++)
    _elem_id_to_var_values[elem_id][qp] = _u[qp];
}

void
StoreVariableByElemIDSideUserObject::threadJoin(const UserObject & uo)
{
  const auto & other_uo = static_cast<const StoreVariableByElemIDSideUserObject &>(uo);
  const auto other_map = other_uo._elem_id_to_var_values;
  for (auto & it : other_map)
    _elem_id_to_var_values[it.first] = it.second;
}

void
StoreVariableByElemIDSideUserObject::finalize()
{
  THM::allGatherADVectorMap(comm(), _elem_id_to_var_values);
}

const std::vector<ADReal> &
StoreVariableByElemIDSideUserObject::getVariableValues(dof_id_type elem_id) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  auto it = _elem_id_to_var_values.find(elem_id);
  if (it != _elem_id_to_var_values.end())
    return it->second;
  else
    mooseError(
        name(), ": The variable values for element ", elem_id, " were requested but not stored.");
}
