//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <unordered_map>
#include <mutex>
#include <deque>

#include "MooseError.h"

template <class Key, class Item, class KeyHash = std::hash<Key>>
class GeneralRegistry
{
public:
  GeneralRegistry(const std::string & name);

  /**
   * @returns The number of registered items
   */
  std::size_t size() const;

  /**
   * @returns The ID assocated with the key \p key
   */
  std::size_t id(const Key & key) const;

  /**
   * @returns Whether or not the key \p key is registered
   */
  bool keyExists(const Key & key) const;
  /**
   * @returns Whether or not the id \p id is registered
   */
  bool idExists(const std::size_t id) const;

  /**
   * @returns The item associated with the key \p key (thread safe)
   */
  const Item & item(const std::size_t id) const;

protected:
  /**
   * @returns The item associated with the key \p key (not thread safe)
   */
  const Item & itemNonLocking(const std::size_t id) const;

  /**
   * Registers an item with key \p key if said key does not exist.
   *
   * @param key The key
   * @param create_item Lambda called to create an item if the key
   * does not exist. Takes a single argument std::size_t which is the
   # new ID and should return an Item
   * @returns The ID of the item
   */
  template <typename CreateItem>
  std::size_t registerItem(const Key & key, CreateItem & create_item);

  /// The name of this registry; used in error handling
  const std::string _name;

  /// Map of keys to IDs
  std::unordered_map<Key, std::size_t, KeyHash> _key_to_id;
  /// Vector of IDs to Items
  std::deque<Item> _id_to_item;

  /// Mutex for locking access to _key_to_id
  /// NOTE: These can be changed to shared_mutexes once we get C++17
  mutable std::mutex _key_to_id_mutex;
  /// Mutex for locking access to _id_to_item
  /// NOTE: These can be changed to shared_mutexes once we get C++17
  mutable std::mutex _id_to_item_mutex;
};

template <class Key, class Item, class KeyHash>
GeneralRegistry<Key, Item, KeyHash>::GeneralRegistry(const std::string & name) : _name(name)
{
}

template <class Key, class Item, class KeyHash>
std::size_t
GeneralRegistry<Key, Item, KeyHash>::size() const
{
  std::lock_guard<std::mutex> lock(_id_to_item_mutex);
  return _id_to_item.size();
}

template <class Key, class Item, class KeyHash>
std::size_t
GeneralRegistry<Key, Item, KeyHash>::id(const Key & key) const
{
  std::lock_guard<std::mutex> lock(_key_to_id_mutex);
  const auto it = _key_to_id.find(key);
  if (it == _key_to_id.end())
    mooseError(_name, ": Key '", key, "' is not registered");
  return it->second;
}

template <class Key, class Item, class KeyHash>
bool
GeneralRegistry<Key, Item, KeyHash>::keyExists(const Key & key) const
{
  std::lock_guard<std::mutex> lock(_key_to_id_mutex);
  return _key_to_id.count(key);
}

template <class Key, class Item, class KeyHash>
bool
GeneralRegistry<Key, Item, KeyHash>::idExists(const std::size_t id) const
{
  std::lock_guard<std::mutex> lock(_id_to_item_mutex);
  return id < _id_to_item.size();
}

template <class Key, class Item, class KeyHash>
const Item &
GeneralRegistry<Key, Item, KeyHash>::item(const std::size_t id) const
{
  std::lock_guard<std::mutex> lock(_id_to_item_mutex);
  return itemNonLocking(id);
}

template <class Key, class Item, class KeyHash>
const Item &
GeneralRegistry<Key, Item, KeyHash>::itemNonLocking(const std::size_t id) const
{
  if (id >= _id_to_item.size())
    mooseError(_name, ": ID '", id, "' is not registered");
  return _id_to_item[id];
}

template <class Key, class Item, class KeyHash>
template <typename CreateItem>
std::size_t
GeneralRegistry<Key, Item, KeyHash>::registerItem(const Key & key, CreateItem & create_item)
{
  std::lock_guard<std::mutex> lock_key(_key_to_id_mutex);

  // Is it already registered?
  const auto it = _key_to_id.find(key);
  if (it != _key_to_id.end())
    return it->second;

  // It's not registered
  std::lock_guard<std::mutex> lock_id(_id_to_item_mutex);
  const auto id = _id_to_item.size();
  _key_to_id.emplace(key, id);
  _id_to_item.emplace_back(std::move(create_item(id)));
  return id;
}
