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
class CyclicDependencyException;

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
  DependencyResolver() = default;
  ~DependencyResolver() = default;

  /**
   * Add a node 'a' to the graph
   */
  void addNode(const T & a)
  {
#ifndef NDEBUG
    bool new_adj_insertion = false, new_inv_insertion = false;
#endif
    if (_adj.find(a) == _adj.end())
    {
#ifndef NDEBUG
      new_adj_insertion = true;
#endif
      _adj[a] = {};
      _insertion_order.push_front(a);
    }

    if (_inv_adj.find(a) == _inv_adj.end())
    {
#ifndef NDEBUG
      new_inv_insertion = true;
#endif
      _inv_adj[a] = {};
    }
    mooseAssert(new_adj_insertion == new_inv_insertion,
                "We should have symmetric behavior between adjacent and inverse-adjacent "
                "insertion/non-insertion.");
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
   * Remove an edge between nodes 'a' and 'b'
   */
  void removeEdge(const T & a, const T & b)
  {
    auto remove_item = [](auto & list, const auto & item)
    {
      auto it = std::find(list.begin(), list.end(), item);
      mooseAssert(it != list.end(), "We should have this item");
      list.erase(it);
    };
    remove_item(_adj[a], b);
    remove_item(_inv_adj[b], a);
  }

  /**
   * Remove edges drawn from 'a'
   */
  void removeEdgesInvolving(const T & a)
  {
    const auto & inv_adjs = _inv_adj[a];
    for (const auto & inv_adj : inv_adjs)
    {
      auto & adj = _adj[inv_adj];
      auto it = std::find(adj.begin(), adj.end(), a);
      mooseAssert(it != adj.end(), "Should have reciprocity");
      adj.erase(it);
    }

    _inv_adj[a].clear();
  }

  /**
   * Insert a dependency pair - the first value or the "key" depends on the second value or the
   * "value"
   */
  void insertDependency(const T & key, const T & value) { addEdge(value, key); }

  /**
   * Delete a dependency (only the edge) between items in the resolver
   */
  void deleteDependency(const T & key, const T & value) { removeEdge(value, key); }

  /**
   * Removes dependencies of the given key
   */
  void deleteDependenciesOfKey(const T & key) { removeEdgesInvolving(key); }

  /**
   * Add an independent item to the set
   */
  void addItem(const T & value) { addNode(value); }

  /**
   * Clear Items from the resolver
   */
  void clear()
  {
    _adj.clear();
    _inv_adj.clear();
    _insertion_order.clear();
  }

  /**
   * Do depth-first search from root nodes to obtain order in which graph nodes should be
   * "executed".
   */
  const std::vector<T> & dfs()
  {
    _sorted_vector.clear();
    _visited.clear();
    _rec_stack.clear();

    for (auto & n : _adj)
    {
      _visited[n.first] = false;
      _rec_stack[n.first] = false;
    }

    bool is_cyclic = false;
    // If there are no adjacencies, then all nodes are both roots and leaves
    bool roots_found = _adj.empty();
    for (auto & n : _insertion_order)
      if (_adj[n].size() == 0)
      {
        roots_found = true;
        is_cyclic = dfsFromNode(n);
        if (is_cyclic)
          break;
      }
    if (!roots_found)
      is_cyclic = true;

    if (is_cyclic)
      throw CyclicDependencyException<T>("cyclic graph detected", *this);

    return _sorted_vector;
  }

  const std::map<T, bool> & recStack() const { return _rec_stack; }

  /**
   * Returns a vector of sets that represent dependency resolved values.  Items in the same
   * subvector have no dependence upon one and other.
   */
  const std::vector<std::vector<T>> & getSortedValuesSets()
  {
    _ordered_items.clear();

    const auto & flat_sorted = dfs();

    std::vector<T> current_group;
    for (const auto & object : flat_sorted)
    {
      if (current_group.empty())
      {
        current_group.push_back(object);
        continue;
      }

      const auto & prev_adj_list = _adj[current_group.back()];
      const bool depends_on_prev =
          std::find(prev_adj_list.begin(), prev_adj_list.end(), object) != prev_adj_list.end();

      if (depends_on_prev)
      {
        _ordered_items.push_back({object});
        auto & finalized_group = _ordered_items.back();
        // Swap the current-group into the back of our ordered items container, and now our newest
        // object becomes the current group
        finalized_group.swap(current_group);
      }
      else
        current_group.push_back(object);
    }

    if (!current_group.empty())
      _ordered_items.push_back(std::move(current_group));

    return _ordered_items;
  }

  /**
   * This function also returns dependency resolved values but with a simpler single vector
   * interface.
   * Some information may be lost as values at the same level that don't depend on one and other
   * can't
   * be represented in a single vector.  This isn't a problem in practice though.
   */
  const std::vector<T> & getSortedValues() { return dfs(); }

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
  bool dependsOn(const T & key, const T & value)
  {
    for (auto & n : _adj)
      _visited[n.first] = false;

    return dependsOnFromNode(key, value);
  }

  /**
   * Return true if any of elements of keys depends on value
   */
  bool dependsOn(const std::vector<T> & keys, const T & value)
  {
    for (const auto & key : keys)
      if (dependsOnFromNode(key, value))
        return true;

    return false;
  }

  /**
   * Returns a list of all values that a given key depends on
   */
  std::list<T> getAncestors(const T & key)
  {
    std::vector<T> ret_vec;
    // Our sorted vector is our work vector but we also return references to it. So we have to make
    // sure at the end that we restore the original data we had in it
    ret_vec.swap(_sorted_vector);

    for (auto & n : _adj)
      _visited[n.first] = false;

    dfsFromNode(key);

    ret_vec.swap(_sorted_vector);
    mooseAssert(ret_vec.back() == key, "Our key should be the back of the vector");

    return {ret_vec.begin(), ret_vec.end()};
  }

  /**
   * Returns the number of unique items stored in the dependency resolver. lindsayad comment: does
   * it really return the number of *unique* items?
   */
  std::size_t size() const { return _sorted_vector.size(); }

  bool operator()(const T & a, const T & b);

protected:
  /**
   * depth first search from a root node for a specific item
   * @param root The node we start from
   * @param item The node we are searching for
   */
  bool dependsOnFromNode(const T & root, const T & item)
  {
    if (root == item)
      return true;

    _visited[root] = true;

    auto & my_dependencies = _inv_adj[root];

    for (auto & i : my_dependencies)
      if (!_visited.at(i) && dependsOnFromNode(i, item))
        return true;

    return false;
  }

  bool dfsFromNode(const T & root)
  {
    bool cyclic = false;
    _visited[root] = true;
    _rec_stack[root] = true;

    for (auto & i : _inv_adj[root])
    {
      if (!_visited.at(i) && dfsFromNode(i))
        cyclic = true;
      else if (_rec_stack.at(i))
        cyclic = true;
    }

    _sorted_vector.push_back(root);
    _rec_stack[root] = false;
    return cyclic;
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
  /// The sorted vector of sets
  std::vector<std::vector<T>> _ordered_items;
  /// Container for keeping track of the insertion order. We will use this to determine iteration
  /// order because it is essential that iteration order be sync'd across multiple
  /// processes. Iterating over maps with pointer keys, for example, can be out of sync on multiple
  /// processes. If dependency resolver memory usage shows up in profiling, we can consider making
  /// this a container of reference wrappers
  std::deque<T> _insertion_order;

  friend class CyclicDependencyException<T>;
};

template <typename T>
class CyclicDependencyException : public std::runtime_error
{
public:
  CyclicDependencyException(const std::string & error, const DependencyResolver<T> & graph) throw()
    : runtime_error(error), _cyclic_items(graph._adj)
  {
  }

  CyclicDependencyException(const std::string & error,
                            const std::map<T, std::list<T>> & cyclic_items) throw()
    : runtime_error(error), _cyclic_items(cyclic_items)
  {
  }

  CyclicDependencyException(const std::string & error,
                            std::map<T, std::list<T>> && cyclic_items) throw()
    : runtime_error(error), _cyclic_items(std::move(cyclic_items))
  {
  }

  CyclicDependencyException(const CyclicDependencyException & e) throw()
    : runtime_error(e), _cyclic_items(e._cyclic_items)
  {
  }

  ~CyclicDependencyException() throw() {}

  const std::map<T, std::list<T>> & getCyclicDependencies() const { return _cyclic_items; }

private:
  std::map<T, std::list<T>> _cyclic_items;
};

template <typename T>
bool
DependencyResolver<T>::operator()(const T & a, const T & b)
{
  if (_sorted_vector.empty())
    getSortedValues();

  auto a_it = std::find(_sorted_vector.begin(), _sorted_vector.end(), a);
  auto b_it = std::find(_sorted_vector.begin(), _sorted_vector.end(), b);

  /**
   * It's possible that a and/or b are not in the resolver in which case
   *  we want those values to come out first.  However, we need to make
   *  sure that we maintain strict weak ordering so we'll compare b_it first,
   *  which will return false for a_it < b_it and b_it < a_it when both values
   *  are not in the ordered_items vector.
   */
  if (b_it == _sorted_vector.end())
    return false;
  if (a_it == _sorted_vector.end())
    return true;
  else
    /**
     * Compare the iterators.  Users sometime fail to state all their
     * items' dependencies, but do introduce dependant items only after
     * the items they depended on; this preserves that sorting.
     */
    return a_it < b_it;
}
