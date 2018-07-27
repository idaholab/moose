//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef THEWAREHOUSE_H
#define THEWAREHOUSE_H

#include <map>
#include <string>
#include <vector>
#include <type_traits>
#include <unordered_map>
#include <iostream>

#include "MooseObject.h"

class MooseObject;
class Storage;
class TheWarehouse;

/// Attribute is an abstract class that can be implemented in order to track custom metadata about
/// MooseObject instances - enabling warehouse queries over the attribute.  Attribute subclasses
/// must be registered with the warehouse (i.e. via TheWarehouse::registerAttrib) where they will
/// be used *before* objects are added to that warehouse.  Specific Attribute instances cannot (and
/// should not) generally be created before the class is registered with a warehouse.
class Attribute
{
public:
  /// Constructs/initializes a new attribute with the specified name for use in warehouse w.  The
  /// attribute must have been previously registered with w prior to calling this constructor.
  Attribute(TheWarehouse & w, const std::string name);
  virtual ~Attribute() {}

  inline bool operator==(const Attribute & other) const
  {
    return _id == other._id && isMatch(other);
  }
  inline bool operator!=(const Attribute & other) const { return !(*this == other); }
  inline bool operator<(const Attribute & other) const
  {
    return _id == other._id ? isLess(other) : _id < other._id;
  }

  /// returns the unique attribute ID associated with all attributes that have the same (mose
  /// derived) class as this object. This ID is determined at construction time
  /// this
  inline unsigned int id() const { return _id; }

  /// initFrom reads and stores the desired meta-data from obj for later matching comparisons.
  virtual void initFrom(const MooseObject * obj) = 0;
  /// isMatch returns true if the meta-data stored in this attribute has the same value as that
  /// stored in other. isMatch does not need to check/compare the values from the instances' id()
  /// functions.
  virtual bool isMatch(const Attribute & other) const = 0;
  /// isLess should return true if the meta-data stored in this object is "less" than that stored
  /// in other (i.e. like an operator< overload). This comparison can be implemented in any way
  /// that makes sense for the type of data being stored.  "attrib1.isLess(attrib2)" and
  /// "attrib2.isLess(attrib1)" should never both be true, and they should only both return false
  /// if and only if "attrib1.isMatch(attrib2)" returns true.  isLess does not need to
  /// check/compare the values from the instances' id() functions.
  virtual bool isLess(const Attribute & other) const = 0;
  /// clone creates and returns and identical (deep) copy of this attribute - i.e. the result of
  /// clone should return true if passed into isMatch.
  virtual std::unique_ptr<Attribute> clone() const = 0;

private:
  int _id = -1;
};

/// TheWarehouse uses this operator function for indexing and caching queries. So this is
/// important even though you don't see it being called (directly) anywhere - it *IS* being used.
bool operator<(const std::unique_ptr<Attribute> & lhs, const std::unique_ptr<Attribute> & rhs);

/// TheWarehouse is a container for MooseObjects that allows querying/filtering over various
/// customizeable attributes.  The meta-data about the objects is read/stored when the objects are
/// added to the warehouse - updates to objects' state will not be reflected in query
/// results unless the object is explicitly updated through the warehouse interface.  The
/// warehouse object can safely be queried concurrently from multiple threads.
///
/// Once Query and Attribute objects have been constructed, they are tied to the specific
/// warehouse they were created with.  They must not be used for different warehouses or the
/// attribute ID they store internally will be wrong and that is bad.
class TheWarehouse
{
public:
  /// Query is a convenient way to construct and pass around (possible partially constructed)
  /// warehouse queries.  The warehouse's "build()" function should generally be used to create
  /// new Query objects rather than constructing them directly.  A Query object holds a list of
  /// conditions used to filter/select objects from the warehouse.  When the query is
  /// executed/run, results are filtered by "and"ing each condition together - i.e. only objects
  /// that match *all* conditions are returned.
  class Query
  {
  public:
    /// Creates a new query operating on the given warehouse w.  You should generally use
    /// TheWarehouse::build() instead.
    Query(TheWarehouse & w) : _w(&w) {}

    Query & operator=(const Query & other)
    {
      if (this == &other)
        return *this;

      _w = other._w;
      _attribs.clear();
      for (auto & attrib : other._attribs)
        _attribs.push_back(attrib->clone());
      return *this;
    }
    Query(const Query & other) : _w(other._w)
    {
      for (auto & attrib : other._attribs)
        _attribs.push_back(attrib->clone());
    }

    /// Adds one or more new conditions to the query.  Each arg must be an Attribute object of the
    /// desired type+value.
    template <typename... Args>
    Query & conds(const Attribute & attrib, Args... args)
    {
      _attribs.push_back(attrib.clone());
      return cond(args...);
    }

    /// Adds a new condition to the query.  The template parameter T is the Attribute class of
    /// interest and args are forwarded to T's constructor to build+add the attribute in-situ.
    template <typename T, typename... Args>
    Query & cond(Args... args)
    {
      _attribs.push_back(std::unique_ptr<Attribute>(new T(*_w, args...)));
      return *this;
    }

    /// clone creates and returns an independent copy of the query in its current state.
    Query clone() const { return std::move(Query(*this)); }
    /// count returns the number of results that match the query (this requires actually running
    /// the query).
    size_t count() { return _w->count(std::move(clone()._attribs)); }

    /// attribs returns a copy of the constructed Attribute list for the query in its current state.
    std::vector<std::unique_ptr<Attribute>> attribs() { return std::move(clone()._attribs); }

    /// queryInto executes the query and stores the results in the given vector.  All results must
    /// be castable to the templated type T.
    template <typename T>
    std::vector<T *> queryInto(std::vector<T *> & results)
    {
      return _w->queryInto(std::move(clone()._attribs), results);
    }

  private:
    Query & cond(const Attribute & attrib)
    {
      _attribs.push_back(attrib.clone());
      return *this;
    }

    TheWarehouse * _w = nullptr;
    std::vector<std::unique_ptr<Attribute>> _attribs;
  };

  TheWarehouse();
  ~TheWarehouse();

  /// registers a new "tracked" attribute of type T for the warehouse.  args are all arguments
  /// necessary to create an instance of the T class excluding the warehouse reference/pointer
  /// which is assumed to be first and automatically inserted.  An instance of every registered
  /// attribute will be created for and initialized to each object added to the warehouse allowing
  /// queries to be executed over specific values the attribute may take on. Attributes must be
  /// registered *before* objects are added to the warehouse.  A unique ID associated with the
  /// registered attribute is returned - which is generally not needed used by users.
  ///
  /// As an example, to register a class with the constructor "YourAttribute(TheWarehouse& w, int
  /// foo)", you would call "registerAttrib<YourAttribute>("your_attrib_name", 42 /* a dummy-value
  /// */)". Custom attribute classes are required to pass an attribute name (i.e.
  /// "your_attrib_name") to the Attribute base class.  The name passed here into registerAttrib
  /// must be the same string as the name passed to the Attribute base class's constructor.
  template <typename T, typename... Args>
  unsigned int registerAttrib(const std::string & name, Args... args)
  {
    auto it = _attrib_ids.find(name);
    if (it != _attrib_ids.end())
      return it->second;

    _attrib_ids[name] = _attrib_list.size();
    _attrib_list.push_back(std::unique_ptr<Attribute>(new T(*this, args...)));
    return _attrib_list.size() - 1;
  }

  /// Returns a unique ID associated with the given attribute name - i.e. an attribute and name
  /// that were previously registered via calls to registerAttrib.  Users should generally *not*
  /// need to use this function.
  unsigned int attribID(const std::string & name);

  /// add adds a new object to the warehouse and stores attributes/metadata about it for running
  /// queries/filtering.  The warehouse will maintain a pointer to the object indefinitely.
  void add(std::shared_ptr<MooseObject> obj, const std::string & system);
  /// update updates the metadata/attribute-info stored for the given object obj that must already
  /// exists in the warehouse.  Call this if an object's state has changed in such a way that its
  /// warehouse attributes have become stale/incorrect.
  void update(MooseObject * obj);
  /// update updates the metadata/attribute-info stored for the given object obj that must already
  /// exists in the warehouse.  Call this if an object's state has changed in such a way that its
  /// warehouse attributes have become stale/incorrect.
  /// Any attribute specified in extra overwrites/trumps one read from the object's current state.
  void update(MooseObject * obj, const Attribute & extra);
  /// build creates and returns an initialized a query object for querying objects from the
  /// warehouse.
  Query build() { return Query(*this); }
  /// count returns the number of objects that match the provided query conditions. This requires
  /// executing a full query operation (i.e. as if calling queryInto). A Query object should
  /// generally be used via the build() member function instead.
  size_t count(std::vector<std::unique_ptr<Attribute>> conds);
  /// queryInto takes the given conditions (i.e. Attributes holding the values to filter/match
  /// over) and filters all objects in the warehouse that match all conditions (i.e. "and"ing the
  /// conditions together) and stores them in the results vector. All result objects must be
  /// castable to the templated type T. This function filters out disabled objects on the fly -
  /// only returning enabled ones.
  template <typename T>
  std::vector<T *> & queryInto(std::vector<std::unique_ptr<Attribute>> conds,
                               std::vector<T *> & results)
  {
    int query_id = -1;
    auto it = _query_cache.find(conds);
    if (it == _query_cache.end())
      query_id = prepare(std::move(conds));
    else
      query_id = it->second;
    return queryInto(query_id, results);
  }

private:
  template <typename T>
  std::vector<T *> & queryInto(int query_id, std::vector<T *> & results, bool show_all = false)
  {
    auto objs = query(query_id);
    results.resize(0);
    for (unsigned int i = 0; i < objs.size(); i++)
    {
      auto obj = objs[i];
      assert(dynamic_cast<T *>(obj));
      if (show_all || obj->enabled())
        results.push_back(dynamic_cast<T *>(obj));
    }
    return results;
  }

  /// prepares a query and returns an associated query_id (i.e. for use with the query function).
  int prepare(std::vector<std::unique_ptr<Attribute>> conds);

  const std::vector<MooseObject *> query(int query_id);

  void readAttribs(const MooseObject * obj,
                   const std::string & system,
                   std::vector<std::unique_ptr<Attribute>> & attribs);

  std::unique_ptr<Storage> _store;
  std::vector<std::shared_ptr<MooseObject>> _objects;
  std::unordered_map<MooseObject *, size_t> _obj_ids;

  // Results from queries are cached here. The outer vector index is the query id as stored by the
  // _query_cache data structure.  A list objects that match each query id are stored.
  std::vector<std::vector<MooseObject *>> _obj_cache;
  // This stores a query id for every query keyed by the query conditions/attributes.
  // User-initiated queries check this map to see if a queries results have already been cached.
  // The query id is an index into the _obj_cache data structure.
  std::map<std::vector<std::unique_ptr<Attribute>>, int> _query_cache;

  std::map<std::string, unsigned int> _attrib_ids;
  std::vector<std::unique_ptr<Attribute>> _attrib_list;
};

#endif // THEWAREHOUSE_H
