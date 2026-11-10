//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2BatchIndexGenerator.h"

registerMooseObject("MooseApp", NEML2BatchIndexGenerator);

InputParameters
NEML2BatchIndexGenerator::validParams()
{
  auto params = DomainUserObject::validParams();
  params.addClassDescription("Generates the element to batch index map for MOOSEToNEML2 gatherers, "
                             "NEML2ToMOOSE retrievers, and the NEML2 executor");

  params.addParam<bool>(
      "volume", true, "Whether element-interior quadrature points contribute to the NEML2 batch");

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

NEML2BatchIndexGenerator::NEML2BatchIndexGenerator(const InputParameters & params)
  : DomainUserObject(params), _volume(getParam<bool>("volume")), _outdated(true)
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
  if (!_outdated)
    return;

  _elem_to_batch_index.clear();
  _elemside_to_batch_index.clear();
  _batch_index = 0;
}

void
NEML2BatchIndexGenerator::executeOnElement()
{
  if (!_outdated)
    return;

  if (!_volume)
    return;

  const auto n_qp = qPoints().size();
  _elem_to_batch_index[_current_elem->id()] = {_batch_index, n_qp};
  _batch_index += n_qp;
}

void
NEML2BatchIndexGenerator::executeOnBoundary()
{
  if (!_outdated)
    return;

  const auto elem_side = ElemSide(_current_elem->id(), _current_side);
  const auto n_qp = qPoints().size();
  const auto [it, inserted] =
      _elemside_to_batch_index.emplace(elem_side, BatchSpan{_batch_index, n_qp});
  if (inserted)
    _batch_index += n_qp;
}

void
NEML2BatchIndexGenerator::executeOnInterface()
{
  if (!_outdated)
    return;

  const auto * neighbor_elem = _current_elem->neighbor_ptr(_current_side);
  const auto n_qp = qPoints().size();
  const auto elem_side = ElemSide(_current_elem->id(), _current_side);
  const auto [it, inserted] =
      _elemside_to_batch_index.emplace(elem_side, BatchSpan{_batch_index, n_qp});
  if (inserted)
    _batch_index += n_qp;

  const auto neighbor_side = neighbor_elem->which_neighbor_am_i(_current_elem);
  const auto neighbor_elem_side = ElemSide(neighbor_elem->id(), neighbor_side);
  const auto [neighbor_it, neighbor_inserted] =
      _elemside_to_batch_index.emplace(neighbor_elem_side, BatchSpan{_batch_index, n_qp});
  if (neighbor_inserted)
    _batch_index += n_qp;
}

void
NEML2BatchIndexGenerator::threadJoin(const UserObject & uo)
{
  if (!_outdated)
    return;

  const auto & m2n = cast_ref<const NEML2BatchIndexGenerator &>(uo);

  // append and renumber maps
  for (const auto & [elem_id, span] : m2n._elem_to_batch_index)
    _elem_to_batch_index[elem_id] = {_batch_index + span.begin, span.size};

  for (const auto & [elem_side, span] : m2n._elemside_to_batch_index)
    _elemside_to_batch_index[elem_side] = {_batch_index + span.begin, span.size};

  _batch_index += m2n._batch_index;
}

void
NEML2BatchIndexGenerator::finalize()
{
  _outdated = false;
}

bool
NEML2BatchIndexGenerator::hasBatchSpan(dof_id_type elem_id) const
{
  return _elem_to_batch_index.count(elem_id);
}

bool
NEML2BatchIndexGenerator::hasSideBatchSpan(const ElemSide & elem_side) const
{
  return _elemside_to_batch_index.count(elem_side);
}

NEML2BatchIndexGenerator::BatchSpan
NEML2BatchIndexGenerator::getBatchSpan(dof_id_type elem_id) const
{
  const auto it = _elem_to_batch_index.find(elem_id);
  if (it == _elem_to_batch_index.end())
    mooseError("No volume batch span found for element id ", elem_id);
  return it->second;
}

NEML2BatchIndexGenerator::BatchSpan
NEML2BatchIndexGenerator::getSideBatchSpan(const ElemSide & elem_side) const
{
  const auto it = _elemside_to_batch_index.find(elem_side);
  if (it == _elemside_to_batch_index.end())
    mooseError("No side batch span found for element side (elem id, side) = (",
               std::get<0>(elem_side),
               ", ",
               std::get<1>(elem_side),
               "). Configure this external side with NEML2 'boundary' or this internal side with "
               "NEML2 'interface'. Volume quadrature-point data cannot be used for side "
               "quadrature points.");
  return it->second;
}
