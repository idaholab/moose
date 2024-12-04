//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Restartable.h"
#include "MooseUtils.h"

class ChainControl;

/**
 * Abstract definition of a ChainControlData value.
 */
class ChainControlDataBase : public Restartable
{
public:
  /**
   * Constructor
   * @param moose_app MooseApp object this object belong to
   * @param name The full (unique) name for this piece of data.
   */
  ChainControlDataBase(MooseApp & moose_app, const std::string & name)
    : Restartable(moose_app, name, "chain_control", 0),
      _name(name),
      _declared(false),
      _chain_control(nullptr)
  {
  }

  virtual ~ChainControlDataBase() = default;

  /**
   * String identifying the type of parameter stored.
   * Must be reimplemented in derived classes.
   */
  virtual std::string type() = 0;

  /**
   * The full (unique) name of this particular piece of data.
   */
  const std::string & name() const { return _name; }

  /**
   * Get the pointer to the control object that declared this control data
   */
  const ChainControl & getChainControl() const;

  /**
   * Set the pointer to the control object that declared this control data
   */
  void setChainControl(ChainControl & ctrl) { _chain_control = &ctrl; }

  /**
   * Mark the data as declared
   */
  void setDeclared() { _declared = true; }

  /**
   * Get the declared state
   */
  bool getDeclared() { return _declared; }

  /**
   * Copy the current value into the old value
   */
  virtual void copyValuesBack() = 0;

protected:
  /// The full (unique) name of this particular piece of data.
  const std::string _name;
  /// true if the data was declared by calling declareControlData. All data must be declared up front.
  bool _declared;
  /// The control object that declared this control data
  ChainControl * _chain_control;
};

/**
 * Concrete definition of a parameter value
 * for a specified type.
 */
template <typename T>
class ChainControlData : public ChainControlDataBase
{
public:
  /**
   * Constructor
   * @param name The full (unique) name for this piece of data.
   */
  ChainControlData(MooseApp & moose_app, std::string name)
    : ChainControlDataBase(moose_app, name),
      _value(declareRestartableData<T>(name)),
      _value_old(declareRestartableData<T>(name + "_old"))
  {
  }

  /**
   * @returns a read-only reference to the parameter value.
   */
  const T & get() const { return _value; }

  /**
   * @returns a read-only reference to the old value.
   */
  const T & getOld() const { return _value_old; }

  /**
   * @returns a writable reference to the current value.
   */
  T & set() { return _value; }

  /**
   * String identifying the type of parameter stored.
   */
  virtual std::string type() override;

  virtual void copyValuesBack() override;

private:
  /// Current value
  T & _value;
  /// Old value
  T & _value_old;
};

template <typename T>
inline std::string
ChainControlData<T>::type()
{
  return MooseUtils::prettyCppType<T>();
}

template <typename T>
inline void
ChainControlData<T>::copyValuesBack()
{
  _value_old = _value;
}
