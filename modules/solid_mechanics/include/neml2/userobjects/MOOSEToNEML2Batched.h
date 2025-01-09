//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MOOSEToNEML2.h"
#include "ElementUserObject.h"

/**
 * @brief Generic gatherer for collecting "batched" MOOSE data for NEML2
 *
 * It is generic in the sense that it can be used for most MOOSE data types that take the form of
 * MooseArray<T>.
 *
 * It is not so generic in the sense that the collected data is always a std::vector of
 * MooseArray<T>, where the vector size is generally the number of elements this ElementUserObject
 * operates on, and the MooseArray<T> size is generally the number of quadrature points in each
 * element.
 *
 * @tparam T Type of the underlying MOOSE data, e.g., Real, SymmetricRankTwoTensor, etc.
 */
template <typename T>
class MOOSEToNEML2Batched : public MOOSEToNEML2, public ElementUserObject
{
public:
  static InputParameters validParams();

  MOOSEToNEML2Batched(const InputParameters & params);

#ifndef NEML2_ENABLED
  void initialize() override {}
  void execute() override {}
  void finalize() override {}
  void threadJoin(const UserObject &) override {}
#else
  void initialize() override;
  void execute() override;
  void finalize() override {}
  void threadJoin(const UserObject &) override;

  neml2::Tensor gatheredData() const override;

  // The number of batches
  std::size_t size() const { return _buffer.size(); }

protected:
  /// MOOSE data for the current element
  virtual const MooseArray<T> & elemMOOSEData() const = 0;

  /// Intermediate data buffer, filled during the element loop
  std::vector<T> _buffer;
#endif
};

template <typename T>
InputParameters
MOOSEToNEML2Batched<T>::validParams()
{
  auto params = MOOSEToNEML2::validParams();
  params += ElementUserObject::validParams();

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
  : MOOSEToNEML2(params), ElementUserObject(params)
{
}

#ifdef NEML2_ENABLED
template <typename T>
void
MOOSEToNEML2Batched<T>::initialize()
{
  _buffer.clear();
}

template <typename T>
void
MOOSEToNEML2Batched<T>::execute()
{
  const auto & elem_data = this->elemMOOSEData();
  for (auto i : index_range(elem_data))
    _buffer.push_back(elem_data[i]);
}

template <typename T>
void
MOOSEToNEML2Batched<T>::threadJoin(const UserObject & uo)
{
  // append vectors
  const auto & m2n = static_cast<const MOOSEToNEML2Batched<T> &>(uo);
  _buffer.insert(_buffer.end(), m2n._buffer.begin(), m2n._buffer.end());
}

template <typename T>
neml2::Tensor
MOOSEToNEML2Batched<T>::gatheredData() const
{
  return NEML2Utils::fromBlob(_buffer);
}
#endif
