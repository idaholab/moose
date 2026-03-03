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
registerMooseObject("MooseApp", NEML2BoundaryBatchIndexGenerator);

template <class Base>
InputParameters
NEML2BatchIndexGeneratorTmpl<Base>::validParams()
{
  auto params = Base::validParams();
  params.addClassDescription("Generates the element to batch index map for MOOSEToNEML2 gatherers, "
                             "NEML2ToMOOSE retrievers, and the NEML2 executor");

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.template set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

template <class Base>
NEML2BatchIndexGeneratorTmpl<Base>::NEML2BatchIndexGeneratorTmpl(const InputParameters & params)
  : Base(params), _outdated(true)
{
}

template <class Base>
void
NEML2BatchIndexGeneratorTmpl<Base>::meshChanged()
{
  _outdated = true;
}

template <class Base>
void
NEML2BatchIndexGeneratorTmpl<Base>::initialize()
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  if (!_outdated)
    return;

  _elem_to_batch_index.clear();

  if constexpr (std::is_same_v<Base, ElementUserObject>)
    _elem_to_batch_index_cache = {libMesh::invalid_uint, 0};
  else
    _elem_to_batch_index_cache = {{libMesh::invalid_uint, 0}, 0};

  _batch_index = 0;
}

template <class Base>
void
NEML2BatchIndexGeneratorTmpl<Base>::execute()
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  if (!_outdated)
    return;

  _elem_to_batch_index[currentIndex()] = _batch_index;
  _batch_index += _qrule->n_points();
}

template <class Base>
void
NEML2BatchIndexGeneratorTmpl<Base>::threadJoin(const UserObject & uo)
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  if (!_outdated)
    return;

  const auto & m2n = static_cast<const NEML2BatchIndexGeneratorTmpl<Base> &>(uo);

  // append and renumber maps
  for (const auto & [key, batch_index] : m2n._elem_to_batch_index)
    _elem_to_batch_index[key] = _batch_index + batch_index;

  _batch_index += m2n._batch_index;
}

template <class Base>
void
NEML2BatchIndexGeneratorTmpl<Base>::finalize()
{
  _outdated = false;
}

template <>
BatchIndexKey<ElementUserObject>
NEML2BatchIndexGeneratorTmpl<ElementUserObject>::currentIndex()
{
  return _current_elem->id();
}

template <>
BatchIndexKey<SideUserObject>
NEML2BatchIndexGeneratorTmpl<SideUserObject>::currentIndex()
{
  return {_current_elem->id(), _current_side};
}

template <class Base>
template <typename B>
std::enable_if_t<std::is_same_v<B, ElementUserObject>, std::size_t>
NEML2BatchIndexGeneratorTmpl<Base>::getBatchIndex(dof_id_type elem_id) const
{
  return getBatchIndexImpl(elem_id);
}

template <class Base>
template <typename B>
std::enable_if_t<std::is_same_v<B, SideUserObject>, std::size_t>
NEML2BatchIndexGeneratorTmpl<Base>::getBatchIndex(dof_id_type elem_id, unsigned int side) const
{
  return getBatchIndexImpl({elem_id, side});
}

template <class Base>
std::size_t
NEML2BatchIndexGeneratorTmpl<Base>::getBatchIndexImpl(const BatchIndexKey<Base> & idx) const
{
  // return cached map lookup if applicable
  if (_elem_to_batch_index_cache.first == idx)
    return _elem_to_batch_index_cache.second;

  // else, search the map
  const auto it = _elem_to_batch_index.find(idx);
  if (it == _elem_to_batch_index.end())
  {
    if constexpr (std::is_same_v<Base, ElementUserObject>)
      mooseError("No batch index found for element id ", idx);
    else
      mooseError("No batch index found for element id ", idx.first, " and side ", idx.second);
  }
  _elem_to_batch_index_cache = *it;
  return it->second;
}

template class NEML2BatchIndexGeneratorTmpl<ElementUserObject>;
template class NEML2BatchIndexGeneratorTmpl<SideUserObject>;
