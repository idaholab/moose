//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/parameters.h"
#include "MooseObjectParameterName.h"
#include "MooseError.h"
#include "ControlOutput.h"

/**
 * An intermediate object for building a "controllable item", where an "item" can refer to multiple
 * input parameters with different names.
 *
 * The name supplied to the constructor is considered the "master" parameter. The parameter(s)
 * added via the connect method are considered the secondaries.
 *
 * In general, an ControllableItem will have a one-to-one relationship with an input parameter
 * value, but in some instances it is desirable to connect parameters with different names together.
 * For example, within MOOSE when a material is defined in an input file multiple Material objects
 * are generated automatically. If a parameter is controlled on one of these objects it is
 * necessary to also have the values on the other controlled as well. This example is the
 * driver behind the creation of this intermediate class.
 */
class ControllableItem
{
public:
  ControllableItem(const MooseObjectParameterName & name,
                   libMesh::Parameters::Value * value,
                   const std::set<ExecFlagType> & flags = {});
  virtual ~ControllableItem() = default;

  ControllableItem(const ControllableItem &) = default;
  ControllableItem(ControllableItem &&) = default;

  ControllableItem & operator=(const ControllableItem &) = delete;
  ControllableItem & operator=(ControllableItem &&) = delete;

  /**
   * Connects the supplied item with this item to allow for multiple parameters to be changed by
   * one.
   */
  void connect(ControllableItem * item, bool type_check = true);

  /**
   * Set the value(s) of the controlled parameters stored in this class.
   *
   * The 'skip_type_check' flag allows this object to work with ControllableParameter that
   * can store values of varying types.
   */
  template <typename T>
  void set(const T & value, bool type_check = true);

  /**
   * Return a copy of all values for this "item".
   */
  template <typename T>
  std::vector<T> get(bool type_check = true) const;

  /**
   * Return true if the template argument is valid for ALL items.
   */
  template <typename T>
  bool check() const;

  ///@{
  /**
   * Use the master name for comparison operators to allow object to work within a set/map.
   */
  bool operator==(const ControllableItem & rhs) const { return name() == rhs.name(); }
  bool operator!=(const ControllableItem & rhs) const { return name() != rhs.name(); }
  bool operator<(const ControllableItem & rhs) const { return name() < rhs.name(); }
  ///@}

  /**
   * Return the name of the master parameter.
   */
  virtual const MooseObjectParameterName & name() const;

  /**
   * Return the type of the master parameter.
   */
  std::string type() const;

  /**
   * Returns a string displaying the parameter name and current value.
   */
  virtual std::string dump(unsigned int indent = 0) const;

  ///@{
  /**
   * Methods for ControlOutput::outputChangedControls, these don't have meaning outside of this
   * function.
   */
  void resetChanged() { _changed = false; }
  bool isChanged() { return _changed; }
  ///@}

  /**
   * Return the execute flag restrictions, an empty set is un-restricted
   */
  const std::set<ExecFlagType> & getExecuteOnFlags() const { return _execute_flags; }

  /// Allows this to be used with std:: cout
  friend std::ostream & operator<<(std::ostream & stream, const ControllableItem & obj);

protected:
  /**
   * Constructor for creating an empty item (see ControllableAlias)
   */
  ControllableItem();

  /// List of names for this item
  std::vector<std::pair<MooseObjectParameterName, libMesh::Parameters::Value *>> _pairs;

  /// Flag for ControlOutput, allows output objects to keep track of when a parameter is altered
  bool _changed = false;

  /// Flags to which the control is restricted (if not set it is unrestricted)
  std::set<ExecFlagType> _execute_flags;
};

template <typename T>
void
ControllableItem::set(const T & value, bool type_check /*=true*/)
{
  for (auto & pair : _pairs)
  {
    libMesh::Parameters::Parameter<T> * param =
        dynamic_cast<libMesh::Parameters::Parameter<T> *>(pair.second);
    if (type_check && param == nullptr)
      mooseError("Failed to set the '",
                 pair.first,
                 "' parameter the supplied template argument must be of type '",
                 pair.second->type(),
                 "'.");
    else if (param != nullptr)
    {
      param->set() = value;
      _changed = true;
    }
  }
}

template <typename T>
std::vector<T>
ControllableItem::get(bool type_check /*=true*/) const
{
  std::vector<T> output;
  output.reserve(_pairs.size());
  for (const auto & pair : _pairs)
  {
    libMesh::Parameters::Parameter<T> * param =
        dynamic_cast<libMesh::Parameters::Parameter<T> *>(pair.second);
    if (type_check && param == nullptr)
      mooseError("Failed to get the '",
                 pair.first,
                 "' parameter the supplied template argument must be of type '",
                 pair.second->type(),
                 "'.");
    else if (param != nullptr)
      output.push_back(param->get());
  }
  return output;
}

template <typename T>
bool
ControllableItem::check() const
{
  return std::all_of(_pairs.begin(),
                     _pairs.end(),
                     [](std::pair<MooseObjectParameterName, libMesh::Parameters::Value *> pair)
                     {
                       libMesh::Parameters::Parameter<T> * param =
                           dynamic_cast<libMesh::Parameters::Parameter<T> *>(pair.second);
                       return param != nullptr;
                     });
}

/**
 * Allows for aliases to be defined via InputParameterWarehouse::addControllableParameterAlias.
 */
class ControllableAlias : public ControllableItem
{
public:
  ControllableAlias(const MooseObjectParameterName & name, ControllableItem *);
  virtual const MooseObjectParameterName & name() const override;
  virtual std::string dump(unsigned int indent = 0) const override;

private:
  MooseObjectParameterName _name;
};
