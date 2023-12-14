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
#include "MooseUtils.h"

// C++ includes
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "nlohmann/json.h"

class RestartableDataReader;
class RestartableDataWriter;
class MooseApp;

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
  RestartableDataValue(const std::string & name, void * const context);

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
   * The type ID of the underlying data.
   */
  virtual const std::type_info & typeId() const = 0;

  /**
   * The full (unique) name of this particular piece of data.
   */
  const std::string & name() const { return _name; }

  /**
   * A context pointer for helping with load / store.
   */
  void * context() { return _context; }

  /**
   * @return Whether or not the data has context set.
   */
  bool hasContext() const { return _context != nullptr; }

  /**
   * Helper that protects access to setDeclared() to only MooseApp
   */
  class SetDeclaredKey
  {
    friend class MooseApp;
    SetDeclaredKey() {}
    SetDeclaredKey(const SetDeclaredKey &) {}
  };

  /**
   * Whether or not this data has been declared
   */
  bool declared() const { return _declared; }

  /**
   * Sets that this restartable value has been declared
   */
  void setDeclared(const SetDeclaredKey);

  /**
   * Whether or not this data has been loaded
   *
   * This is typically reset on a call to
   * RestartableDataReader::restore()
   */
  bool loaded() const { return _loaded; }

  /**
   * Helper that protects access to setNotLoaded() to only RestartableDataReader
   */
  class SetNotLoadedKey
  {
    friend class RestartableDataReader;
    SetNotLoadedKey() {}
    SetNotLoadedKey(const SetNotLoadedKey &) {}
  };

  /**
   * Sets that this restartable value has been loaded
   */
  void setNotLoaded(const SetNotLoadedKey) { _loaded = false; }

  /**
   * Whether or not this data has been loaded
   *
   * This is typically reset on a call to
   * RestartableDataWriter::write()
   */
  bool stored() const { return _stored; }

  /**
   * Helper that protects access to setNotStored() to only RestartableDataWriter
   */
  class SetNotStoredKey
  {
    friend class RestartableDataWriter;
    SetNotStoredKey() {}
    SetNotStoredKey(const SetNotStoredKey &) {}
  };

  /**
   * Sets that this restartable value has been loaded
   */
  void setNotStored(const SetNotStoredKey) { _stored = false; }

  /**
   * Stores the value into the stream \p stream and sets it as stored
   */
  void store(std::ostream & stream);
  /**
   * Loads the value from the stream \p stream and sets it as loaded
   */
  void load(std::istream & stream);

  /**
   * Internal method that stores the value into the stream \p stream
   * in the specialized class.
   */
  virtual void storeInternal(std::ostream & stream) = 0;
  /**
   * Internal method that loads the value from the stream \p stream
   * in the specialized class.
   */
  virtual void loadInternal(std::istream & stream) = 0;

  /**
   * @return Whether or not this value supports storing JSON via \p store
   */
  virtual bool hasStoreJSON() const = 0;

  /**
   * Struct that represents parameters for how to store the JSON value via \p store
   */
  struct StoreJSONParams
  {
    StoreJSONParams() {} // fixes a compiler bug with default constructors
    bool value = true;
    bool type = true;
    bool name = false;
    bool declared = false;
    bool loaded = false;
    bool stored = false;
    bool has_context = false;
  };

  /**
   * Stores this restartable data in the JSON entry \p json, with the options
   * set by \p params (optional; defaults to just the type and underlying value)
   *
   * If the underlying type is not supported for JSON output (if hasStoreJSON() == false),
   * and the parameters have the value output as enabled, this will error.
   */
  void store(nlohmann::json & json, const StoreJSONParams & params = StoreJSONParams{}) const;

protected:
  /**
   * Internal method for storing the underlying JSON value
   */
  virtual void storeJSONValue(nlohmann::json & json) const = 0;

  /// The full (unique) name of this particular piece of data.
  const std::string _name;

  /// A context pointer for helping with load and store
  void * const _context;

private:
  /// Whether or not this data has been declared (true) or only retreived (false)
  bool _declared;

  /// Whether or not this has value has been loaded
  bool _loaded;

  /// Whether or not this has value has been stored
  bool _stored;
};

/**
 * Concrete definition of a parameter value
 * for a specified type.
 */
template <typename T>
class RestartableData : public RestartableDataValue
{
public:
  /// Whether or not this type has a JSON store method implemented
  static constexpr bool has_store_json = std::is_constructible_v<nlohmann::json, T>;

  /**
   * Constructor
   * @param name The full (unique) name for this piece of data.
   * @param context 'typeless' pointer to user-specific data.
   * @param arg Forwarded arguments that are passed to the constructor of the data.
   */
  template <typename... Params>
  RestartableData(const std::string & name, void * const context, Params &&... args)
    : RestartableDataValue(name, context),
      _value(std::make_unique<T>(std::forward<Params>(args)...))
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
   * Resets (destructs) the underlying data.
   */
  void reset();

  /**
   * String identifying the type of parameter stored.
   */
  virtual std::string type() const override final;

  virtual const std::type_info & typeId() const override final { return typeid(T); }

  virtual bool hasStoreJSON() const override final { return has_store_json; }

protected:
  /**
   * Store the RestartableData into a binary stream
   */
  virtual void storeInternal(std::ostream & stream) override;

  /**
   * Load the RestartableData from a binary stream
   */
  virtual void loadInternal(std::istream & stream) override;

  virtual void storeJSONValue(nlohmann::json & json) const override final;

private:
  /// Stored value.
  std::unique_ptr<T> _value;
};

// ------------------------------------------------------------
// RestartableData<> class inline methods
template <typename T>
inline const T &
RestartableData<T>::get() const
{
  mooseAssert(_value, "Not valid");
  return *_value;
}

template <typename T>
inline T &
RestartableData<T>::set()
{
  mooseAssert(_value, "Not valid");
  return *_value;
}

template <typename T>
inline void
RestartableData<T>::reset()
{
  mooseAssert(_value, "Not valid"); // shouldn't really call this twice
  _value.reset();
}

template <typename T>
inline std::string
RestartableData<T>::type() const
{
  return MooseUtils::prettyCppType<T>();
}

template <typename T>
inline void
RestartableData<T>::storeInternal(std::ostream & stream)
{
  storeHelper(stream, set(), _context);
}

template <typename T>
inline void
RestartableData<T>::loadInternal(std::istream & stream)
{
  loadHelper(stream, set(), _context);
}

template <typename T>
inline void
RestartableData<T>::storeJSONValue(nlohmann::json & json) const
{
  if constexpr (RestartableData<T>::has_store_json)
    nlohmann::to_json(json, get());
  else
    mooseAssert(false, "Should not be called");
}

using DataNames = std::unordered_set<std::string>;
