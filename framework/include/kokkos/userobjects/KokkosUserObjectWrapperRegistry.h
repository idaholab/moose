//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosPreprocessorUtils.h"
#include "KokkosUserObjectWrapper.h"

#include "MooseError.h"
#include "MooseUtils.h"

#include <typeindex>
#include <type_traits>

namespace Moose::Kokkos
{

class UserObjectWrapperRegistry
{
public:
  /**
   * Register a user object
   * @tparam Base The base user object type
   * @tparam Object The derived user object type
   * @param name The derived user object type name
   */
  template <typename Base, typename Object>
  static char add(const std::string & name)
  {
    const auto [_, inserted] = getRegistry()._entries.emplace(
        std::make_pair(name, std::type_index(typeid(Base))), &builder<Base, Object>);
    if (!inserted)
      mooseError("Virtual Kokkos user object registration already exists for type '",
                 name,
                 "' and base class '",
                 MooseUtils::prettyCppType<Base>(),
                 "'.");

    return 0;
  }

  /**
   * Build and get a host wrapper of a user object
   * @tparam Base The base user object type
   * @param object The pointer to the derived user object
   * @param name The derived user object type name
   * @returns The host user object wrapper
   */
  template <typename Base>
  static std::shared_ptr<UserObjectWrapperHostBase> build(const void * object,
                                                          const std::string & name)
  {
    const auto entry =
        getRegistry()._entries.find(std::make_pair(name, std::type_index(typeid(Base))));
    if (entry == getRegistry()._entries.end())
      mooseError("Virtual Kokkos user object type '",
                 name,
                 "' is not registered with base class '",
                 MooseUtils::prettyCppType<Base>(),
                 "'.");

    return entry->second(object);
  }

private:
  /**
   * Get the registry singleton
   * @returns The registry singleton
   */
  static UserObjectWrapperRegistry & getRegistry();

  using Builder = std::shared_ptr<UserObjectWrapperHostBase> (*)(const void * object);

  /**
   * Build a host wrapper for a registered user object type
   * @tparam Base The base user object type
   * @tparam Object The derived user object type
   * @param object The pointer to the derived user object
   * @returns The host user object wrapper
   */
  template <typename Base, typename Object>
  static std::shared_ptr<UserObjectWrapperHostBase> builder(const void * object)
  {
    return std::make_shared<UserObjectWrapperHost<Base, Object>>(object);
  }

  /**
   * Map containing host user object wrapper builders keyed by registered object type name and base
   * type index
   */
  std::map<std::pair<std::string, std::type_index>, Builder> _entries;
};

/**
 * Generate the metadata and nested implementation types required for one named hook.
 *
 * For a registered Base::method with signature Return(Args...), Base_methodHook contains the
 * virtual device base, concrete forwarding device layer, and user-facing handle. It also validates
 * concrete implementations and connects the nested types to UserObjectWrapperInterface.
 *
 * The generated hook name includes both the registered base and method name so multiple virtual
 * user object bases can use identical hook names without colliding.
 */
#define MOOSE_KOKKOS_USER_OBJECT_HOOK_NAME(classname, method, suffix)                              \
  MOOSE_KOKKOS_PP_CAT(MOOSE_KOKKOS_PP_CAT(MOOSE_KOKKOS_PP_CAT(classname, _), method), suffix)
#define MOOSE_KOKKOS_USER_OBJECT_DECLARE_HOOK(classname, method)                                   \
  template <typename Signature>                                                                    \
  struct MOOSE_KOKKOS_USER_OBJECT_HOOK_NAME(classname, method, Hook);                              \
  template <typename Return, typename... Args>                                                     \
  struct MOOSE_KOKKOS_USER_OBJECT_HOOK_NAME(classname, method, Hook)<Return(Args...)>              \
  {                                                                                                \
    using Signature = Return(Args...);                                                             \
    class DeviceBase                                                                               \
    {                                                                                              \
    public:                                                                                        \
      KOKKOS_FUNCTION KOKKOS_VIRTUAL Return method(Args...) const                                  \
      {                                                                                            \
        KOKKOS_ASSERT(false);                                                                      \
        return Return();                                                                           \
      }                                                                                            \
    };                                                                                             \
    template <typename Object, typename Base>                                                      \
    class Device : public Base                                                                     \
    {                                                                                              \
    public:                                                                                        \
      using Base::Base;                                                                            \
      KOKKOS_FUNCTION Return method(Args... args) const KOKKOS_OVERRIDE                            \
      {                                                                                            \
        return this->_object->method(static_cast<Args &&>(args)...);                               \
      }                                                                                            \
    };                                                                                             \
    template <typename Owner>                                                                      \
    class Handle                                                                                   \
    {                                                                                              \
    public:                                                                                        \
      KOKKOS_FUNCTION Return method(Args... args) const                                            \
      {                                                                                            \
        const auto hook_device =                                                                   \
            static_cast<const Owner *>(this)->template getHookDevice<DeviceBase>();                \
        return hook_device->method(static_cast<Args &&>(args)...);                                 \
      }                                                                                            \
    };                                                                                             \
    template <typename Object>                                                                     \
    static constexpr bool matches =                                                                \
        std::is_same_v<                                                                            \
            typename Moose::Kokkos::UserObjectHookTraits<decltype(&Object::method)>::ObjectType,   \
            Object> &&                                                                             \
        std::is_same_v<                                                                            \
            typename Moose::Kokkos::UserObjectHookTraits<decltype(&Object::method)>::Signature,    \
            Signature>;                                                                            \
  };

/**
 * Transform a hook name into the metadata type generated by MOOSE_KOKKOS_USER_OBJECT_DECLARE_HOOK
 */
#define MOOSE_KOKKOS_USER_OBJECT_HOOK_TRANSFORM(classname, method)                                 \
  Moose::Kokkos::detail::MOOSE_KOKKOS_USER_OBJECT_HOOK_NAME(                                       \
      classname,                                                                                   \
      method,                                                                                      \
      Hook)<typename Moose::Kokkos::UserObjectHookTraits<decltype(&classname::method)>::Signature>

/**
 * Register a virtual Kokkos user object base and its named hook methods.
 *
 * This macro must appear in the base header after the complete class definition. MAP emits the
 * declarations and metadata for every hook. MAP_LIST transforms the same hook names into the
 * template argument list used by UserObjectWrapperInterface. The registration specialization
 * validates every concrete Object and connects it to the runtime registry.
 * The specialization is declared with a qualified name and intentionally omits its terminating
 * semicolon, allowing the conventional semicolon following the macro invocation to terminate the
 * specialization itself.
 */
#define registerVirtualKokkosUserObjectBase(classname, ...)                                        \
  namespace Moose::Kokkos::detail                                                                  \
  {                                                                                                \
  MOOSE_KOKKOS_PP_MAP(MOOSE_KOKKOS_USER_OBJECT_DECLARE_HOOK, classname, __VA_ARGS__)               \
  }                                                                                                \
  template <>                                                                                      \
  struct Moose::Kokkos::UserObjectWrapperRegistration<classname>                                   \
  {                                                                                                \
    using Interface = UserObjectWrapperInterface<MOOSE_KOKKOS_PP_MAP_LIST(                         \
        MOOSE_KOKKOS_USER_OBJECT_HOOK_TRANSFORM, classname, __VA_ARGS__)>;                         \
    template <typename Object>                                                                     \
    static char add(const std::string & object_name)                                               \
    {                                                                                              \
      static_assert(std::is_base_of_v<classname, Object>,                                          \
                    "Registered Kokkos user object must derive from its virtual base class");      \
      static_assert(Interface::template matches<Object>,                                           \
                    "Registered Kokkos user object must define every registered hook with the "    \
                    "declared signature");                                                         \
      return UserObjectWrapperRegistry::add<classname, Object>(object_name);                       \
    }                                                                                              \
  }

/**
 * Register a concrete user object type with an explicitly specified virtual base class.
 *
 * The first argument is the concrete derived object and the second argument is the base registered
 * with registerVirtualKokkosUserObjectBase(). The resulting static variable invokes
 * add<derived>() during startup. The base registration verifies the inheritance relationship and
 * every hook signature before adding the entry. Explicitly naming the base also allows an
 * intermediate class to be registered as a concrete implementation of one interface while serving
 * as the registered virtual base of another interface.
 */
#define registerVirtualKokkosUserObject(derived, base)                                             \
  [[maybe_unused]] static char combineNames(kokkos_user_object_wrapper_, __COUNTER__) =            \
      Moose::Kokkos::UserObjectWrapperRegistration<base>::template add<derived>(#derived)

} // namespace Moose::Kokkos
