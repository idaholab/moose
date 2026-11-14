//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosTypes.h"

#include <memory>
#include <utility>

namespace Moose::Kokkos
{

/**
 * Common device-side polymorphic base for one registered virtual user object interface.
 *
 * Each HookBases type declares one named virtual hook. Multiple inheritance combines those
 * independent hook bases into one object with all registered entry points. The wrapper is
 * constructed in device-accessible memory so its virtual table contains addresses valid for the
 * active Kokkos execution space.
 */
template <typename... HookBases>
class UserObjectWrapperDeviceBase : public HookBases...
{
public:
  /// Construct the common base in the memory space where virtual calls will execute.
  KOKKOS_FUNCTION UserObjectWrapperDeviceBase() {}

  /// Provide a virtual device destructor so the common base is polymorphic.
  KOKKOS_FUNCTION KOKKOS_VIRTUAL ~UserObjectWrapperDeviceBase() {}
};

/**
 * Store the concrete user object pointer at the bottom of the generated device inheritance chain.
 *
 * Hook-specific device classes inherit from one another and eventually reach this class. They all
 * therefore share one pointer to the copied concrete object while independently overriding their
 * corresponding named hook.
 */
template <typename Object, typename Base>
class UserObjectWrapperDeviceStorage : public Base
{
public:
  KOKKOS_FUNCTION UserObjectWrapperDeviceStorage(Object * object) : _object(object) {}

protected:
  /// Concrete user object copy in the active Kokkos memory space.
  Object * _object;
};

/**
 * Recursively compose the hook-specific device classes for a concrete user object.
 *
 * For hooks H1, H2, ..., the resulting type is equivalent to
 * H1::Device<Object, H2::Device<Object, ... Storage<Object, DeviceBase>>>. Each layer overrides
 * one virtual hook and forwards that hook statically through the stored Object pointer.
 */
template <typename Object, typename Base, typename... Hooks>
struct UserObjectWrapperDeviceHooks;

/// Recursion terminator: no hooks remain, so retain the accumulated base type.
template <typename Object, typename Base>
struct UserObjectWrapperDeviceHooks<Object, Base>
{
  using type = Base;
};

/// Add the current hook layer outside the device type generated for the remaining hooks.
template <typename Object, typename Base, typename Hook, typename... Hooks>
struct UserObjectWrapperDeviceHooks<Object, Base, Hook, Hooks...>
{
  using Next = typename UserObjectWrapperDeviceHooks<Object, Base, Hooks...>::type;
  using type = typename Hook::template Device<Object, Next>;
};

template <typename... Hooks>
class VirtualUserObjectImplementation;

template <typename Wrapper>
struct UserObjectWrapperRegistration;

/**
 * Type-erased host ownership for a copied Kokkos user object and its device wrapper.
 *
 * The registry must store entries for unrelated concrete user object types, so allocation and copy
 * operations are exposed through this non-template base. The derived host wrapper owns the device
 * object and device wrapper allocations and is shared by every VirtualUserObjectImplementation
 * handle copy.
 */
class UserObjectWrapperHostBase
{
public:
  virtual ~UserObjectWrapperHostBase() {}

  /**
   * Allocate the concrete object storage and construct its device wrapper.
   * @returns The wrapper viewed through the common registered device base
   */
  virtual void * allocate() = 0;

  /**
   * Invoke the concrete object's host copy constructor and copy that result into device storage.
   */
  virtual void copyObject() = 0;

  /// Release the temporary host copy created by copyObject().
  virtual void freeObject() = 0;
};

/**
 * Host ownership specialized for one concrete user object and its hooks.
 *
 * A normal MOOSE object is constructed on the host and can contain host-only references that must
 * be converted by its Kokkos copy constructor before use in a parallel dispatch. This class keeps
 * a reference to that original object, creates the converted host copy when the virtual handle is
 * copied into a Kokkos object, and transfers the converted bytes to the preallocated device object.
 */
template <typename Wrapper, typename Object>
class UserObjectWrapperHost final : public UserObjectWrapperHostBase
{
public:
  using Registration = UserObjectWrapperRegistration<Wrapper>;
  using DeviceBase = typename Registration::DeviceBase;
  using DeviceWrapper = typename Registration::template Device<Object>;

  UserObjectWrapperHost(const void * object) : _object_host(*static_cast<const Object *>(object)) {}

  ~UserObjectWrapperHost();

  void * allocate() override;
  void copyObject() override;
  void freeObject() override;

private:
  /// Original MOOSE object whose concrete dynamic type is Object.
  const Object & _object_host;

  /// Temporary host copy used to invoke Object's Kokkos-aware copy constructor.
  std::unique_ptr<Object> _object_copy;

  /// Raw storage for the converted Object in the active Kokkos memory space.
  Object * _object_device = nullptr;

  /// Concrete virtual wrapper in the active Kokkos memory space.
  DeviceWrapper * _wrapper_device = nullptr;
};

template <typename Wrapper, typename Object>
void *
UserObjectWrapperHost<Wrapper, Object>::allocate()
{
  // Reserve object storage first so the device wrapper can be constructed with its final pointer.
  _object_device =
      static_cast<Object *>(::Kokkos::kokkos_malloc<ExecSpace::memory_space>(sizeof(Object)));

  // The concrete wrapper type contains the device virtual table and one static forwarding layer for
  // every registered hook.
  _wrapper_device = static_cast<DeviceWrapper *>(
      ::Kokkos::kokkos_malloc<ExecSpace::memory_space>(sizeof(DeviceWrapper)));

  // Placement construction must execute in the target execution space. Constructing this object on
  // the host and copying its bytes would copy a host virtual table, which is invalid on a GPU.
  auto object_device = _object_device;
  auto wrapper_device = _wrapper_device;
  ::Kokkos::parallel_for(
      1, KOKKOS_LAMBDA(const int) { new (wrapper_device) DeviceWrapper(object_device); });

  // DeviceWrapper derives from the common base generated for Wrapper, so callers can erase the
  // concrete type while retaining virtual access to all registered hooks.
  return static_cast<DeviceBase *>(_wrapper_device);
}

template <typename Wrapper, typename Object>
void
UserObjectWrapperHost<Wrapper, Object>::copyObject()
{
  // Invoke the concrete Kokkos copy constructor on the host before transferring bytes. Kokkos-MOOSE
  // objects use this copy step to replace host references with device-accessible data handles.
  _object_copy = std::make_unique<Object>(_object_host);

  // Object is intentionally copied as raw storage after its copy constructor has prepared all
  // device-facing members, matching the established Kokkos functor and function wrapper pattern.
  ::Kokkos::Impl::DeepCopy<MemSpace, ::Kokkos::HostSpace>(
      _object_device, _object_copy.get(), sizeof(Object));
}

template <typename Wrapper, typename Object>
void
UserObjectWrapperHost<Wrapper, Object>::freeObject()
{
  // The device object remains allocated for the shared handle lifetime; only the temporary host
  // copy is associated with an individual dispatch copy and can be released here.
  _object_copy.reset();
}

template <typename Wrapper, typename Object>
UserObjectWrapperHost<Wrapper, Object>::~UserObjectWrapperHost()
{
  ::Kokkos::kokkos_free<ExecSpace::memory_space>(_wrapper_device);
  ::Kokkos::kokkos_free<ExecSpace::memory_space>(_object_device);
}

/**
 * Lightweight user-facing value that exposes all registered hook names.
 *
 * Each empty Hooks::Handle base contributes one ordinary named method such as value() or combine().
 * The method views the common device wrapper through its hook-specific virtual base. Host ownership
 * is shared so copies placed in Kokkos dispatch objects refer to the same allocated concrete object
 * and device wrapper.
 */
template <typename... Hooks>
class VirtualUserObjectImplementation
  : public Hooks::template Handle<VirtualUserObjectImplementation<Hooks...>>...
{
  using DeviceBase = UserObjectWrapperDeviceBase<typename Hooks::DeviceBase...>;

public:
  /// Construct an empty handle for containers or optional members that require default construction.
  VirtualUserObjectImplementation() = default;

  /// Allocate the device wrapper using the type-erased host owner supplied by the registry.
  VirtualUserObjectImplementation(std::shared_ptr<UserObjectWrapperHostBase> wrapper);

  /// Share ownership and synchronize the concrete object copy for a Kokkos dispatch copy.
  VirtualUserObjectImplementation(const VirtualUserObjectImplementation & object);

  /// Release per-copy state; the shared host owner frees device storage with the last handle.
  ~VirtualUserObjectImplementation();

  /// Whether this handle contains a host owner or device wrapper in the current execution context.
  KOKKOS_FUNCTION explicit operator bool() const;

  /// View the common device wrapper through the virtual base associated with one hook.
  template <typename HookDeviceBase>
  KOKKOS_FUNCTION HookDeviceBase * getHookDevice() const;

private:
  /// Shared host owner that knows the registered concrete Object type.
  std::shared_ptr<UserObjectWrapperHostBase> _wrapper_host;

  /// Common device base containing the virtual entry points for all hooks.
  DeviceBase * _wrapper_device = nullptr;
};

template <typename... Hooks>
template <typename HookDeviceBase>
KOKKOS_FUNCTION HookDeviceBase *
VirtualUserObjectImplementation<Hooks...>::getHookDevice() const
{
  KOKKOS_ASSERT(_wrapper_device);
  return static_cast<HookDeviceBase *>(_wrapper_device);
}

template <typename... Hooks>
VirtualUserObjectImplementation<Hooks...>::VirtualUserObjectImplementation(
    std::shared_ptr<UserObjectWrapperHostBase> wrapper)
  : _wrapper_host(std::move(wrapper)),
    _wrapper_device(static_cast<DeviceBase *>(_wrapper_host->allocate()))
{
}

template <typename... Hooks>
VirtualUserObjectImplementation<Hooks...>::VirtualUserObjectImplementation(
    const VirtualUserObjectImplementation & object)
  : _wrapper_host(object._wrapper_host), _wrapper_device(object._wrapper_device)
{
  // Copying the containing Kokkos object is the point at which its members are converted for the
  // target execution space, so synchronize the concrete user object into its allocated storage.
  if (_wrapper_host)
    _wrapper_host->copyObject();
}

template <typename... Hooks>
VirtualUserObjectImplementation<Hooks...>::~VirtualUserObjectImplementation()
{
  // Release the host-side converted copy associated with the most recent dispatch copy.
  if (_wrapper_host)
    _wrapper_host->freeObject();
}

template <typename... Hooks>
KOKKOS_FUNCTION VirtualUserObjectImplementation<Hooks...>::operator bool() const
{
  // std::shared_ptr is host-only, while the raw wrapper pointer is valid in Kokkos device code.
  KOKKOS_IF_ON_HOST(return static_cast<bool>(_wrapper_host);)

  return _wrapper_device != nullptr;
}

/// Public type generated from the hook metadata registered for Base.
template <typename Base>
using VirtualUserObject = typename UserObjectWrapperRegistration<Base>::Handle;

} // namespace Moose::Kokkos
