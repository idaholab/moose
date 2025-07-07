//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addParam<std::vector<MaterialPropertyName>>(
      "mat_prop", "The names of material properties that this VectorPostprocessor operates on");
  params.addRequiredParam<std::vector<ExtraElementIDName>>(
      "id_name", "List of extra element ID names by which to separate integral(s).");
  params.addParam<bool>("average", false, "Whether or not to compute volume average");
  params.addParam<std::string>("spatial_value_name",
                               "To specify which variable or material property is to be used when "
                               "using as a spatial user object functor");
  params.addClassDescription("Integrates or averages variables based on extra element IDs");
  params.makeParamNotRequired("variable");
  return params;
}

ExtraIDIntegralVectorPostprocessor::ExtraIDIntegralVectorPostprocessor(
    const InputParameters & parameters)
  : SpatialUserObjectFunctor<ElementVariableVectorPostprocessor>(parameters),
    _average(getParam<bool>("average")),
    _nvar(isParamValid("variable") ? coupledComponents("variable") : 0),
    _nprop(isParamValid("mat_prop") ? getParam<std::vector<MaterialPropertyName>>("mat_prop").size()
                                    : 0),
    _prop_names(isParamValid("mat_prop") ? getParam<std::vector<MaterialPropertyName>>("mat_prop")
                                         : std::vector<MaterialPropertyName>()),
    _extra_id(getParam<std::vector<ExtraElementIDName>>("id_name")),
    _n_extra_id(_extra_id.size()),
    _spatial_evaluation_index(std::numeric_limits<unsigned int>::max())
{
  if (!_nvar && !_nprop)
    mooseError("Neither 'variable' nor 'mat_prop' was specified.");

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
    _extra_ids.push_back(&p);
  }

  std::vector<std::string> vector_names;

  // declare vectors containing variable integral values
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    _vars.push_back(getVar("variable", i));
    _var_values.push_back(&coupledValue("variable", i));
    vector_names.push_back(_vars[i]->name());
    auto & p = declareVector(vector_names.back());
    p.resize((*_extra_ids[0]).size());
    _integrals.push_back(&p);
  }

  // declare vectors containing material property integral values
  for (auto & name : _prop_names)
  {
    _props.push_back(&getMaterialPropertyByName<Real>(name));
    vector_names.push_back(name);
    auto & p = declareVector(vector_names.back());
    p.resize((*_extra_ids[0]).size());
    _integrals.push_back(&p);
  }

  if (isParamValid("spatial_value_name"))
  {
    const auto & name = getParam<std::string>("spatial_value_name");
    _spatial_evaluation_index = vector_names.size();
    for (const auto i : index_range(vector_names))
      if (name == vector_names[i])
      {
        _spatial_evaluation_index = i;
        break;
      }
    if (_spatial_evaluation_index == vector_names.size())
      paramError("spatial_value_name",
                 "not a variable name or material property name of this vector postprocessor");
  }
}

void
ExtraIDIntegralVectorPostprocessor::initialize()
{
  for (auto & integral : _integrals)
    std::fill(integral->begin(), integral->end(), 0);

  if (_average)
    _volumes.assign((*_extra_ids[0]).size(), 0);
}

void
ExtraIDIntegralVectorPostprocessor::execute()
{
  if (hasBlocks(_current_elem->subdomain_id()))
  {
    unsigned int i = 0;
    auto ipos = _unique_vpp_ids[_current_elem->id()];
    for (unsigned int ivar = 0; ivar < _nvar; ++ivar, ++i)
      if (_vars[ivar]->hasBlocks(_current_elem->subdomain_id()))
        for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
          (*_integrals[i])[ipos] += _JxW[qp] * _coord[qp] * (*_var_values[ivar])[qp];

    for (unsigned int iprop = 0; iprop < _nprop; ++iprop, ++i)
      for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
        (*_integrals[i])[ipos] += _JxW[qp] * _coord[qp] * (*_props[iprop])[qp];

    if (_average)
      _volumes[ipos] += _current_elem->volume();
  }
}

void
ExtraIDIntegralVectorPostprocessor::finalize()
{
  for (auto & integral : _integrals)
    gatherSum(*integral);

  if (_average)
  {
    gatherSum(_volumes);

    for (auto & integral : _integrals)
      for (unsigned int i = 0; i < integral->size(); ++i)
        (*integral)[i] /= _volumes[i];
  }
}

void
ExtraIDIntegralVectorPostprocessor::threadJoin(const UserObject & s)
{
  const auto & sibling = static_cast<const ExtraIDIntegralVectorPostprocessor &>(s);

  for (unsigned int i = 0; i < _integrals.size(); ++i)
    for (size_t j = 0; j < (*_integrals[i]).size(); ++j)
      (*_integrals[i])[j] += (*sibling._integrals[i])[j];

  if (_average)
    for (unsigned int i = 0; i < _volumes.size(); ++i)
      _volumes[i] += sibling._volumes[i];
}

Real
ExtraIDIntegralVectorPostprocessor::evaluate(const ElemArg & elem,
                                             const Moose::StateArg & libmesh_dbg_var(state)) const
{
  mooseAssert(state.state == 0, "We do not currently support evaluating at old states");
  return elementValue(elem.elem);
}

Real
ExtraIDIntegralVectorPostprocessor::evaluate(const ElemQpArg & elem,
                                             const Moose::StateArg & libmesh_dbg_var(state)) const
{
  mooseAssert(state.state == 0, "We do not currently support evaluating at old states");
  return elementValue(elem.elem);
}

Real
ExtraIDIntegralVectorPostprocessor::elementValue(const Elem * elem) const
{
  if (_spatial_evaluation_index == std::numeric_limits<unsigned int>::max())
    paramError("spatial_value_name",
               "Must set when ExtraIDIntegralVectorPostprocessor is used as a functor");
  const auto it = _unique_vpp_ids.find(elem->id());
  if (it == _unique_vpp_ids.end())
  {
    if (!hasBlocks(elem->subdomain_id()))
      mooseError(
          "Failed evaluating spatial value of element ",
          elem->id(),
          " in subdomain ",
          elem->subdomain_id(),
          ". This is likely due to the object using this "
          "ExtraIDIntegralVectorPostprocessor as a functor operates on a subdomain outside of "
          "this ExtraIDIntegralVectorPostprocessor");
    else
      mooseError("Failed evaluating spatial value of element ",
                 elem->id(),
                 ". We should not hit this error. Please contact code developers for help");
  }
  return (*_integrals[_spatial_evaluation_index])[it->second];
}
