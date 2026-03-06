//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MOOSEToNEML2.h"
#include "DomainUserObject.h"

/**
 * @brief Generic gatherer for collecting "batched" MOOSE data for NEML2
 *
 * It is generic in the sense that it can be used for most MOOSE data types that take the form of
 * MooseArray<T>.
 *
 * It is not so generic in the sense that the collected data is always a std::vector of
 * MooseArray<T>, where the vector size is generally the number of elements this DomainUserObject
 * operates on, and the MooseArray<T> size is generally the number of quadrature points in each
 * element.
 *
 * @tparam T Type of the underlying MOOSE data, e.g., Real, SymmetricRankTwoTensor, etc.
 */
template <typename T>
class MOOSEToNEML2Batched : public MOOSEToNEML2, public DomainUserObject
{
public:
  static InputParameters validParams();

  MOOSEToNEML2Batched(const InputParameters & params);

  void finalize() override {}

#ifndef NEML2_ENABLED
  void initialize() override {}
  void executeOnElement() override {}
  void executeOnBoundary() override {}
  void executeOnInterface() override {}
  void threadJoin(const UserObject &) override {}
#else
  void initialize() override;
  void executeOnElement() override;
  void executeOnBoundary() override;
  void executeOnInterface() override;
  void threadJoin(const UserObject &) override;

  neml2::Tensor gatheredData() const override;

  // The number of batches
  std::size_t size() const { return _buffer.size(); }

protected:
  using ElemSide = std::tuple<dof_id_type, unsigned int>;

  /// MOOSE data for the current element
  virtual const MooseArray<T> & elemMOOSEData() const = 0;

  /// MOOSE data for the current element side
  virtual const MooseArray<T> & elemSideMOOSEData() const = 0;

  /// MOOSE data for the neighboring element side on internal/interface sides
  virtual const MooseArray<T> & elemNeighborSideMOOSEData() const = 0;

  /// Intermediate data buffer, filled during the element loop
  std::vector<T> _buffer;

  /// Element-side keys already gathered in this iteration
  std::set<ElemSide> _visited_elem_sides;
#endif
};

template <typename T>
InputParameters
MOOSEToNEML2Batched<T>::validParams()
{
  auto params = MOOSEToNEML2::validParams();
  params += DomainUserObject::validParams();

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

template <typename T>
MOOSEToNEML2Batched<T>::MOOSEToNEML2Batched(const InputParameters & params)
  : MOOSEToNEML2(params), DomainUserObject(params)
{
}

#ifdef NEML2_ENABLED
template <typename T>
void
MOOSEToNEML2Batched<T>::initialize()
{
  _buffer.clear();
  _visited_elem_sides.clear();
}

template <typename T>
void
MOOSEToNEML2Batched<T>::executeOnElement()
{
  const auto & elem_data = this->elemMOOSEData();
  for (auto i : index_range(elem_data))
    _buffer.push_back(elem_data[i]);
}

template <typename T>
void
MOOSEToNEML2Batched<T>::executeOnBoundary()
{
  if (_current_elem->neighbor_ptr(_current_side))
    return;

  const auto elem_side = ElemSide(_current_elem->id(), _current_side);
  if (_visited_elem_sides.insert(elem_side).second)
  {
    const auto & elem_data = this->elemSideMOOSEData();
    for (auto i : index_range(elem_data))
      _buffer.push_back(elem_data[i]);
  }
}

template <typename T>
void
MOOSEToNEML2Batched<T>::executeOnInterface()
{
  const auto elem_side = ElemSide(_current_elem->id(), _current_side);
  if (_visited_elem_sides.insert(elem_side).second)
  {
    const auto & elem_data = this->elemSideMOOSEData();
    for (auto i : index_range(elem_data))
      _buffer.push_back(elem_data[i]);
  }

  const auto * neighbor_elem = _current_elem->neighbor_ptr(_current_side);

  if (neighbor_elem)
  {
    const auto neighbor_side = neighbor_elem->which_neighbor_am_i(_current_elem);
    const auto neighbor_elem_side = ElemSide(neighbor_elem->id(), neighbor_side);
    if (_visited_elem_sides.insert(neighbor_elem_side).second)
    {
      const auto & neighbor_elem_data = this->elemNeighborSideMOOSEData();
      for (auto i : index_range(neighbor_elem_data))
        _buffer.push_back(neighbor_elem_data[i]);
    }
  }
}

template <typename T>
void
MOOSEToNEML2Batched<T>::threadJoin(const UserObject & uo)
{
  // append vectors
  const auto & m2n = static_cast<const MOOSEToNEML2Batched<T> &>(uo);
  _buffer.insert(_buffer.end(), m2n._buffer.begin(), m2n._buffer.end());
  _visited_elem_sides.insert(m2n._visited_elem_sides.begin(), m2n._visited_elem_sides.end());
}

template <typename T>
neml2::Tensor
MOOSEToNEML2Batched<T>::gatheredData() const
{
  return NEML2Utils::fromBlob(_buffer);
}
#endif
