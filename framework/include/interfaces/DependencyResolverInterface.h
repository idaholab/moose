//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// STL includes
#include <string>
#include <set>
#include <iostream>
#include <algorithm>

// MOOSE includes
#include "DependencyResolver.h"

/**
 * Interface for sorting dependent vectors of objects.
 */
class DependencyResolverInterface
{
public:
  /**
   * Constructor.
   */
  DependencyResolverInterface() {}

  /**
   * Return a set containing the names of items requested by the object.
   */
  virtual const std::set<std::string> & getRequestedItems() = 0;

  /**
   * Return a set containing the names of items owned by the object.
   */
  virtual const std::set<std::string> & getSuppliedItems() = 0;

  /**
   * Given a vector, sort using the getRequested/SuppliedItems sets.
   */
  template <typename T>
  static void sort(typename std::vector<T> & vector);

  /**
   * Given a vector, sort using the depth-first search
   */
  template <typename T>
  static void sortDFS(typename std::vector<T> & vector);

  /**
   * A helper method for cyclic errors.
   */
  template <typename T>
  static void cyclicDependencyError(CyclicDependencyException<T> & e, const std::string & header);
};

template <typename T>
void
DependencyResolverInterface::sort(typename std::vector<T> & vector)
{
  DependencyResolver<T> resolver;

  typename std::vector<T>::iterator start = vector.begin();
  typename std::vector<T>::iterator end = vector.end();

  for (typename std::vector<T>::iterator iter = start; iter != end; ++iter)
  {
    const std::set<std::string> & requested_items = (*iter)->getRequestedItems();

    for (typename std::vector<T>::iterator iter2 = start; iter2 != end; ++iter2)
    {
      if (iter == iter2)
        continue;

      const std::set<std::string> & supplied_items = (*iter2)->getSuppliedItems();

      std::set<std::string> intersect;
      std::set_intersection(requested_items.begin(),
                            requested_items.end(),
                            supplied_items.begin(),
                            supplied_items.end(),
                            std::inserter(intersect, intersect.end()));

      // If the intersection isn't empty then there is a dependency here
      if (!intersect.empty())
        resolver.insertDependency(*iter, *iter2);
    }
  }

  // Sort based on dependencies
  std::stable_sort(start, end, resolver);
}

template <typename T>
void
DependencyResolverInterface::sortDFS(typename std::vector<T> & vector)
{
  if (vector.size() <= 1)
    return;

  /**
   * Class that represents the dependecy as a graph
   */
  class Graph
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
    typename std::map<T, std::list<T>> _adj;
    /// adjacency lists (from roots to leaves)
    typename std::map<T, std::list<T>> _inv_adj;
    /// vector of visited nodes
    typename std::map<T, bool> _visited;
    /// recursive stack
    typename std::map<T, bool> _rec_stack;
    /// "sorted" vector of nodes
    typename std::vector<T> _sorted_vector;
  } graph;

  // Map of suppliers: what is supplied -> by what object
  typename std::map<std::string, T> suppliers_map;
  for (auto & v : vector)
  {
    for (auto & ri : v->getSuppliedItems())
    {
      suppliers_map[ri] = v;
      graph.addNode(v);
    }
  }

  // build the dependency graph
  for (auto & v : vector)
  {
    for (auto & ri : v->getRequestedItems())
    {
      const auto & ri_it = suppliers_map.find(ri);
      if (ri_it != suppliers_map.end())
        graph.addEdge(ri_it->second, v);
      else
        graph.addNode(v);
    }
  }

  if (graph.isCyclic())
  {
    std::ostringstream oss;
    oss << "Cyclic dependency detected in object ordering:" << std::endl;
    auto rec_stack = graph.recStack();
    auto first = rec_stack.begin();
    auto first_name = first->first->name();
    oss << first_name << " -> ";
    for (auto & it = ++first; it != rec_stack.end(); ++it)
    {
      oss << it->first->name() << std::endl;
      oss << it->first->name() << " -> ";
    }
    oss << first_name << std::endl;
    mooseError(oss.str());
  }
  else
    vector = graph.dfs();
}

template <typename T>
void
DependencyResolverInterface::cyclicDependencyError(CyclicDependencyException<T> & e,
                                                   const std::string & header)
{
  std::ostringstream oss;

  oss << header << ":\n";
  const typename std::multimap<T, T> & depends = e.getCyclicDependencies();
  for (typename std::multimap<T, T>::const_iterator it = depends.begin(); it != depends.end(); ++it)
    oss << (static_cast<T>(it->first))->name() << " -> " << (static_cast<T>(it->second))->name()
        << "\n";
  mooseError(oss.str());
}
