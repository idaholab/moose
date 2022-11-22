//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExtraIDIntegralVectorPostprocessor.h"
#include "MooseVariable.h"
#include "MooseMesh.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_base.h"

registerMooseObject("MooseApp", ExtraIDIntegralVectorPostprocessor);

InputParameters
ExtraIDIntegralVectorPostprocessor::validParams()
{
  InputParameters params = ElementVariableVectorPostprocessor::validParams();
  params.addRequiredParam<std::vector<ExtraElementIDName>>(
      "id_name", "List of extra element ID names by which to separate integral(s).");
  params.addClassDescription("Integrates variables based on extra element IDs");
  return params;
}

ExtraIDIntegralVectorPostprocessor::ExtraIDIntegralVectorPostprocessor(
    const InputParameters & parameters)
  : ElementVariableVectorPostprocessor(parameters),
    _nvar(coupledComponents("variable")),
    _extra_id(getParam<std::vector<ExtraElementIDName>>("id_name")),
    _n_extra_id(_extra_id.size())
{
  // create map of element ids to parsed vpp ids
  _unique_vpp_ids =
      MooseMeshUtils::getExtraIDUniqueCombinationMap(_mesh.getMesh(), blockIDs(), _extra_id);

  // set up variable vector containing parsed extra ids
  for (unsigned int i = 0; i < _n_extra_id; ++i)
  {
    const auto id_type = _extra_id[i];
    auto & p = declareVector("Level-" + std::to_string(i) + "-" + id_type);
    // collect local map of local vpp id to extra id
    std::map<dof_id_type, dof_id_type> extra_ids;
    for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
    {
      if (!hasBlocks(elem->subdomain_id()))
        continue;
      auto vpp_id = _unique_vpp_ids[elem->id()];
      if (extra_ids.find(vpp_id) == extra_ids.end())
        extra_ids[vpp_id] = elem->get_extra_integer(getElementIDIndexByName(id_type));
    }
    // create global map of local vpp id to extra id
    comm().set_union(extra_ids);
    p.resize(extra_ids.size());
    for (auto it : extra_ids)
      p[it.first] = it.second;
    _var_extra_ids.push_back(&p);
  }

  // declare vectors containing integral values
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    _vars.push_back(getVar("variable", i));
    _var_values.push_back(&coupledValue("variable", i));
    auto & p = declareVector(_vars[i]->name());
    p.resize((*_var_extra_ids[0]).size());
    _var_integrals.push_back(&p);
  }
}

void
ExtraIDIntegralVectorPostprocessor::initialize()
{
  for (auto & var_integral : _var_integrals)
    std::fill(var_integral->begin(), var_integral->end(), 0);
}

void
ExtraIDIntegralVectorPostprocessor::execute()
{
  if (hasBlocks(_current_elem->subdomain_id()))
  {
    auto ipos = _unique_vpp_ids[_current_elem->id()];
    for (unsigned int ivar = 0; ivar < _nvar; ++ivar)
      if (_vars[ivar]->hasBlocks(_current_elem->subdomain_id()))
        for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
          (*_var_integrals[ivar])[ipos] += _JxW[qp] * _coord[qp] * (*_var_values[ivar])[qp];
  }
}

void
ExtraIDIntegralVectorPostprocessor::finalize()
{
  for (auto & var_integral : _var_integrals)
    gatherSum(*var_integral);
}

void
ExtraIDIntegralVectorPostprocessor::threadJoin(const UserObject & s)
{
  const ExtraIDIntegralVectorPostprocessor & sibling =
      static_cast<const ExtraIDIntegralVectorPostprocessor &>(s);

  for (unsigned int ivar = 0; ivar < _nvar; ++ivar)
    for (size_t i = 0; i < (*_var_integrals[ivar]).size(); ++i)
      (*_var_integrals[ivar])[i] += (*sibling._var_integrals[ivar])[i];
}
