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

class FunctorWrapperRegistry
{
public:
  FunctorWrapperRegistry() = default;

  FunctorWrapperRegistry(FunctorWrapperRegistry const &) = delete;
  FunctorWrapperRegistry & operator=(FunctorWrapperRegistry const &) = delete;

  FunctorWrapperRegistry(FunctorWrapperRegistry &&) = delete;
  FunctorWrapperRegistry & operator=(FunctorWrapperRegistry &&) = delete;

  /**
   * Register a functor
   * @tparam Object The functor class type
   * @param name The registered functor type name
   */
  template <typename Object>
  static char addFunctor(const std::string & name)
  {
    getRegistry()._functors[name] = &functorBuilder<Object>;

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
    getRegistry()._functions[name] = &functionBuilder<Object>;

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

    return it->second(object);
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

    return it->second(object);
  }

private:
  /**
   * Get the registry singleton
   * @returns The registry singleton
   */
  static FunctorWrapperRegistry & getRegistry();

  using FunctorBuilder = std::unique_ptr<FunctorWrapperHostBase> (*)(const void * object);
  using FunctionBuilder = std::unique_ptr<FunctionWrapperHostBase> (*)(const void * object);

  /**
   * Build a host wrapper for a registered functor type
   * @tparam Object The functor class type
   * @param object The pointer to the functor
   * @returns The host functor wrapper
   */
  template <typename Object>
  static std::unique_ptr<FunctorWrapperHostBase> functorBuilder(const void * object)
  {
    return std::make_unique<FunctorWrapperHost<Object>>(object);
  }

  /**
   * Build a host wrapper for a registered function type
   * @tparam Object The function class type
   * @param object The pointer to the function
   * @returns The host function wrapper
   */
  template <typename Object>
  static std::unique_ptr<FunctionWrapperHostBase> functionBuilder(const void * object)
  {
    return std::make_unique<FunctionWrapperHost<Object>>(object);
  }

  /**
   * Map containing host functor wrapper builders keyed by registered object type name
   */
  std::map<std::string, FunctorBuilder> _functors;
  /**
   * Map containing host function wrapper builders keyed by registered object type name
   */
  std::map<std::string, FunctionBuilder> _functions;
};

} // namespace Moose::Kokkos

#define registerKokkosFunction(app, classname)                                                     \
  registerMooseObject(app, classname);                                                             \
  static char combineNames(kokkos_functor_##classname, __COUNTER__) =                              \
      Moose::Kokkos::FunctorWrapperRegistry::addFunctor<classname>(#classname);                    \
  static char combineNames(kokkos_function_##classname, __COUNTER__) =                             \
      Moose::Kokkos::FunctorWrapperRegistry::addFunction<classname>(#classname)

#define registerKokkosFunctionAliased(app, classname, alias)                                       \
  registerMooseObjectAliased(app, classname, alias);                                               \
  static char combineNames(kokkos_functor_##classname, __COUNTER__) =                              \
      Moose::Kokkos::FunctorWrapperRegistry::addFunctor<classname>(alias);                         \
  static char combineNames(kokkos_function_##classname, __COUNTER__) =                             \
      Moose::Kokkos::FunctorWrapperRegistry::addFunction<classname>(alias)
