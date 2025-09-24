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

class DispatcherBase
{
public:
  virtual ~DispatcherBase() {}
  virtual void parallelFor(const Policy & policy) = 0;
};

template <typename Operation, typename Object>
class Dispatcher : public DispatcherBase
{
public:
  Dispatcher(const void * object)
    : _functor_host(*static_cast<const Object *>(object)), _functor_device(_functor_host)
  {
  }
  Dispatcher(const Dispatcher & functor)
    : _functor_host(functor._functor_host), _functor_device(functor._functor_host)
  {
  }

  void parallelFor(const Policy & policy) override final
  {
    ::Kokkos::parallel_for(policy, *this);
    ::Kokkos::fence();
  }

  KOKKOS_FUNCTION void operator()(const ThreadID tid) const
  {
    _functor_device(Operation{}, tid, _functor_device);
  }

private:
  const Object & _functor_host;
  const Object _functor_device;
};

class DispatcherRegistryEntryBase
{
public:
  virtual ~DispatcherRegistryEntryBase() {}
  virtual std::unique_ptr<DispatcherBase> build(const void * object) const = 0;

  void hasUserMethod(bool flag) { _has_user_method = flag; }
  bool hasUserMethod() const { return _has_user_method; }

private:
  bool _has_user_method = false;
};

template <typename Operation, typename Object>
class DispatcherRegistryEntry : public DispatcherRegistryEntryBase
{
public:
  std::unique_ptr<DispatcherBase> build(const void * object) const override final
  {
    return std::make_unique<Dispatcher<Operation, Object>>(object);
  }
};

class DispatcherRegistry
{
public:
  DispatcherRegistry() = default;

  DispatcherRegistry(DispatcherRegistry const &) = delete;
  DispatcherRegistry & operator=(DispatcherRegistry const &) = delete;

  DispatcherRegistry(DispatcherRegistry &&) = delete;
  DispatcherRegistry & operator=(DispatcherRegistry &&) = delete;

  template <typename Operation, typename Object>
  static void add(const std::string & name)
  {
    auto operation = std::type_index(typeid(Operation));

    getRegistry()._dispatchers[std::make_pair(operation, name)] =
        std::make_unique<DispatcherRegistryEntry<Operation, Object>>();
  }

  template <typename Operation>
  static void hasUserMethod(const std::string & name, bool flag)
  {
    getDispatcher<Operation>(name)->hasUserMethod(flag);
  }

  template <typename Operation>
  static bool hasUserMethod(const std::string & name)
  {
    return getDispatcher<Operation>(name)->hasUserMethod();
  }

  template <typename Operation>
  static std::unique_ptr<DispatcherBase> build(const void * object, const std::string & name)
  {
    return getDispatcher<Operation>(name)->build(object);
  }

private:
  static DispatcherRegistry & getRegistry();

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

  std::map<std::pair<std::type_index, std::string>, std::unique_ptr<DispatcherRegistryEntryBase>>
      _dispatchers;
};

} // namespace Kokkos
} // namespace Moose

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
