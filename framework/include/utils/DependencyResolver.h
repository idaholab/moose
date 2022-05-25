//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"
#include "MooseError.h"

#include "libmesh/utility.h"
#include "libmesh/simple_range.h"

// C++ includes
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>
#include <unordered_set>
#include <algorithm>
#include <sstream>
#include <exception>

template <typename T>
class DependencyResolverComparator
{
public:
  DependencyResolverComparator(const std::vector<T> & original_order)
    : _original_order(original_order)
  {
  }

  bool operator()(const T & a, const T & b) const
  {
    auto a_it = std::find(_original_order.begin(), _original_order.end(), a);
    auto b_it = std::find(_original_order.begin(), _original_order.end(), b);

    mooseAssert(a_it != _original_order.end(), "Bad DependencyResolverComparator request");
    mooseAssert(b_it != _original_order.end(), "Bad DependencyResolverComparator request");

    /**
     * Compare the iterators based on their original ordering.
     */
    return a_it < b_it;
  }

private:
  const std::vector<T> & _original_order;
};

/**
 * Class that represents the dependecy as a graph
 */
template <typename T>
class DependencyResolver
{
public:
  /**
   * Add a node 'a' to the graph
   */
  void addNode(const T & a)
  {
    if (_adj.find(a) == _adj.end())
      _adj[a] = {};

    if (_inv_adj.find(a) == _inv_adj.end())
      _inv_adj[a] = {};
  }

  /**
   * Add an edge between nodes 'a' and 'b'
   */
  void addEdge(const T & a, const T & b)
  {
    addNode(a);
    addNode(b);

    _adj[a].push_back(b);
    _inv_adj[b].push_back(a);
  }

  /**
   * Return true, if the grpah has a cycle, otherwise false
   */
  bool isCyclic()
  {
    _visited.clear();
    _rec_stack.clear();

    // mark all nodes as not visited and not part of recursion stack
    for (auto & n : _adj)
    {
      _visited[n.first] = false;
      _rec_stack[n.first] = false;
    }

    // detect cycle for all nodes
    for (auto & i : _adj)
      if (isCyclicHelper(i.first))
        return true;

    return false;
  }

  /**
   * Do depth-first search from root nodes to obtain order in which graph nodes should be
   * "executed".
   */
  typename std::vector<T> dfs()
  {
    _sorted_vector.clear();

    for (auto & n : _adj)
      _visited[n.first] = false;

    for (auto & n : _adj)
    {
      if (n.second.size() == 0)
        dfsFromNode(n.first);
    }

    return _sorted_vector;
  }

  typename std::map<T, bool> recStack() const { return _rec_stack; }

protected:
  /**
   * depth first search from a root node
   * @param root The node we start from
   */
  void dfsFromNode(const T & root)
  {
    _visited[root] = true;

    for (auto & i : _inv_adj[root])
    {
      if (!_visited.at(i))
        dfsFromNode(i);
    }

    _sorted_vector.push_back(root);
  }

  bool isCyclicHelper(const T & v)
  {
    if (!_visited[v])
    {
      _visited[v] = true;
      _rec_stack[v] = true;

      for (auto & i : _adj[v])
      {
        if (!_visited.at(i) && isCyclicHelper(i))
          return true;
        else if (_rec_stack.at(i))
          return true;
      }
    }
    _rec_stack[v] = false;
    return false;
  }

  /// adjacency lists (from leaves to roots)
  std::map<T, std::list<T>> _adj;
  /// adjacency lists (from roots to leaves)
  std::map<T, std::list<T>> _inv_adj;
  /// vector of visited nodes
  std::map<T, bool> _visited;
  /// recursive stack
  std::map<T, bool> _rec_stack;
  /// "sorted" vector of nodes
  std::vector<T> _sorted_vector;
};

template <typename T>
class DependencyResolver
{
public:
  DependencyResolver() {}

  ~DependencyResolver() {}

  /**
   * Insert a dependency pair - the first value or the "key" depends on the second value or the
   * "value"
   */
  void insertDependency(const T & key, const T & value);

  /**
   * Delete a dependency (only the edge) between items in the resolver. If either item is orphaned
   * due to the deletion of the edge, the items are inserted into the independent items set so they
   * will still come out when running the resolver.
   */
  void deleteDependency(const T & key, const T & value);

  /**
   * Removes dependencies of the given key. Does not fixup the graph or change indpendent items.
   */
  void deleteDependenciesOfKey(const T & key);

  /**
   * Add an independent item to the set
   */
  void addItem(const T & value);

  /**
   * Clear Items from the resolver
   */
  void clear();

  /**
   * Returns a vector of sets that represent dependency resolved values.  Items in the same
   * subvector have no dependence upon one and other.
   */
  const std::vector<std::vector<T>> & getSortedValuesSets();

  /**
   * This function also returns dependency resolved values but with a simpler single vector
   * interface.
   * Some information may be lost as values at the same level that don't depend on one and other
   * can't
   * be represented in a single vector.  This isn't a problem in practice though.
   */
  const std::vector<T> & getSortedValues();

  /**
   * Return true if key depends on value.
   * That is, return true, if a chain of calls of the form
   * insertDependency(key, v0)
   * insertDependency(v0, v1)
   * insertDependency(v1, v2)
   * ...
   * insertDependency(vN, value)
   * has been performed.
   * dependsOn(x, x) always returns true
   */
  bool dependsOn(const T & key, const T & value);

  /**
   * Return true if any of elements of keys depends on value
   */
  bool dependsOn(const std::vector<T> & keys, const T & value);

  /**
   * Returns a list of all values that a given key depends on
   */
  std::list<T> getAncestors(const T & key);

  /**
   * Returns the number of unique items stored in the dependency resolver.
   */
  std::size_t size() const;

  bool operator()(const T & a, const T & b);

  /**
   * Enumeration used to implement the coloring-based depth-first search (DFS) algorithm described
   * in https://www.geeksforgeeks.org/detect-cycle-direct-graph-using-colors/. The algorithm
   * searches for back-edges. A back-edge is an edge that points from a node to itself or from one
   * of a node's descendents (determined through DFS) to itself. The meaning of the colors used in
   * this algorithm are:
   * WHITE : Node is not processed yet. Initially, all nodes are WHITE.
   * GRAY: Node is being processed (DFS for this node has started, but not finished which means that
   * all descendants (in DFS tree) of this node are not processed yet (or this node is in the
   * function call stack)
   * BLACK : Node and all its descendants are processed.
   */
  enum Color
  {
    BLACK,
    WHITE,
    GRAY
  };

private:
  /**
   * Indicates whether any cyclic dependency is detected when descending a directed graph from this
   * node. This is a depth-first search (DFS) algorithm based on
   * https://www.geeksforgeeks.org/detect-cycle-direct-graph-using-colors/
   * @param node The node in the directed graph from which to begin descent
   * @return whether any cyclic dependency was discovered while descending from the node
   */
  bool hasCycle(const T & node);

  /**
   * Helper classes for returning only keys or values in an iterator format
   */
  template <typename map_type>
  class key_iterator;

  template <typename map_type>
  class value_iterator;

  /// This is our main data structure a multimap that contains any number of dependencies in a key = value format
  std::multimap<T, T> _depends;

  /// Used to avoid duplicate tracking of identical insertions of dependencies
  std::set<std::pair<T, T>> _unique_deps;

  /// Extra items that need to come out in the sorted list but contain no dependencies
  std::vector<T> _independent_items;

  // A vector retaining the order in which items were added to the
  // resolver, to disambiguate ordering of items with no
  // mutual interdependencies
  std::vector<T> _ordering_vector;

  /// A map used to implement the depth-first search algorithm, based on
  /// https://www.geeksforgeeks.org/detect-cycle-direct-graph-using-colors/, for use in detecting
  /// cyclic dependencies
  std::map<T, Color> _colors;

  /// The sorted vector of sets
  std::vector<std::vector<T>> _ordered_items;

  /// The sorted vector (if requested)
  std::vector<T> _ordered_items_vector;
};

template <typename T>
class CyclicDependencyException : public std::runtime_error
{
public:
  CyclicDependencyException(const std::string & error,
                            const std::multimap<T, T> & cyclic_items) throw()
    : runtime_error(error), _cyclic_items(cyclic_items)
  {
  }

  CyclicDependencyException(const CyclicDependencyException & e) throw()
    : runtime_error(e), _cyclic_items(e._cyclic_items)
  {
  }

  ~CyclicDependencyException() throw() {}

  const std::multimap<T, T> & getCyclicDependencies() const { return _cyclic_items; }

private:
  std::multimap<T, T> _cyclic_items;
};

/**
 * Helper class definitions
 */
template <typename T>
template <typename map_type>
class DependencyResolver<T>::key_iterator : public map_type::iterator
{
public:
  typedef typename map_type::iterator map_iterator;
  typedef typename map_iterator::value_type::first_type key_type;

  key_iterator(const map_iterator & other) : map_type::iterator(other){};

  key_type & operator*() { return map_type::iterator::operator*().first; }
};

template <typename T>
template <typename map_type>
class DependencyResolver<T>::value_iterator : public map_type::iterator
{
public:
  typedef typename map_type::iterator map_iterator;
  typedef typename map_iterator::value_type::second_type value_type;

  value_iterator(const map_iterator & other) : map_type::iterator(other){};

  value_type & operator*() { return map_type::iterator::operator*().second; }
};

/**
 * DependencyResolver class definitions
 */
template <typename T>
void
DependencyResolver<T>::insertDependency(const T & key, const T & value)
{
  auto k = std::make_pair(key, value);
  if (_unique_deps.count(k) > 0)
    return;
  _unique_deps.insert(k);
  auto insert_it = _depends.insert(k);

  for (auto & pr : _colors)
    pr.second = WHITE;
  _colors.emplace(key, WHITE);
  _colors.emplace(value, WHITE);
  if (hasCycle(key))
  {
    decltype(_depends) depends_copy(_depends);
    _depends.erase(insert_it);

    throw CyclicDependencyException<T>(
        "DependencyResolver: attempt to insert dependency will result in cyclic graph",
        depends_copy);
  }
  if (std::find(_ordering_vector.begin(), _ordering_vector.end(), key) == _ordering_vector.end())
    _ordering_vector.push_back(key);
  if (std::find(_ordering_vector.begin(), _ordering_vector.end(), value) == _ordering_vector.end())
    _ordering_vector.push_back(value);
}

template <typename T>
bool
DependencyResolver<T>::hasCycle(const T & root)
{
  // Catch as reference since we'll modify this value
  auto & root_color = libmesh_map_find(_colors, root);
  root_color = GRAY;

  auto [first, last] = _depends.equal_range(root);
  for (auto & val : as_range(first, last))
  {
    const auto & adj = val.second;
    // We'll copy since its a builtin and we don't need to modify this value
    const auto color_adj = libmesh_map_find(_colors, adj);

    if (color_adj == GRAY)
      // We're in the act of processing this node which means that we've now encountered this node
      // twice during descent and we have a cycle
      return true;

    // We are not currently processing (GRAY) nor have we already processed (BLACK) the adjacent
    // node, so we continue to descend
    if (color_adj == WHITE && hasCycle(adj))
      return true;
  }

  // We did not encounter any cycles while descending from this node
  root_color = BLACK;

  return false;
}

template <typename T>
void
DependencyResolver<T>::deleteDependency(const T & key, const T & value)
{
  std::pair<const int, int> k = std::make_pair(key, value);
  _unique_deps.erase(k);

  // We don't want to remove every entry in the multimap with this key. We need to find the exact
  // entry (e.g. the key/value pair).
  auto eq_range = _depends.equal_range(key);
  for (auto it = eq_range.first; it != eq_range.second; ++it)
    if (*it == k)
    {
      _depends.erase(it);
      break;
    }

  // Now that we've removed the dependency, we need to see if either one of the items is orphaned.
  // If it is, we'll need to add those items to the independent set.
  if (_depends.find(key) == _depends.end())
    addItem(key);

  bool found = false;
  for (auto pair_it : _depends)
    if (pair_it.second == value)
    {
      found = true;
      break;
    }

  if (!found)
    addItem(value);
}

template <typename T>
void
DependencyResolver<T>::deleteDependenciesOfKey(const T & key)
{
  auto eq_range = _depends.equal_range(key);
  _depends.erase(eq_range.first, eq_range.second);
}

template <typename T>
void
DependencyResolver<T>::addItem(const T & value)
{
  if (std::find(_independent_items.begin(), _independent_items.end(), value) ==
      _independent_items.end())
    _independent_items.push_back(value);
  if (std::find(_ordering_vector.begin(), _ordering_vector.end(), value) == _ordering_vector.end())
    _ordering_vector.push_back(value);
}

template <typename T>
void
DependencyResolver<T>::clear()
{
  _depends.clear();
  _independent_items.clear();
  _ordering_vector.clear();
  _ordered_items.clear();
  _ordered_items_vector.clear();
}

template <typename T>
const std::vector<std::vector<T>> &
DependencyResolver<T>::getSortedValuesSets()
{
  // Use the original ordering for ordering subvectors
  DependencyResolverComparator<T> comp(_ordering_vector);

  /**
   * Make a copy of the map to work on since:
   * 1) we will remove values from the map
   * 2) We need the copy to be sorted in an unambiguous order.
   */
  typedef std::multimap<T, T, DependencyResolverComparator<T>> dep_multimap;
  dep_multimap depends(_depends.begin(), _depends.end(), comp);

  // Build up a set of all keys in depends that have nothing depending on them,
  // and put it in the orphans set.
  std::set<T> nodepends;

  std::set<T> all;
  std::set<T> dependees;
  for (auto & entry : depends)
  {
    dependees.insert(entry.second);
    all.insert(entry.first);
    all.insert(entry.second);
  }

  std::set<T> orphans;
  std::set_difference(all.begin(),
                      all.end(),
                      dependees.begin(),
                      dependees.end(),
                      std::inserter(orphans, orphans.end()));

  // Remove items from _independent_items if they actually appear in depends
  for (auto siter = _independent_items.begin(); siter != _independent_items.end();)
  {
    T key = *siter;
    bool founditem = false;
    for (auto i2 : depends)
    {
      if (i2.first == key || i2.second == key)
      {
        founditem = true;
        break;
      }
    }
    if (founditem)
      siter = _independent_items.erase(siter); // post increment to maintain a valid iterator
    else
      ++siter;
  }

  /* Clear the ordered items vector */
  _ordered_items.clear();

  // Put the independent items into the first set in _ordered_items
  std::vector<T> next_set(_independent_items.begin(), _independent_items.end());

  /* Topological Sort */
  while (!depends.empty())
  {
    /* Work with sets since set_difference doesn't always work properly with multi_map due
     * to duplicate keys
     */
    std::set<T, DependencyResolverComparator<T>> keys(
        typename DependencyResolver<T>::template key_iterator<dep_multimap>(depends.begin()),
        typename DependencyResolver<T>::template key_iterator<dep_multimap>(depends.end()),
        comp);

    std::set<T, DependencyResolverComparator<T>> values(
        typename DependencyResolver<T>::template value_iterator<dep_multimap>(depends.begin()),
        typename DependencyResolver<T>::template value_iterator<dep_multimap>(depends.end()),
        comp);

    std::vector<T> current_set(next_set);
    next_set.clear();

    /* This set difference creates a set of items that have no dependencies in the depend map*/
    std::set<T, DependencyResolverComparator<T>> difference(comp);

    std::set_difference(values.begin(),
                        values.end(),
                        keys.begin(),
                        keys.end(),
                        std::inserter(difference, difference.end()),
                        comp);

    /* Now remove items from the temporary map that have been "resolved" */
    if (!difference.empty())
    {
      for (auto iter = depends.begin(); iter != depends.end();)
      {
        if (difference.find(iter->second) != difference.end())
        {
          T key = iter->first;
          depends.erase(iter++); // post increment to maintain a valid iterator

          // If the item is at the end of a dependency chain (by being an orphan) AND
          // is not still in the depends map because it still has another unresolved link
          // insert it into the next_set
          if (orphans.find(key) != orphans.end() && depends.find(key) == depends.end())
            next_set.push_back(key);
        }
        else
          ++iter;
      }
      /* Add the current set of resolved items to the ordered vector */
      current_set.insert(current_set.end(), difference.begin(), difference.end());
      _ordered_items.push_back(current_set);
    }
    else
    {

      /* If the last set difference was empty but there are still items that haven't come out then
       * there is a cyclic dependency somewhere in the map.
       */
      if (!depends.empty())
      {
        std::ostringstream oss;
        oss << "Cyclic dependency detected in the Dependency Resolver.  Remaining items are:\n";
        for (auto j : depends)
          oss << j.first << " -> " << j.second << "\n";
        // Return a multimap without a weird comparator, to avoid
        // dangling reference problems and for backwards compatibility
        std::multimap<T, T> cyclic_deps(depends.begin(), depends.end());
        throw CyclicDependencyException<T>(oss.str(), cyclic_deps);
      }
    }
  }

  if (next_set.empty())
  {
    if (!_independent_items.empty() || !depends.empty())
      mooseError("DependencyResolver error: next_set shouldn't be empty!");
  }
  else
  {
    _ordered_items.push_back(next_set);
  }

  return _ordered_items;
}

template <typename T>
const std::vector<T> &
DependencyResolver<T>::getSortedValues()
{
  _ordered_items_vector.clear();

  getSortedValuesSets();

  for (auto subset : _ordered_items)
    std::copy(subset.begin(), subset.end(), std::back_inserter(_ordered_items_vector));

  return _ordered_items_vector;
}

template <typename T>
bool
DependencyResolver<T>::dependsOn(const T & key, const T & value)
{
  if (key == value)
    return true;

  // recursively call dependsOn on all the things that key depends on
  auto ret = _depends.equal_range(key);
  for (auto it = ret.first; it != ret.second; ++it)
    if (dependsOn(it->second, value))
      return true;

  // No dependencies were found,
  // or the key is not in the tree (so it has no dependencies).
  // In this latter case, the only way that key depends on value is if key == value,
  // but we've already checked that
  return false;
}

template <typename T>
bool
DependencyResolver<T>::dependsOn(const std::vector<T> & keys, const T & value)
{
  for (auto key : keys)
    if (dependsOn(key, value))
      return true;
  return false;
}

template <typename T>
std::list<T>
DependencyResolver<T>::getAncestors(const T & key)
{
  std::list<T> ancestors = {key};
  std::unordered_set<T> unique_values;

  auto it = ancestors.begin();
  while (it != ancestors.end())
  {
    auto ret = _depends.equal_range(*it);

    for (auto it_range = ret.first; it_range != ret.second; ++it_range)
    {
      auto & item = it_range->second;
      if (unique_values.find(item) == unique_values.end())
      {
        ancestors.push_back(item);
        unique_values.insert(item);
      }
    }

    ++it;
  }

  return ancestors;
}

template <typename T>
std::size_t
DependencyResolver<T>::size() const
{
  return _ordering_vector.size();
}

template <typename T>
bool
DependencyResolver<T>::operator()(const T & a, const T & b)
{
  if (_ordered_items_vector.empty())
    getSortedValues();

  auto a_it = std::find(_ordered_items_vector.begin(), _ordered_items_vector.end(), a);
  auto b_it = std::find(_ordered_items_vector.begin(), _ordered_items_vector.end(), b);

  /**
   * It's possible that a and/or b are not in the resolver in which case
   *  we want those values to come out first.  However, we need to make
   *  sure that we maintain strict weak ordering so we'll compare b_it first,
   *  which will return false for a_it < b_it and b_it < a_it when both values
   *  are not in the ordered_items vector.
   */
  if (b_it == _ordered_items_vector.end())
    return false;
  if (a_it == _ordered_items_vector.end())
    return true;
  else
    /**
     * Compare the iterators.  Users sometime fail to state all their
     * items' dependencies, but do introduce dependant items only after
     * the items they depended on; this preserves that sorting.
     */
    return a_it < b_it;
}
