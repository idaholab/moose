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
#include "KokkosFunctionWrapper.h"

namespace Moose::Kokkos
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
  std::unique_ptr<FunctorWrapperHostBase> build(const void * object) const override final
  {
    return std::make_unique<FunctorWrapperHost<Object>>(object);
  }
};

class FunctionRegistryEntryBase
{
public:
  virtual ~FunctionRegistryEntryBase() {}
  /**
   * Build a host wrapper for this function
   * @param object The pointer to the function
   */
  virtual std::unique_ptr<FunctionWrapperHostBase> build(const void * object) const = 0;
};

template <typename Object>
class FunctionRegistryEntry : public FunctionRegistryEntryBase
{
public:
  std::unique_ptr<FunctionWrapperHostBase> build(const void * object) const override final
  {
    return std::make_unique<FunctionWrapperHost<Object>>(object);
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
  static char addFunctor(const std::string & name)
  {
    getRegistry()._functors[name] = std::make_unique<FunctorRegistryEntry<Object>>();

    return 0;
  }

  /**
   * Register a function
   * @tparam Object The function class type
   * @param name The registered function type name
   */
  template <typename Object>
  static char addFunction(const std::string & name)
  {
    getRegistry()._functions[name] = std::make_unique<FunctionRegistryEntry<Object>>();

    return 0;
  }

  /**
   * Build and get a host wrapper of a functor
   * @param object The pointer to the functor
   * @param name The registered functor type name
   * @returns The host wrapper
   */
  static std::unique_ptr<FunctorWrapperHostBase> buildFunctor(const void * object,
                                                              const std::string & name)
  {
    auto it = getRegistry()._functors.find(name);
    if (it == getRegistry()._functors.end())
      mooseError("Kokkos functor not registered for type '",
                 name,
                 "'. Double check that you used Kokkos-specific registration macro.");

    return it->second->build(object);
  }

  /**
   * Build and get a host wrapper of a function
   * @param object The pointer to the function
   * @param name The registered function type name
   * @returns The host wrapper
   */
  static std::unique_ptr<FunctionWrapperHostBase> buildFunction(const void * object,
                                                                const std::string & name)
  {
    auto it = getRegistry()._functions.find(name);
    if (it == getRegistry()._functions.end())
      mooseError("Kokkos function not registered for type '",
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
   * Map containing the host wrapper shells of functors with the key being the registered object
   * type name
   */
  std::map<std::string, std::unique_ptr<FunctorRegistryEntryBase>> _functors;
  /**
   * Map containing the host wrapper shells of functions with the key being the registered object
   * type name
   */
  std::map<std::string, std::unique_ptr<FunctionRegistryEntryBase>> _functions;
};

} // namespace Moose::Kokkos

#define registerKokkosFunction(app, classname)                                                     \
  registerMooseObject(app, classname);                                                             \
  static char combineNames(kokkos_functor_##classname, __COUNTER__) =                              \
      Moose::Kokkos::FunctorRegistry::addFunctor<classname>(#classname);                           \
  static char combineNames(kokkos_function_##classname, __COUNTER__) =                             \
      Moose::Kokkos::FunctorRegistry::addFunction<classname>(#classname)

#define registerKokkosFunctionAliased(app, classname, alias)                                       \
  registerMooseObjectAliased(app, classname, alias);                                               \
  static char combineNames(kokkos_functor_##classname, __COUNTER__) =                              \
      Moose::Kokkos::FunctorRegistry::addFunctor<classname>(alias);                                \
  static char combineNames(kokkos_function_##classname, __COUNTER__) =                             \
      Moose::Kokkos::FunctorRegistry::addFunction<classname>(alias)
