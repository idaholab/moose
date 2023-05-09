//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <mutex>

#include "MooseObject.h"
#include "MooseHashing.h"

class MooseObject;
class Storage;
class TheWarehouse;

/// Attribute is an abstract class that can be implemented in order to track custom metadata about
/// MooseObject instances - enabling warehouse queries over the attribute.  Attribute subclasses
/// must be registered with the warehouse (i.e. via TheWarehouse::registerAttribute) where they will
/// be used *before* objects are added to that warehouse.  Specific Attribute instances cannot (and
/// should not) generally be created before the class is registered with a warehouse.
///
/// In order to work with QueryCache objects, attribute classes should include
/// a public typedef for a Key type, as well as a setFromKey function that
/// takes this type as an argument:
///
/// @begincode
/// class FooAttribute : public Attribute
/// {
/// public:
///   typedef [type-for-foo] Key;
///
///   void
///   setFrom(Key k)
///   {
///     // code to mutate/reinitialize FooAttribute using k
///   }
/// }
/// @endcode
class Attribute
{
public:
  /// Constructs/initializes a new attribute with the specified name for use in warehouse w.  The
  /// attribute must have been previously registered with w prior to calling this constructor.
  Attribute(TheWarehouse & w, const std::string name);
  virtual ~Attribute() {}

  inline bool operator==(const Attribute & other) const
  {
    return _id == other._id && isEqual(other);
  }
  inline bool operator!=(const Attribute & other) const { return !(*this == other); }

  /// returns the unique attribute ID associated with all attributes that have the same (mose
  /// derived) class as this object. This ID is determined at construction time
  /// this
  inline unsigned int id() const { return _id; }

  /// This function must return a deterministic value that is uniquely determined by
  /// the data the attribute holds (i.e. is initialized with).  Ideally, the data should be
  /// uniformly and randomly distributed across the domain of size_t values - e.g. 1 and 2 should
  /// hash to completely unrelated values.  Use of std::hash for POD is encouraged.  A convenience
  /// hash_combine function is also provided to combine the results an existing hash with one or
  /// more other values.
  virtual std::size_t hash() const = 0;

  /// initFrom reads and stores the desired meta-data from obj for later matching comparisons.
  virtual void initFrom(const MooseObject * obj) = 0;
  /// isMatch returns true if the meta-data stored in this attribute is equivalent to that
  /// stored in other. This is is for query matching - not exact equivalence. isMatch does not need
  /// to check/compare the values from the instances' id() functions.
  virtual bool isMatch(const Attribute & other) const = 0;
  /// isEqual returns true if the meta-data stored in this attribute is identical to that
  /// stored in other. isEqual does not need to check/compare the values from the instances' id()
  /// functions.
  virtual bool isEqual(const Attribute & other) const = 0;
  /// clone creates and returns and identical (deep) copy of this attribute - i.e. the result of
  /// clone should return true if passed into isMatch.
  virtual std::unique_ptr<Attribute> clone() const = 0;

private:
  int _id = -1;
};

#define clonefunc(T)                                                                               \
  virtual std::unique_ptr<Attribute> clone() const override                                        \
  {                                                                                                \
    return std::unique_ptr<Attribute>(new T(*this));                                               \
  }

#define hashfunc(...)                                                                              \
  virtual std::size_t hash() const override                                                        \
  {                                                                                                \
    std::size_t h = 0;                                                                             \
    Moose::hash_combine(h, __VA_ARGS__);                                                           \
    return h;                                                                                      \
  }

/**
 * This attribute describes sorting state
 */
class AttribSorted : public Attribute
{
public:
  typedef bool Key;
  void setFrom(const Key & k) { _val = k; }

  AttribSorted(TheWarehouse & w) : Attribute(w, "sorted"), _val(false), _initd(false) {}
  AttribSorted(TheWarehouse & w, bool is_sorted)
    : Attribute(w, "sorted"), _val(is_sorted), _initd(true)
  {
  }
  AttribSorted(const AttribSorted &) = default;
  AttribSorted(AttribSorted &&) = default;
  AttribSorted & operator=(const AttribSorted &) = default;
  AttribSorted & operator=(AttribSorted &&) = default;

  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribSorted);

private:
  bool _val;
  bool _initd;
};

#undef clonefunc
#undef hashfunc

/// TheWarehouse uses this operator function for indexing and caching queries. So this is
/// important even though you don't see it being called (directly) anywhere - it *IS* being used.
bool operator==(const std::unique_ptr<Attribute> & lhs, const std::unique_ptr<Attribute> & rhs);

namespace std
{
/// This template specialization allows Attributes to be used as unordered map key.
template <>
struct hash<Attribute>
{
public:
  std::size_t operator()(const Attribute & attrib) const
  {
    std::size_t h = attrib.hash();
    Moose::hash_combine(h, attrib.id());
    return h;
  }
};

/// This template specialization allows vector<Attribute> to be used as unordered map key.
template <>
struct hash<std::vector<std::unique_ptr<Attribute>>>
{
public:
  std::size_t operator()(const std::vector<std::unique_ptr<Attribute>> & attribs) const
  {
    std::size_t h = 0;
    for (auto & attrib : attribs)
      Moose::hash_combine(h, *attrib);
    return h;
  }
};
}

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
  template <typename T>
  using KeyType = typename T::Key;
  template <typename T>
  using AttribType = T *;

  /// QueryCache is a convenient way to construct and pass around (possible
  /// partially constructed) warehouse queries.  The warehouse's "query()" or
  /// "queryCache(...)" functions should generally be used to create new Query
  /// objects rather than constructing them directly.  A Query object holds a
  /// list of persistent conditions used to filter/select objects from the
  /// warehouse.  When the query is executed/run, results are filtered by
  /// "and"ing each condition together - i.e. only objects that match *all*
  /// conditions are returned.
  ///
  ///
  /// Template arguments (i.e. Attribs) are used to specify parametrized query
  /// conditions.  The passed template parameters should be the Attribute
  /// classes you want to use for parameterization (i.e. the values that will
  /// change from query to query).
  template <typename... Attribs>
  class QueryCache
  {
  public:
    typedef std::tuple<KeyType<Attribs>...> KeyTuple;
    typedef std::tuple<AttribType<Attribs>...> AttribTuple;

    QueryCache() {}

    /// Creates a new query operating on the given warehouse w.  You should generally use
    /// TheWarehouse::query() instead.
    QueryCache(TheWarehouse & w) : _w(&w)
    {
      addAttribs<0, Attribs...>();
      _attribs.reserve(5);
    }

    template <typename T>
    QueryCache(const T & q) : _w(&q.warehouse())
    {
      addAttribs<0, Attribs...>();
      _attribs.reserve(5);

      for (auto & attrib : q.attributes())
        _attribs.push_back(attrib->clone());
    }

    QueryCache & operator=(const QueryCache & other)
    {
      if (this == &other)
        return *this;

      _attribs.clear();
      _w = other._w;
      // MUST have own pointers to attribs to avoid data races - don't copy _key_attribs.
      // initialize parametrized attributes and tuple:
      addAttribs<0, Attribs...>();
      _key_tup = other._key_tup;
      // do NOT copy the cache.

      // only copy over non-parametrized attributes
      for (std::size_t i = std::tuple_size<AttribTuple>::value; i < other._attribs.size(); i++)
        _attribs.push_back(other._attribs[i]->clone());
      return *this;
    }

    /// Copy constructor from another Query
    template <typename T>
    QueryCache & operator=(const T & q)
    {
      _attribs.clear();
      _w = &q.warehouse();

      addAttribs<0, Attribs...>();
      _attribs.reserve(5);

      for (auto & attrib : q.attributes())
        _attribs.push_back(attrib->clone());

      return *this;
    }

    QueryCache(const QueryCache & other) : _w(other._w), _key_tup(other._key_tup)
    {
      // do NOT copy the cache.

      // initialize parametrized attributes and tuple:
      addAttribs<0, Attribs...>(); // MUST have own pointers to attribs to avoid data races.
      for (std::size_t i = std::tuple_size<AttribTuple>::value; i < other._attribs.size(); i++)
        _attribs.push_back(other._attribs[i]->clone());
    }

    /// Adds a new condition to the query.  The template parameter T is the Attribute class of
    /// interest and args are forwarded to T's constructor to build+add the attribute in-situ.
    /// Conditions represent persistent query conditions that do not change
    /// from query to query for a particular QueryCache instance.
    template <typename T, typename... Args>
    QueryCache & condition(Args &&... args)
    {
      _attribs.emplace_back(new T(*_w, std::forward<Args>(args)...));
      _cache.clear(); // invalidate cache if base query changes.
      return *this;
    }

    /// clone creates and returns an independent copy of the query in its current state.
    QueryCache clone() const { return Query(*this); }
    /// count returns the number of results that match the query (this requires actually running
    /// the query).
    std::size_t count() { return _w->count(_attribs); }

    TheWarehouse & warehouse() const { return *_w; }

    /// attribs returns a copy of the constructed Attribute list for the query in its current state.
    std::vector<std::unique_ptr<Attribute>> attributes() const { return clone()._attribs; }

    /// queryInto executes the query and stores the results in the given
    /// vector.  For parametrized queries (i.e. QueryCaches created with more
    /// than zero template arguments) args must contain one value for each
    /// parametrized query attribute - the types of args should be equal to the
    /// ::Key typedef specified for each corresponding parametrized attribute.
    /// All results must be castable to the templated type T. If the objects
    /// we are querying into inherit from the dependency resolver interface, then
    /// they will be sorted
    template <typename T, typename... Args>
    std::vector<T *> & queryInto(std::vector<T *> & results, Args &&... args)
    {
      return queryIntoHelper(results, true, args...);
    }

    /// queryInto executes the query and stores the results in the given
    /// vector.  For parametrized queries (i.e. QueryCaches created with more
    /// than zero template arguments) args must contain one value for each
    /// parametrized query attribute - the types of args should be equal to the
    /// ::Key typedef specified for each corresponding parametrized attribute.
    /// All results must be castable to the templated type T. These objects
    /// will not be sorted
    template <typename T, typename... Args>
    std::vector<T *> & queryIntoUnsorted(std::vector<T *> & results, Args &&... args)
    {
      return queryIntoHelper(results, false, args...);
    }

    /// Gets the number of attributes associated with the cached query
    std::size_t numAttribs() const { return _attribs.size(); }

  private:
    /// queryInto executes the query and stores the results in the given
    /// vector.  For parametrized queries (i.e. QueryCaches created with more
    /// than zero template arguments) args must contain one value for each
    /// parametrized query attribute - the types of args should be equal to the
    /// ::Key typedef specified for each corresponding parametrized attribute.
    /// All results must be castable to the templated type T. If the objects
    /// we are querying into inherit from the dependency resolver interface, then
    /// they will be sorted if \p sort is true
    template <typename T, typename... Args>
    std::vector<T *> & queryIntoHelper(std::vector<T *> & results, const bool sort, Args &&... args)
    {
      std::lock_guard<std::mutex> lock(_cache_mutex);
      setKeysInner<0, KeyType<Attribs>...>(args...);

      std::size_t query_id;
      const auto entry = _cache.find(std::make_pair(sort, _key_tup));
      if (entry == _cache.end())
      {
        setAttribsInner<0, KeyType<Attribs>...>(args...);
        // add the sort attribute. No need (I think) to clear the cache because the base query is
        // not changing
        _attribs.emplace_back(new AttribSorted(*_w, sort));
        query_id = _w->queryID(_attribs);
        _cache[std::make_pair(sort, _key_tup)] = query_id;
        // remove the sort attribute
        _attribs.pop_back();
      }
      else
        query_id = entry->second;

      return _w->queryInto(query_id, results);
    }

    template <int Index, typename A, typename... As>
    void addAttribs()
    {
      std::get<Index>(_attrib_tup) = new A(*_w);
      _attribs.emplace_back(std::get<Index>(_attrib_tup));
      addAttribs<Index + 1, As...>();
    }
    template <int Index>
    void addAttribs()
    {
    }

    template <int Index, typename K, typename... Args>
    void setKeysInner(K & k, Args &... args)
    {
      std::get<Index>(_key_tup) = k;
      setKeysInner<Index + 1, Args...>(args...);
    }
    template <int Index>
    void setKeysInner()
    {
    }

    template <int Index, typename K, typename... Args>
    void setAttribsInner(K k, Args &... args)
    {
      std::get<Index>(_attrib_tup)->setFrom(k);
      setAttribsInner<Index + 1, Args...>(args...);
    }
    template <int Index>
    void setAttribsInner()
    {
    }

    TheWarehouse * _w = nullptr;
    std::vector<std::unique_ptr<Attribute>> _attribs;

    KeyTuple _key_tup;
    AttribTuple _attrib_tup;
    std::map<std::pair<bool, KeyTuple>, std::size_t> _cache;
    std::mutex _cache_mutex;
  };

  using Query = QueryCache<>;

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
  /// foo)", you would call "registerAttribute<YourAttribute>("your_attrib_name", constructor_arg1,
  /// ...)".  Custom attribute classes are required to pass an attribute name (i.e.
  /// "your_attrib_name") to the Attribute base class.  The dummy args are forwarded to the attrib
  /// class' constructor. The name passed here into registerAttribute
  /// must be the same string as the name passed to the Attribute base class's constructor.
  template <typename T, typename... Args>
  unsigned int registerAttribute(const std::string & name, Args... dummy_args)
  {
    auto it = _attrib_ids.find(name);
    if (it != _attrib_ids.end())
      return it->second;

    _attrib_ids[name] = _attrib_list.size();
    _attrib_list.push_back(std::unique_ptr<Attribute>(new T(*this, dummy_args...)));
    return _attrib_list.size() - 1;
  }

  /// Returns a unique ID associated with the given attribute name - i.e. an attribute and name
  /// that were previously registered via calls to registerAttribute.  Users should generally *not*
  /// need to use this function.
  inline unsigned int attribID(const std::string & name)
  {
    auto it = _attrib_ids.find(name);
    if (it != _attrib_ids.end())
      return it->second;
    mooseError("no ID exists for unregistered attribute '", name, "'");
  }

  /// add adds a new object to the warehouse and stores attributes/metadata about it for running
  /// queries/filtering.  The warehouse will maintain a pointer to the object indefinitely.
  void add(std::shared_ptr<MooseObject> obj);

  /// update updates the metadata/attribute-info stored for the given object obj that must already
  /// exists in the warehouse.  Call this if an object's state has changed in such a way that its
  /// warehouse attributes have become stale/incorrect.
  void update(MooseObject * obj);
  /// update updates the metadata/attribute-info stored for the given object obj that must already
  /// exists in the warehouse.  Call this if an object's state has changed in such a way that its
  /// warehouse attributes have become stale/incorrect.
  /// Any attribute specified in extra overwrites/trumps one read from the object's current state.
  void update(MooseObject * obj, const Attribute & extra);
  /// query creates and returns an initialized a query object for querying objects from the
  /// warehouse.
  Query query() { return Query(*this); }
  /// count returns the number of objects that match the provided query conditions. This requires
  /// executing a full query operation (i.e. as if calling queryInto). A Query object should
  /// generally be used via the query() member function instead.
  std::size_t count(const std::vector<std::unique_ptr<Attribute>> & conds);
  /// queryInto takes the given conditions (i.e. Attributes holding the values to filter/match
  /// over) and filters all objects in the warehouse that match all conditions (i.e. "and"ing the
  /// conditions together) and stores them in the results vector. All result objects must be
  /// castable to the templated type T. This function filters out disabled objects on the fly -
  /// only returning enabled ones.
  template <typename T>
  std::vector<T *> & queryInto(const std::vector<std::unique_ptr<Attribute>> & conds,
                               std::vector<T *> & results)
  {
    return queryInto(queryID(conds), results);
  }

  std::size_t queryID(const std::vector<std::unique_ptr<Attribute>> & conds);

  template <typename T>
  std::vector<T *> & queryInto(int query_id, std::vector<T *> & results, bool show_all = false)
  {
    std::lock_guard<std::mutex> lock(_obj_cache_mutex);
    auto & objs = query(query_id);
    results.clear();
    results.reserve(objs.size());
    for (unsigned int i = 0; i < objs.size(); i++)
    {
      auto obj = objs[i];
      mooseAssert(dynamic_cast<T *>(obj),
                  "queried object has incompatible c++ type for object named " + obj->name());
      if (show_all || obj->enabled())
        results.push_back(dynamic_cast<T *>(obj));
    }
    return results;
  }

private:
  /// prepares a query and returns an associated query_id (i.e. for use with the query function).
  int prepare(std::vector<std::unique_ptr<Attribute>> conds);

  /// callers of this function must lock _obj_cache_mutex as long as a reference to the returned
  /// vector is being used.
  const std::vector<MooseObject *> & query(int query_id);

  void readAttribs(const MooseObject * obj, std::vector<std::unique_ptr<Attribute>> & attribs);

  std::unique_ptr<Storage> _store;
  std::vector<std::shared_ptr<MooseObject>> _objects;
  std::unordered_map<MooseObject *, std::size_t> _obj_ids;

  // Results from queries are cached here. The outer vector index is the query id as stored by the
  // _query_cache data structure.  A list objects that match each query id are stored.
  std::vector<std::vector<MooseObject *>> _obj_cache;
  // This stores a query id for every query keyed by the query conditions/attributes.
  // User-initiated queries check this map to see if a queries results have already been cached.
  // The query id is an index into the _obj_cache data structure.
  std::unordered_map<std::vector<std::unique_ptr<Attribute>>, int> _query_cache;

  std::unordered_map<std::string, unsigned int> _attrib_ids;
  std::vector<std::unique_ptr<Attribute>> _attrib_list;

  std::mutex _obj_mutex;
  std::mutex _query_cache_mutex;
  std::mutex _obj_cache_mutex;
};
