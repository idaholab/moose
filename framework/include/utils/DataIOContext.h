//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <type_traits>
#include <vector>
#include <set>
#include <list>
#include <deque>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <memory>

// Forward declarations for libMesh types
namespace libMesh
{
class MeshBase;
class Elem;
class Node;
namespace Parallel
{
class Communicator;
}
template <typename T>
class NumericVector;
} // namespace libMesh

class MultiApp;

namespace Moose
{

/**
 * Type traits for compile-time context checking in dataLoad/dataStore.
 *
 * These traits define what context type is required for loading/storing
 * specific data types. By default, no specific context is required (void).
 *
 * Key insight: Store and Load have asymmetric requirements:
 * - dataStore for Elem/Node pointers: Does NOT use context (stores element ID only)
 * - dataLoad for Elem/Node pointers: REQUIRES MeshBase pointer (looks up element by ID)
 */

/// Default: no specific context required for loading (accepts anything)
template <typename T>
struct required_load_context
{
  using type = void;
};

/// Default: no specific context required for storing (accepts anything)
template <typename T>
struct required_store_context
{
  using type = void;
};

// Elem/Node load requires MeshBase* - declared but defined in DataIO.C due to incomplete types
template <>
struct required_load_context<const libMesh::Elem *>
{
  using type = libMesh::MeshBase *;
};
template <>
struct required_load_context<libMesh::Elem *>
{
  using type = libMesh::MeshBase *;
};
template <>
struct required_load_context<const libMesh::Node *>
{
  using type = libMesh::MeshBase *;
};
template <>
struct required_load_context<libMesh::Node *>
{
  using type = libMesh::MeshBase *;
};

// Helper aliases
template <typename T>
using required_load_context_t = typename required_load_context<T>::type;

template <typename T>
using required_store_context_t = typename required_store_context<T>::type;

/**
 * Check if a provided context type is compatible with a required context type.
 *
 * A context is compatible if:
 * - No requirement exists (Required is void)
 * - The required type accepts nullptr
 * - The provided type is convertible to the required type
 */
template <typename Required, typename Provided>
inline constexpr bool context_compatible_v =
    std::is_void_v<Required> ||                                   // No requirement
    std::is_same_v<std::remove_cv_t<Required>, std::nullptr_t> || // Accepts nullptr
    std::is_convertible_v<Provided, Required>;                    // Convertible

// Propagate context requirements through containers

// std::vector
template <typename T>
struct required_load_context<std::vector<T>> : required_load_context<T>
{
};
template <typename T>
struct required_store_context<std::vector<T>> : required_store_context<T>
{
};

// std::set
template <typename T>
struct required_load_context<std::set<T>> : required_load_context<T>
{
};
template <typename T>
struct required_store_context<std::set<T>> : required_store_context<T>
{
};

// std::list
template <typename T>
struct required_load_context<std::list<T>> : required_load_context<T>
{
};
template <typename T>
struct required_store_context<std::list<T>> : required_store_context<T>
{
};

// std::deque
template <typename T>
struct required_load_context<std::deque<T>> : required_load_context<T>
{
};
template <typename T>
struct required_store_context<std::deque<T>> : required_store_context<T>
{
};

// std::unordered_set
template <typename T>
struct required_load_context<std::unordered_set<T>> : required_load_context<T>
{
};
template <typename T>
struct required_store_context<std::unordered_set<T>> : required_store_context<T>
{
};

// std::optional
template <typename T>
struct required_load_context<std::optional<T>> : required_load_context<T>
{
};
template <typename T>
struct required_store_context<std::optional<T>> : required_store_context<T>
{
};

// std::pair - requires compatible context for both types
template <typename T, typename U>
struct required_load_context<std::pair<T, U>>
{
  // If either type requires a specific context, use that. If both require different contexts,
  // this will need to be handled specially (currently not supported - would need void)
  using type = std::conditional_t<
      !std::is_void_v<required_load_context_t<T>>,
      required_load_context_t<T>,
      std::conditional_t<!std::is_void_v<required_load_context_t<U>>, required_load_context_t<U>,
                         void>>;
};
template <typename T, typename U>
struct required_store_context<std::pair<T, U>>
{
  using type = std::conditional_t<
      !std::is_void_v<required_store_context_t<T>>,
      required_store_context_t<T>,
      std::conditional_t<!std::is_void_v<required_store_context_t<U>>, required_store_context_t<U>,
                         void>>;
};

// std::map - context requirement from key and value types
template <typename K, typename V>
struct required_load_context<std::map<K, V>>
{
  using type = std::conditional_t<
      !std::is_void_v<required_load_context_t<K>>,
      required_load_context_t<K>,
      std::conditional_t<!std::is_void_v<required_load_context_t<V>>, required_load_context_t<V>,
                         void>>;
};
template <typename K, typename V>
struct required_store_context<std::map<K, V>>
{
  using type = std::conditional_t<
      !std::is_void_v<required_store_context_t<K>>,
      required_store_context_t<K>,
      std::conditional_t<!std::is_void_v<required_store_context_t<V>>, required_store_context_t<V>,
                         void>>;
};

// std::unordered_map
template <typename K, typename V>
struct required_load_context<std::unordered_map<K, V>>
{
  using type = std::conditional_t<
      !std::is_void_v<required_load_context_t<K>>,
      required_load_context_t<K>,
      std::conditional_t<!std::is_void_v<required_load_context_t<V>>, required_load_context_t<V>,
                         void>>;
};
template <typename K, typename V>
struct required_store_context<std::unordered_map<K, V>>
{
  using type = std::conditional_t<
      !std::is_void_v<required_store_context_t<K>>,
      required_store_context_t<K>,
      std::conditional_t<!std::is_void_v<required_store_context_t<V>>, required_store_context_t<V>,
                         void>>;
};

// std::shared_ptr
template <typename T>
struct required_load_context<std::shared_ptr<T>> : required_load_context<T>
{
};
template <typename T>
struct required_store_context<std::shared_ptr<T>> : required_store_context<T>
{
};

// std::unique_ptr
template <typename T>
struct required_load_context<std::unique_ptr<T>> : required_load_context<T>
{
};
template <typename T>
struct required_store_context<std::unique_ptr<T>> : required_store_context<T>
{
};

} // namespace Moose
