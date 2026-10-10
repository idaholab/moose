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
#include "NEML2Utils.h"
#include "ElementUserObject.h"

#include "RankTwoTensor.h"
#include "SymmetricRankTwoTensor.h"

/**
 * Gather a MOOSE quantity for insertion into the NEML2 model.
 */
template <typename T, unsigned int state>
class MOOSEQuantityToNEML2 : public MOOSEToNEML2, public ElementUserObject
{
public:
  static InputParameters validParams();

  MOOSEQuantityToNEML2(const InputParameters & params);

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

  at::Tensor gatheredData() const override;

protected:
  T qpData(unsigned int) const;

  /// MOOSE quantity type to read from
  const NEML2Utils::MOOSEIOType _type;

  ///@{
  /// candidate MOOSE quantities to read data from
  const VariableValue * _var_scalar = nullptr;
  const VariableValue * _var_scalar_old = nullptr;
  const Function * _func = nullptr;
  const MaterialProperty<T> * _mat_prop = nullptr;
  const MaterialProperty<T> * _mat_prop_old = nullptr;
  const VariableValue * _var = nullptr;
  const VariableValue * _var_old = nullptr;
  ///@}

  /// Whether the gathered data should be batched
  bool _batched = false;

  /// Intermediate data buffer, filled during the element loop
  std::vector<T> _buffer;
#endif
};

#define defineMOOSEQuantityToNEML2(T)                                                              \
  using MOOSE##T##ToNEML2 = MOOSEQuantityToNEML2<T, 0>;                                            \
  using MOOSEOld##T##ToNEML2 = MOOSEQuantityToNEML2<T, 1>
defineMOOSEQuantityToNEML2(Real);
defineMOOSEQuantityToNEML2(RankTwoTensor);
defineMOOSEQuantityToNEML2(SymmetricRankTwoTensor);
defineMOOSEQuantityToNEML2(RealVectorValue);
#undef defineMOOSEQuantityToNEML2
