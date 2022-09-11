//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <gtest/gtest.h>
#include "MooseObjectName.h"
#include "MooseTypes.h"
#include "ControllableItem.h"
#include "ControllableParameter.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "ControlOutput.h"

// Forward declarations
class InputParameters;

/**
 * Storage container for all InputParamter objects.
 *
 * This object is responsible for InputParameter objects, all MooseObjects should
 * contain a reference to the parameters object stored here.
 *
 * To avoid abuse, this warehouse is also designed to restrict the ability to change the parameter
 * to Control objects only.
 */
class InputParameterWarehouse
{
public:
  /**
   * Class constructor
   */
  InputParameterWarehouse();

  /**
   * Destruction
   */
  virtual ~InputParameterWarehouse() = default;

  ///@{
  /**
   * Return a const reference to the InputParameters for the named object
   * @param tag The tag of the object (e.g., 'Kernel')
   * @param name The name of the parameters object, including the tag (name only input) or
   * MooseObjectName object
   * @param tid The thread id
   * @return A const reference to the warehouse copy of the InputParameters
   */
  const InputParameters & getInputParametersObject(const std::string & name,
                                                   THREAD_ID tid = 0) const;
  const InputParameters & getInputParametersObject(const std::string & tag,
                                                   const std::string & name,
                                                   THREAD_ID tid = 0) const;
  const InputParameters & getInputParametersObject(const MooseObjectName & object_name,
                                                   THREAD_ID tid = 0) const;
  ///@}
  /**
   * Return const reference to the map containing the InputParameter objects
   */
  const std::multimap<MooseObjectName, std::shared_ptr<InputParameters>> &
  getInputParameters(THREAD_ID tid = 0) const;

  /**
   * Method for linking control parameters of different names
   */
  void addControllableParameterConnection(const MooseObjectParameterName & primary,
                                          const MooseObjectParameterName & secondary,
                                          bool error_on_empty = true);

  /**
   * Method for creating alias to an existing controllable parameters.
   *
   * @param alias The new name to serve as an alias.
   * @param secondary The name of the secondary parameter to be aliased.
   */
  void addControllableParameterAlias(const MooseObjectParameterName & alias,
                                     const MooseObjectParameterName & secondary);

  /**
   * Method for creating alias for all shared controllable parameters between the two objects.
   */
  void addControllableObjectAlias(const MooseObjectName & alias, const MooseObjectName & secondary);

  /***
   * Helper method for printing controllable items.
   */
  std::string dumpChangedControls(bool reset_changed) const;

  /**
   * Returns a copy of the current values for a controllable parameter. This method
   * is designed to provide access to objects for monitoring the state of a controllable parameter.
   */
  template <typename T>
  std::vector<T> getControllableParameterValues(const MooseObjectParameterName & input) const;

  /**
   * Return a vector of parameters names matching the supplied name.
   */
  std::vector<MooseObjectParameterName>
  getControllableParameterNames(const MooseObjectParameterName & input) const;

private:
  /// Storage for the InputParameters objects
  /// TODO: Remove multimap
  std::vector<std::multimap<MooseObjectName, std::shared_ptr<InputParameters>>> _input_parameters;

  /// Storage for controllable parameters via ControllableItem objects, a unique_ptr is
  /// used to avoid creating multiple copies. All access to the objects are done via
  /// pointers. The ControllableItem objects are not designed and will not be used directly in
  /// user code. All user level access goes through the ControllableParameter object.
  std::vector<std::vector<std::shared_ptr<ControllableItem>>> _controllable_items;

  /**
   * Method for adding a new InputParameters object
   * @param parameters The InputParameters object to copy and store in the warehouse
   * @return A reference to the warehouse copy of the InputParameters, this
   *         is what should be passed into the MooseObjects constructors.
   *
   * A new object is created from the old object because InputParameters objects
   * are generic until Factory::create() is called and the actual MooseObject
   * is created.
   *
   * This method is private, because only the factories that are creating objects should be
   * able to call this method.
   */
  InputParameters & addInputParameters(const std::string & name,
                                       const InputParameters & parameters,
                                       THREAD_ID tid = 0);

  /**
   * Allows for the deletion and cleanup of an object while the simulation is running.
   */
  void removeInputParameters(const MooseObject & moose_object, THREAD_ID tid = 0);

  /**
   * Returns a ControllableParameter object that contains all matches to ControllableItem objects
   * for the provided name.
   *
   * This is private because it should only be accessed via a Control object.
   */
  ControllableParameter getControllableParameter(const MooseObjectParameterName & input) const;

  /**
   * Returns a ControllableItem iterator, if the name is located.
   * @see Control
   */
  std::vector<ControllableItem *> getControllableItems(const MooseObjectParameterName & desired,
                                                       THREAD_ID tid = 0) const;

  ///@{
  /**
   * Return a reference to the InputParameters for the named object
   * @param tag The tag of the object (e.g., 'Kernel')
   * @param name The name of the parameters object, including the tag (name only input) or
   * MooseObjectName object
   * @param tid The thread id
   * @return A reference to the warehouse copy of the InputParameters
   *
   * If you are using this method to access a writable reference to input parameters, this
   * will break the ability to control the parameters with the MOOSE control logic system.
   * Only change parameters if you know what you are doing. Hence, this is private for a reason.
   */
  InputParameters & getInputParameters(const std::string & name, THREAD_ID tid = 0) const;
  InputParameters &
  getInputParameters(const std::string & tag, const std::string & name, THREAD_ID tid = 0) const;
  InputParameters & getInputParameters(const MooseObjectName & object_name,
                                       THREAD_ID tid = 0) const;
  ///@}

  ///@{
  /// The factory is allowed to call addInputParameters and removeInputParameters.
  friend MooseObjectPtr Factory::create(
      const std::string &, const std::string &, const InputParameters &, THREAD_ID, bool);
  friend std::shared_ptr<Action>
  ActionFactory::create(const std::string &, const std::string &, InputParameters &);
  friend void Factory::releaseSharedObjects(const MooseObject &, THREAD_ID);
  ///@}

  /// Only controls are allowed to call getControllableParameter. The
  /// Control::getControllableParameter is the only method that calls getControllableParameter.
  /// However, this method cannot be made a friend explicitly because the method would need to be
  /// public. If the method is public then it is possible to call the method by getting access to
  /// the Control object.
  friend class Control;

  // Allow unit test to call methods
  FRIEND_TEST(InputParameterWarehouse, getControllableItems);
  FRIEND_TEST(InputParameterWarehouse, getControllableParameter);
  FRIEND_TEST(InputParameterWarehouse, getControllableParameterValues);
  FRIEND_TEST(InputParameterWarehouse, emptyControllableParameterValues);
  FRIEND_TEST(InputParameterWarehouse, addControllableParameterConnection);
  FRIEND_TEST(InputParameterWarehouse, addControllableParameterAlias);
};

template <typename T>
std::vector<T>
InputParameterWarehouse::getControllableParameterValues(
    const MooseObjectParameterName & input) const
{
  ControllableParameter param = getControllableParameter(input);
  return param.get<T>();
}
