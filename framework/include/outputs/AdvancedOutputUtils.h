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
#include "MooseError.h"
#include "ExecFlagEnum.h"

// Forward declarations
class InputParameters;

/**
 * A structure for storing the various lists that contain
 * the names of the items to be exported. An instance of this
 * struct exists for each of the output types (non-linear variables,
 * scalar variables, postprocessors, etc.)
 *
 * @see OutputDataWarehouse
 */
struct OutputData
{
  /// A list of all possible outputs
  std::set<std::string> available;

  /// User-supplied list of outputs to display
  std::set<std::string> show;

  /// User-supplied list of outputs to hide
  std::set<std::string> hide;

  /// A list of the outputs to write
  std::set<std::string> output;

  /// Clear existing sets for re-initialization
  void reset();
};

/**
 * In newer versions of Clang calling operator[] on a map with a component that
 * has a default constructor is an error, thus utilizing a map directly to store
 * a ExecFlagEnum is not possible.
 *
 * This template class is a map wrapper that provides the basic map-like functionality
 * for accessing map types with operator[] by using find internally. It also produces
 * an error if the map key does not exists, this it provides a layer of protection not
 * available to maps operator[] in general.
 *
 * This class is used here to create two different warehouse containers below.
 *
 * @see OutputOnWarehouse OutputDataWarehouse
 */
template <typename T>
class OutputMapWrapper
{
public:
  /**
   * Constructor
   */
  OutputMapWrapper(){};

  /**
   * A map accessor that errors if the key is not found
   */
  T & operator[](const std::string & name)
  {
    // Locate the map entry, error if it is not found
    typename std::map<std::string, T>::iterator iter = _map.find(name);
    if (iter == _map.end())
      mooseError("Unknown map key ", name);
    return iter->second;
  }

  ///@{
  /**
   * Provide iterator and find access to the underlying map data
   */
  typename std::map<std::string, T>::iterator begin() { return _map.begin(); }
  typename std::map<std::string, T>::iterator end() { return _map.end(); }
  typename std::map<std::string, T>::iterator find(const std::string & name)
  {
    return _map.find(name);
  }
  typename std::map<std::string, T>::const_iterator begin() const { return _map.begin(); }
  typename std::map<std::string, T>::const_iterator end() const { return _map.end(); }
  const typename std::map<std::string, T>::const_iterator find(const std::string & name) const
  {
    return _map.find(name);
  }
  ///@}

  /**
   * A method for testing of a key exists
   */
  bool contains(const std::string & name) { return find(name) != end(); }

protected:
  /// Data storage
  typename std::map<std::string, T> _map;
};

/**
 * A helper warehouse class for storing the "execute_on" settings for
 * the various output types.
 *
 * In order to allow for new output types to be defined and to minimize
 * the number of member variables the "execute_on" parameter for each of
 * the output types (e.g., execute_postprocessors_on) are stored in a map.
 *
 * This allows for iterative access to these parameters, which makes creating
 * generic code (e.g., AdvancedOutput::shouldOutput) possible. However, ExecFlagEnum
 * has a private constructor, so calling operator[] on the map is a compile time error.
 *
 * To get around this and to provide a more robust storage structure, one that will error
 * if the wrong output name is given, this warehouse was created. For the purposes of the
 * AdvancedOutput object this warehouse functions exactly like a std::map, but provides
 * an operator[] that works with ExecFlagEnum and errors if called on an invalid key.
 *
 * @see OutputMapWrapper OutputDataWarehouse
 */
class OutputOnWarehouse : public OutputMapWrapper<ExecFlagEnum>
{
public:
  /**
   * Constructor
   * @param execute_on The general "execute_on" settings for the object.
   * @param parameters The parameters object holding data for the class to use.
   */
  OutputOnWarehouse(const ExecFlagEnum & execute_on, const InputParameters & parameters);
};

/**
 * A helper warehouse for storing OutputData objects for the various output types
 *
 * To keep syntax consistent and to provide the error checking for accessing invalid map keys
 * the OutputMapWrapper is used for accessing the OutputData classes as well.
 *
 * @see OutputOnWarehouse OutputMapWrapper OutputData
 */
class OutputDataWarehouse : public OutputMapWrapper<OutputData>
{
public:
  static InputParameters validParams();

  /**
   * Populate the OutputData structures for all output types that are 'variable' based
   */
  OutputDataWarehouse();

  /**
   * False when the show lists for all variables is empty.
   *
   * When false everything should output.
   * @see AdvancedOutput::initOutputList
   */
  bool hasShowList() { return _has_show_list; }

  /**
   * Set the show list bool.
   *
   * This is set to true when the user supplies a show list.
   * @see AdvancedOutput::initShowHideLists
   */
  void setHasShowList(bool value) { _has_show_list = value; }

  /**
   * Clear existing lists for re-initialization
   */
  void reset();

private:
  // True when the input file contains a show/hide list
  bool _has_show_list;
};
