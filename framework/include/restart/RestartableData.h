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

class RestartableDataReader;

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
   * Whether or not this data has been declared
   */
  bool declared() const { return _declared; }

  /**
   * Sets that this restartable value has been declared
   */
  void setDeclared();

  /**
   * Whether or not this data has been restored
   */
  bool restored() const { return _restored; }

  /**
   * Helper that protects access to setRestored() to only RestartableDataReader
   */
  class WriteRestoredKey
  {
    friend class RestartableDataReader;

    WriteRestoredKey() {}
    WriteRestoredKey(const WriteRestoredKey &) {}
  };

  /**
   * Sets the restored state of this value to \p restored
   */
  void setRestored(const bool restored, const WriteRestoredKey);

  // save/restore in a file
  virtual void store(std::ostream & stream) = 0;
  virtual void load(std::istream & stream) = 0;

protected:
  /// The full (unique) name of this particular piece of data.
  const std::string _name;

  /// A context pointer for helping with load and store
  void * const _context;

private:
  /// Whether or not this data has been declared (true) or only retreived (false)
  bool _declared;

  /// Whether or not this has value has been restored
  bool _restored;
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

  /**
   * Store the RestartableData into a binary stream
   */
  virtual void store(std::ostream & stream) override;

  /**
   * Load the RestartableData from a binary stream
   */
  virtual void load(std::istream & stream) override;

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

using DataNames = std::unordered_set<std::string>;
