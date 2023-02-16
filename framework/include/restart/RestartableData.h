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
#include "DataIO.h"
#include "JsonIO.h"
#include "MooseUtils.h"

// C++ includes
#include <vector>
#include <unordered_set>
#include <unordered_map>

// JSON object
#include "nlohmann/json.h"

// Forward declarations
class RestartableDataValue;

/**
 * Abstract definition of a RestartableData value.
 */
class RestartableDataValue
{
public:
  /**
   * Constructor
   * @param name The full (unique) name for this piece of data.
   * @param context 'typeless' pointer to user-specific data.
   */
  RestartableDataValue(std::string name, void * context) : _name(name), _context(context) {}

  /**
   * Destructor.
   */
  virtual ~RestartableDataValue() = default;

  /**
   * String identifying the type of parameter stored.
   * Must be reimplemented in derived classes.
   */
  virtual std::string type() const = 0;

  /**
   * The full (unique) name of this particular piece of data.
   */
  const std::string & name() const { return _name; }

  /**
   * A context pointer for helping with load / store.
   */
  void * context() { return _context; }

  virtual void swap(RestartableDataValue * rhs) = 0;

  // save/restore in a file
  virtual void store(std::ostream & stream) = 0;
  virtual void load(std::istream & stream) = 0;

  // save/load to JSON object
  virtual void toJSON(nlohmann::json & json) const = 0;
  virtual void fromJSON(const nlohmann::json & json) = 0;

protected:
  /// The full (unique) name of this particular piece of data.
  std::string _name;

  /// A context pointer for helping with load and store
  void * _context;
};

/**
 * Concrete definition of a parameter value
 * for a specified type.
 */
template <typename T>
class RestartableData : public RestartableDataValue
{
public:
  /**
   * Constructor
   * @param name The full (unique) name for this piece of data.
   * @param context 'typeless' pointer to user-specific data.
   * @param arg Forwarded arguments that are passed to the constructor of the data.
   */
  template <typename... Params>
  RestartableData(std::string name, void * context, Params &&... args)
    : RestartableDataValue(name, context),
      _value_ptr(std::make_unique<T>(std::forward<Params>(args)...))
  {
  }

  /**
   * @returns a read-only reference to the parameter value.
   */
  const T & get() const;

  /**
   * @returns a writable reference to the parameter value.
   */
  T & set();

  /**
   * String identifying the type of parameter stored.
   */
  virtual std::string type() const override;

  /**
   * Swap
   */
  virtual void swap(RestartableDataValue * rhs) override;

  /**
   * Store the RestartableData into a binary stream
   */
  virtual void store(std::ostream & stream) override;

  /**
   * Load the RestartableData from a binary stream
   */
  virtual void load(std::istream & stream) override;

  /**
   * Store the restartable data into a JSON object
   */
  virtual void toJSON(nlohmann::json & json) const override;

  /**
   * Load the restartable data into a JSON object
   */
  virtual void fromJSON(const nlohmann::json & json) override;

private:
  /// Stored value.
  const std::unique_ptr<T> _value_ptr;
};

// ------------------------------------------------------------
// RestartableData<> class inline methods
template <typename T>
const T &
RestartableData<T>::get() const
{
  mooseAssert(_value_ptr, "Value not initialized");
  return *_value_ptr;
}

template <typename T>
T &
RestartableData<T>::set()
{
  mooseAssert(_value_ptr, "Value not initialized");
  return *_value_ptr;
}

template <typename T>
inline std::string
RestartableData<T>::type() const
{
  return MooseUtils::prettyCppType<T>();
}

template <typename T>
inline void
RestartableData<T>::swap(RestartableDataValue * libmesh_dbg_var(rhs))
{
  mooseAssert(rhs, "Assigning NULL?");
  //  _value.swap(cast_ptr<RestartableData<T>*>(rhs)->_value);
}

template <typename T>
inline void
RestartableData<T>::store(std::ostream & stream)
{
  storeHelper(stream, set(), _context);
}

template <typename T>
inline void
RestartableData<T>::load(std::istream & stream)
{
  loadHelper(stream, set(), _context);
}

template <typename T>
inline void
RestartableData<T>::toJSON(nlohmann::json & /*json*/) const
{
  // TODO: see JsonIO.h
  // T & tmp = *_value_ptr;
  // storeHelper(json, tmp, _context);
}

template <typename T>
inline void
RestartableData<T>::fromJSON(const nlohmann::json & /*json*/)
{
  // TODO: see JsonIO.h
  // T & tmp = *_value_ptr;
  // loadHelper(json, tmp, _context);
}

/**
 * Struct and Aliases for Restartable/Recoverable structures
 */
struct RestartableDataValuePair
{
  RestartableDataValuePair(std::unique_ptr<RestartableDataValue> v, bool d)
    : value(std::move(v)), declared(d)
  {
  }

  std::unique_ptr<RestartableDataValue> value;
  bool declared;
};

using RestartableDataMap = std::unordered_map<std::string, RestartableDataValuePair>;
using RestartableDataMaps = std::vector<RestartableDataMap>;

using DataNames = std::unordered_set<std::string>;
