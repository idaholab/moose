//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2BatchIndexGenerator.h"
#include "NEML2Utils.h"

registerMooseObject("MooseApp", NEML2BatchIndexGenerator);

InputParameters
NEML2BatchIndexGenerator::validParams()
{
  auto params = DomainUserObject::validParams();
  params.addClassDescription("Generates the element to batch index map for MOOSEToNEML2 gatherers, "
                             "NEML2ToMOOSE retrievers, and the NEML2 executor");

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

NEML2BatchIndexGenerator::NEML2BatchIndexGenerator(const InputParameters & params)
  : DomainUserObject(params), _outdated(true)
{
}

void
NEML2BatchIndexGenerator::meshChanged()
{
  _outdated = true;
}

void
NEML2BatchIndexGenerator::initialize()
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  if (!_outdated)
    return;

  _elem_to_batch_index.clear();
  _elem_to_batch_index_cache = {libMesh::invalid_uint, 0};
  _elemside_to_batch_index.clear();
  _elemside_to_batch_index_cache = {ElemSide(libMesh::invalid_uint, libMesh::invalid_uint), 0};
  _batch_index = 0;
}

void
NEML2BatchIndexGenerator::executeOnElement()
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  if (!_outdated)
    return;

  _elem_to_batch_index[_current_elem->id()] = _batch_index;
  _batch_index += qPoints().size();
}

void
NEML2BatchIndexGenerator::executeOnBoundary()
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  if (!_outdated)
    return;

  _elemside_to_batch_index[ElemSide(_current_elem->id(), _current_side)] = _batch_index;
  _batch_index += qPoints().size();
}

void
NEML2BatchIndexGenerator::threadJoin(const UserObject & uo)
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  if (!_outdated)
    return;

  const auto & m2n = static_cast<const NEML2BatchIndexGenerator &>(uo);

  // append and renumber maps
  for (const auto & [elem_id, batch_index] : m2n._elem_to_batch_index)
    _elem_to_batch_index[elem_id] = _batch_index + batch_index;

  for (const auto & [elem_side, batch_index] : m2n._elemside_to_batch_index)
    _elemside_to_batch_index[elem_side] = _batch_index + batch_index;

  _batch_index += m2n._batch_index;
}

void
NEML2BatchIndexGenerator::finalize()
{
  _outdated = false;
}

std::size_t
NEML2BatchIndexGenerator::getBatchIndex(dof_id_type elem_id) const
{
  // return cached map lookup if applicable
  if (_elem_to_batch_index_cache.first == elem_id)
    return _elem_to_batch_index_cache.second;

  // else, search the map
  const auto it = _elem_to_batch_index.find(elem_id);
  if (it == _elem_to_batch_index.end())
    mooseError("No batch index found for element id ", elem_id);
  _elem_to_batch_index_cache = *it;
  return it->second;
}

std::size_t
NEML2BatchIndexGenerator::getSideBatchIndex(const ElemSide & elem_side) const
{
  // return cached map lookup if applicable
  if (_elemside_to_batch_index_cache.first == elem_side)
    return _elemside_to_batch_index_cache.second;

  // else, search the map
  const auto it = _elemside_to_batch_index.find(elem_side);
  if (it == _elemside_to_batch_index.end())
    mooseError("No batch index found for element side (elem id, side) = (",
               std::get<0>(elem_side),
               ", ",
               std::get<1>(elem_side),
               ")");
  _elemside_to_batch_index_cache = *it;
  return it->second;
}
