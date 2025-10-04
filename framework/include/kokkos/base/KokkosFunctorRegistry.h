//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosFunctorWrapper.h"

namespace Moose
{
namespace Kokkos
{

class FunctorRegistryEntryBase
{
public:
  virtual ~FunctorRegistryEntryBase() {}
  /**
   * Build a host wrapper for this functor
   * @param object The pointer to the functor
   */
  virtual std::unique_ptr<FunctorWrapperHostBase> build(const void * object) const = 0;
};

template <typename Object>
class FunctorRegistryEntry : public FunctorRegistryEntryBase
{
public:
  virtual std::unique_ptr<FunctorWrapperHostBase> build(const void * object) const
  {
    return std::make_unique<FunctorWrapperHost<Object>>(object);
  }
};

class FunctorRegistry
{
public:
  FunctorRegistry() = default;

  FunctorRegistry(FunctorRegistry const &) = delete;
  FunctorRegistry & operator=(FunctorRegistry const &) = delete;

  FunctorRegistry(FunctorRegistry &&) = delete;
  FunctorRegistry & operator=(FunctorRegistry &&) = delete;

  /**
   * Register a functor
   * @tparam Object The functor class type
   * @param name The registered functor type name
   */
  template <typename Object>
  static char add(const std::string & name)
  {
    getRegistry()._functors[name] = std::make_unique<FunctorRegistryEntry<Object>>();

    return 0;
  }

  /**
   * Build and get a host wrapper of a functor
   * @param object The pointer to the functor
   * @param name The registered functor type name
   * @returns The host wrapper
   */
  static std::unique_ptr<FunctorWrapperHostBase> build(const void * object,
                                                       const std::string & name)
  {
    auto it = getRegistry()._functors.find(name);
    if (it == getRegistry()._functors.end())
      mooseError("Kokkos functor not registered for type '",
                 name,
                 "'. Double check that you used Kokkos-specific registration macro.");

    return it->second->build(object);
  }

private:
  /**
   * Get the registry singleton
   * @returns The registry singleton
   */
  static FunctorRegistry & getRegistry();

  /**
   * Map containing the host wrapper shells with the key being the registered object type name
   */
  std::map<std::string, std::unique_ptr<FunctorRegistryEntryBase>> _functors;
};

} // namespace Kokkos
} // namespace Moose

#define registerKokkosFunctor(app, classname)                                                      \
  registerMooseObject(app, classname);                                                             \
  static char combineNames(kokkos_functor_##classname, __COUNTER__) =                              \
      Moose::Kokkos::FunctorRegistry::add<classname>(#classname)

#define registerKokkosFunctorAliased(app, classname, alias)                                        \
  registerMooseObjectAliased(app, classname, alias);                                               \
  static char combineNames(kokkos_functor_##classname, __COUNTER__) =                              \
      Moose::Kokkos::FunctorRegistry::add<classname>(alias)
