//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_common.h"
#include <utility>

class UserObject;
class InputParameters;

template <class T>
class ExtremeValueBase : public T
{
public:
  static InputParameters validParams();

  ExtremeValueBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual libMesh::Real getValue() const override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual std::pair<libMesh::Real, libMesh::Real> getProxyValuePair() = 0;

  /// Get the extreme value with a functor element argument
  virtual void computeExtremeValue();

  /// Type of extreme value we are going to compute
  enum class ExtremeType
  {
    MAX,
    MIN,
    MAX_ABS
  } _type;

  /// Extreme value and proxy value at the same point
  std::pair<libMesh::Real, libMesh::Real> _proxy_value;
};
