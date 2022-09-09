//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <utility>

template <class T>
class ExtremeValueBase : public T
{
public:
  static InputParameters validParams();

  ExtremeValueBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual Real getValue() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual std::pair<Real, Real> getProxyValuePair() = 0;

  /// Get the extreme value with a functor element argument
  virtual void computeExtremeValue();

  /// Type of extreme value we are going to compute
  enum class ExtremeType
  {
    MAX,
    MIN
  } _type;

  /// Extreme value and proxy value at the same point
  std::pair<Real, Real> _proxy_value;

  /// use proxy value. Requires more expensive MPI communication
  bool _use_proxy;
};
