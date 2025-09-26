//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosHeader.h"
#include "KokkosThread.h"

#include <typeindex>

namespace Moose
{
namespace Kokkos
{

using Policy = ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>>;

/**
 * Base class for Kokkos functor dispatcher.
 * Used for type erasure so that the base class of functors can hold the dispatcher without knowing
 * the actual type of functors.
 */
class DispatcherBase
{
public:
  virtual ~DispatcherBase() {}
  /**
   * Dispatch this functor with Kokkos parallel_for() given a Kokkos execution policy
   * @param policy The Kokkos execution policy
   */
  virtual void parallelFor(const Policy & policy) = 0;
};

/**
 * Class that dispatches an operation of a Kokkos functor.
 * Calls operator() of the functor with a specified function tag.
 * @tparam Operation The function tag of operator() to be dispatched
 * @tparam Object The functor class type
 */
template <typename Operation, typename Object>
class Dispatcher : public DispatcherBase
{
public:
  /**
   * Constructor
   * @param object The pointer to the functor. This dispatcher is constructed by the base class of
   * functors, and the actual type of functors is unknown by the base class. Therefore, it is passed
   * as a void pointer and cast to the actual type here.
   */
  Dispatcher(const void * object)
    : _functor_host(*static_cast<const Object *>(object)), _functor_device(_functor_host)
  {
  }
  /**
   * Copy constructor for parallel dispatch
   */
  Dispatcher(const Dispatcher & functor)
    : _functor_host(functor._functor_host), _functor_device(functor._functor_host)
  {
  }

  void parallelFor(const Policy & policy) override final
  {
    ::Kokkos::parallel_for(policy, *this);
    ::Kokkos::fence();
  }

  /**
   * The parallel computation entry function called by Kokkos
   */
  KOKKOS_FUNCTION void operator()(const ThreadID tid) const
  {
    _functor_device(Operation{}, tid, _functor_device);
  }

private:
  /**
   * Reference of the functor on host
   */
  const Object & _functor_host;
  /**
   * Copy of the functor on device
   */
  const Object _functor_device;
};

/**
 * Base class for dispatcher registry entry.
 * Used for type erasure so that the registry can hold dispatchers for different functor types in a
 * single container.
 */
class DispatcherRegistryEntryBase
{
public:
  virtual ~DispatcherRegistryEntryBase() {}
  /**
   * Build a dispatcher for this operation and functor
   * @param object The pointer to the functor
   */
  virtual std::unique_ptr<DispatcherBase> build(const void * object) const = 0;

  /**
   * Set whether the user has overriden the hook method associated with this operation
   * @param flag Whether the user has overriden the hook method
   */
  void hasUserMethod(bool flag) { _has_user_method = flag; }
  /**
   * Get whether the user has overriden the hook method associated with this operation
   * @returns Whether the user has overriden the hook method
   */
  bool hasUserMethod() const { return _has_user_method; }

private:
  /**
   * Flag whether the user has overriden the hook method associated with this operation
   */
  bool _has_user_method = false;
};

/**
 * Class that stores the information of a dispatcher and builds it.
 * This shell class is the entry of the dispatcher registry instead of the dispatcher itself.
 * The reason this class does not dispatch the functor directly is to let the dispatcher hold
 * the reference of the functor so that the functor does not need to be copied twice at each
 * dispatch. Namely, dispatchers are to be built and held by the functors, not the registry.
 * @tparam Operation The function tag of operator() to be dispatched
 * @tparam Object The functor class type
 */
template <typename Operation, typename Object>
class DispatcherRegistryEntry : public DispatcherRegistryEntryBase
{
public:
  std::unique_ptr<DispatcherBase> build(const void * object) const override final
  {
    return std::make_unique<Dispatcher<Operation, Object>>(object);
  }
};

/**
 * Class that registers dispatchers of all Kokkos functors
 */
class DispatcherRegistry
{
public:
  DispatcherRegistry() = default;

  DispatcherRegistry(DispatcherRegistry const &) = delete;
  DispatcherRegistry & operator=(DispatcherRegistry const &) = delete;

  DispatcherRegistry(DispatcherRegistry &&) = delete;
  DispatcherRegistry & operator=(DispatcherRegistry &&) = delete;

  /**
   * Register a dispatcher of an operation of a functor
   * @tparam Operation The function tag of operator() to be dispatched
   * @tparam Object The functor class type
   * @param name The registered object type name
   */
  template <typename Operation, typename Object>
  static void add(const std::string & name)
  {
    auto operation = std::type_index(typeid(Operation));

    getRegistry()._dispatchers[std::make_pair(operation, name)] =
        std::make_unique<DispatcherRegistryEntry<Operation, Object>>();
  }

  /**
   * Set whether the user has overriden the hook method associated with an operation of a functor
   * @tparam Operation The function tag of operator()
   * @param name The registered object type name
   * @param flag Whether the user has overriden the hook method
   */
  template <typename Operation>
  static void hasUserMethod(const std::string & name, bool flag)
  {
    getDispatcher<Operation>(name)->hasUserMethod(flag);
  }

  /**
   * Get whether the user has overriden the hook method associated with an operation of a functor
   * @tparam Operation The function tag of operator()
   * @param name The registered object type name
   * @returns Whether the user has overriden the hook method
   */
  template <typename Operation>
  static bool hasUserMethod(const std::string & name)
  {
    return getDispatcher<Operation>(name)->hasUserMethod();
  }

  /**
   * Build and get a dispatcher of an operation of a functor
   * @tparam Operation The function tag of operator()
   * @param object The pointer to the functor
   * @param name The registered object type name
   * @returns The dispatcher
   */
  template <typename Operation>
  static std::unique_ptr<DispatcherBase> build(const void * object, const std::string & name)
  {
    return getDispatcher<Operation>(name)->build(object);
  }

private:
  /**
   * Get the registry singleton
   * @returns The registry singleton
   */
  static DispatcherRegistry & getRegistry();

  /**
   * Get the dispatcher shell of an operation of a functor
   * @tparam Operation The function tag of operator()
   * @param name The registered object type name
   * @returns The dispatcher shell
   */
  template <typename Operation>
  static auto & getDispatcher(const std::string & name)
  {
    auto operation = std::type_index(typeid(Operation));

    auto it = getRegistry()._dispatchers.find(std::make_pair(operation, name));
    if (it == getRegistry()._dispatchers.end())
      mooseError("Kokkos functor dispatcher not registered for object type '",
                 name,
                 "'. Double check that you used Kokkos-specific registration macro.");

    return it->second;
  }

  /**
   * Map containing the dispatcher shells with the key being the pair of function tag type index and
   * registered object type name
   */
  std::map<std::pair<std::type_index, std::string>, std::unique_ptr<DispatcherRegistryEntryBase>>
      _dispatchers;
};

} // namespace Kokkos
} // namespace Moose

// Kernel, NodalKernel, BC

#define callRegisterKokkosResidualObjectFunction(classname, objectname)                            \
  static char registerKokkosResidualObject##classname()                                            \
  {                                                                                                \
    using namespace Moose::Kokkos;                                                                 \
                                                                                                   \
    DispatcherRegistry::add<classname::ResidualLoop, classname>(objectname);                       \
    DispatcherRegistry::add<classname::JacobianLoop, classname>(objectname);                       \
    DispatcherRegistry::add<classname::OffDiagJacobianLoop, classname>(objectname);                \
    DispatcherRegistry::hasUserMethod<classname::JacobianLoop>(                                    \
        objectname, &classname::computeQpJacobian != classname::defaultJacobian());                \
    DispatcherRegistry::hasUserMethod<classname::OffDiagJacobianLoop>(                             \
        objectname, &classname::computeQpOffDiagJacobian != classname::defaultOffDiagJacobian());  \
                                                                                                   \
    return 0;                                                                                      \
  }                                                                                                \
                                                                                                   \
  static char combineNames(kokkos_dispatcher_residual_object_##classname, __COUNTER__) =           \
      registerKokkosResidualObject##classname()

#define registerKokkosResidualObject(app, classname)                                               \
  registerMooseObject(app, classname);                                                             \
  callRegisterKokkosResidualObjectFunction(classname, #classname)

#define registerKokkosResidualObjectAliased(app, classname, alias)                                 \
  registerMooseObjectAliased(app, classname, alias);                                               \
  callRegisterKokkosResidualObjectFunction(classname, alias)

// Material

#define callRegisterKokkosMaterialFunction(classname, objectname)                                  \
  static char registerKokkosMaterial##classname()                                                  \
  {                                                                                                \
    using namespace Moose::Kokkos;                                                                 \
                                                                                                   \
    DispatcherRegistry::add<classname::ElementInit, classname>(objectname);                        \
    DispatcherRegistry::add<classname::SideInit, classname>(objectname);                           \
    DispatcherRegistry::add<classname::NeighborInit, classname>(objectname);                       \
    DispatcherRegistry::add<classname::ElementCompute, classname>(objectname);                     \
    DispatcherRegistry::add<classname::SideCompute, classname>(objectname);                        \
    DispatcherRegistry::add<classname::NeighborCompute, classname>(objectname);                    \
    DispatcherRegistry::hasUserMethod<classname::ElementInit>(                                     \
        objectname, &classname::initQpStatefulProperties != classname::defaultInitStateful());     \
    DispatcherRegistry::hasUserMethod<classname::SideInit>(                                        \
        objectname, &classname::initQpStatefulProperties != classname::defaultInitStateful());     \
    DispatcherRegistry::hasUserMethod<classname::NeighborInit>(                                    \
        objectname, &classname::initQpStatefulProperties != classname::defaultInitStateful());     \
                                                                                                   \
    return 0;                                                                                      \
  }                                                                                                \
                                                                                                   \
  static char combineNames(kokkos_dispatcher_material_##classname, __COUNTER__) =                  \
      registerKokkosMaterial##classname()

#define registerKokkosMaterial(app, classname)                                                     \
  registerMooseObject(app, classname);                                                             \
  callRegisterKokkosMaterialFunction(classname, #classname)

#define registerKokkosMaterialAliased(app, classname, alias)                                       \
  registerMooseObjectAliased(app, classname, alias);                                               \
  callRegisterKokkosMaterialFunction(classname, alias)

// AuxKernel

#define callRegisterKokkosAuxKernelFunction(classname, objectname)                                 \
  static char registerKokkosAuxKernel##classname()                                                 \
  {                                                                                                \
    using namespace Moose::Kokkos;                                                                 \
                                                                                                   \
    DispatcherRegistry::add<classname::ElementLoop, classname>(objectname);                        \
    DispatcherRegistry::add<classname::NodeLoop, classname>(objectname);                           \
                                                                                                   \
    return 0;                                                                                      \
  }                                                                                                \
                                                                                                   \
  static char combineNames(kokkos_dispatcher_auxkernel_##classname, __COUNTER__) =                 \
      registerKokkosAuxKernel##classname()

#define registerKokkosAuxKernel(app, classname)                                                    \
  registerMooseObject(app, classname);                                                             \
  callRegisterKokkosAuxKernelFunction(classname, #classname)

#define registerKokkosAuxKernelAliased(app, classname, alias)                                      \
  registerMooseObjectAliased(app, classname, alias);                                               \
  callRegisterKokkosAuxKernelFunction(classname, alias)
