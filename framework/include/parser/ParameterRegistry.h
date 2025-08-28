//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <functional>
#include <map>

#include "libmesh/parameters.h"

#include "MooseError.h"

#ifdef MOOSE_UNIT_TEST
#include "gtest/gtest.h"
class GTEST_TEST_CLASS_NAME_(ParameterRegistryTest, add);
class GTEST_TEST_CLASS_NAME_(ParameterRegistryTest, addExists);
class GTEST_TEST_CLASS_NAME_(ParameterRegistryTest, set);
class GTEST_TEST_CLASS_NAME_(ParameterRegistryTest, setNotRegistered);
#endif

namespace hit
{
class Field;
}

namespace Moose
{

/**
 * Registry that allows for the typeless setting of a parameter value
 * from a hit field
 */
class ParameterRegistry
{
public:
  /**
   * Get the singleton registry
   */
  static ParameterRegistry & get();

  /**
   * Add a parameter
   */
  template <class T, class F>
  char add(F && f);

  /**
   * Sets a parameter value given a hit field
   */
  void set(libMesh::Parameters::Value & value, const hit::Field & field) const;

private:
  /**
   * Constructor; private so that it can only be created with the singleton
   */
  ParameterRegistry() {};

#ifdef MOOSE_UNIT_TEST
  FRIEND_TEST(::ParameterRegistryTest, add);
  FRIEND_TEST(::ParameterRegistryTest, addExists);
  FRIEND_TEST(::ParameterRegistryTest, set);
  FRIEND_TEST(::ParameterRegistryTest, setNotRegistered);
#endif

  /// Registration map of type -> function to fill each type
  std::map<std::string, std::function<void(libMesh::Parameters::Value & value, const hit::Field &)>>
      _registry;
};

template <class T, class F>
char
ParameterRegistry::add(F && f)
{
  static_assert(std::is_invocable_r_v<void, F &, T &, const hit::Field &>,
                "Setter function must be callable with (T &, const hit::Field &) and return void");

  // We register a function that stores T &, but the registry will only ever be
  // called from a bare libMesh::Parameters::Value. Thus, we need to cast the
  // bare Value to the derived value and then call the setter.
  auto setter = [f = std::forward<F>(f)](libMesh::Parameters::Value & param_value,
                                         const hit::Field & field) -> void
  {
    auto cast_param_value = dynamic_cast<libMesh::Parameters::Parameter<T> *>(&param_value);
    mooseAssert(cast_param_value, "Cast failed");
    f(cast_param_value->set(), field);
  };

  const auto key = demangle(typeid(T).name());
  const auto it_inserted_pair = _registry.emplace(key, std::move(setter));

  if (!it_inserted_pair.second)
    mooseError("ParameterRegistry: Parameter with type '", key, "' is already registered");

  return 0;
}

} // end of namespace Moose
