//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Standard includes
#include <string>

// MOOSE includes
#include "MooseTypes.h"

// Forward Declarations
class FEProblemBase;
class InputParameters;
class PostprocessorName;
class MooseObject;

#define usingPostprocessorInterfaceMembers                                                         \
  using PostprocessorInterface::getPostprocessorValue;                                             \
  using PostprocessorInterface::getPostprocessorValueOld;                                          \
  using PostprocessorInterface::getPostprocessorValueOlder;                                        \
  using PostprocessorInterface::coupledPostprocessors

/**
 * Interface class for classes which interact with Postprocessors.
 * Provides the getPostprocessorValueXYZ() and related interfaces.
 */
class PostprocessorInterface
{
public:
  PostprocessorInterface(const MooseObject * moose_object);

  PostprocessorInterface(const FEProblemBase * problem);

  static InputParameters validParams();

  ///@{
  /**
   * doco-normal-methods-begin
   * Retrieve the value of a Postprocessor or one of it's old or older values
   * @param param_name The name of the Postprocessor parameter (see below)
   * @param index The index of the Postprocessor
   * @return A reference to the desired value
   *
   * The name required by this method is the name that is hard-coded into
   * your source code. For example, if you have a Kernel that requires
   * a Postprocessor you may have an input file with "pp = my_pp", this function
   * requires the "pp" name as input (see .../moose_test/functions/PostprocessorFunction.C)
   *
   * see getPostprocessorValueByName getPostprocessorValueOldByName getPostprocessorValueOlderByName
   */
  const PostprocessorValue & getPostprocessorValue(const std::string & param_name,
                                                   const unsigned int index = 0) const;
  const PostprocessorValue & getPostprocessorValueOld(const std::string & param_name,
                                                      const unsigned int index = 0) const;
  const PostprocessorValue & getPostprocessorValueOlder(const std::string & param_name,
                                                        const unsigned int index = 0) const;
  // doco-normal-methods-end

  ///@}

  ///@{
  /**
   * Retrieve the value of the Postprocessor
   * @param name Postprocessor name (see below)
   * @return A reference to the desired value
   *
   * The name required by this method is the name defined in the input file. For example,
   * if you have a Kernel that requires a Postprocessor you may have an input file with
   * "pp = my_pp", this method requires the "my_pp" name as input
   * (see .../moose_test/functions/PostprocessorFunction.C)
   *
   * see getPostprocessorValue getPostprocessorValueOld getPostprocessorValueOlder
   */
  virtual const PostprocessorValue &
  getPostprocessorValueByName(const PostprocessorName & name) const;
  const PostprocessorValue & getPostprocessorValueOldByName(const PostprocessorName & name) const;
  const PostprocessorValue & getPostprocessorValueOlderByName(const PostprocessorName & name) const;
  ///@}

  /**
   * Determine whether or not the Postprocessor is a default value. A default value is when
   * the value is either the value set by addParam, or is a user-set value in input instead of
   * a name to a postprocessor.
   * @param param_name The name of the Postprocessor parameter
   * @param index The index of the postprocessor
   * @return True if the Postprocessor is a default value, false if the Postprocessor
   * is the name of a Postprocessor
   */
  bool isDefaultPostprocessorValue(const std::string & param_name,
                                   const unsigned int index = 0) const;

  /**
   * Determine if the Postprocessor data exists
   * @param param_name The name of the Postprocessor parameter
   * @param index The index of the Postprocessor
   * @return True if the Postprocessor exists
   *
   * @see hasPostprocessorByName getPostprocessorValue
   */
  bool hasPostprocessor(const std::string & param_name, const unsigned int index = 0) const;

  /**
   * Determine if the Postprocessor data exists
   * @param name The name of the Postprocessor
   * @return True if the Postprocessor exists
   *
   * @see hasPostprocessor getPostprocessorValueByName
   */
  bool hasPostprocessorByName(const PostprocessorName & name) const;

  /**
   * Returns number of Postprocessors coupled under parameter name
   * @param param_name The name of the Postprocessor parameter
   * @return Number of coupled post-processors, 1 if it's a single
   *
   */
  std::size_t coupledPostprocessors(const std::string & param_name) const;

  /**
   * Get the name of a postprocessor. This can only be used if the postprocessor
   * parameter does _not_ have a default value set (see isDefaultPostprocessorValue()),
   * in which case the "name" is actually the default value.
   * @param param_name The name of the Postprocessor parameter
   * @param index The index of the Postprocessor
   * @return The name of the given Postprocessor
   */
  const PostprocessorName & getPostprocessorName(const std::string & param_name,
                                                 const unsigned int index = 0) const;

protected:
  /**
   * Helper for deriving classes to override to add dependencies when a Postprocessor is requested.
   */
  virtual void addPostprocessorDependencyHelper(const PostprocessorName & /* name */) const {}

private:
  /// The MooseObject that uses this interface
  const MooseObject & _ppi_moose_object;

  /// PostprocessorInterface Parameters
  const InputParameters & _ppi_params;

  /// Reference the the FEProblemBase class
  const FEProblemBase & _ppi_feproblem;

  /// Holds the default postprocessor values that are requested (key is PostprocessorName)
  mutable std::map<PostprocessorName, std::unique_ptr<PostprocessorValue>> _default_values;

  /**
   * Internal method for getting the PostprocessorName associated with a paremeter.
   * Needed in order to allow the return of a name that is a default value.
   */
  const PostprocessorName &
  getPostprocessorNameInternal(const std::string & param_name,
                               const unsigned int index,
                               const bool allow_default_value = true) const;

  /**
   * Internal methods for getting Postprocessor values.
   */
  ///@{
  const PostprocessorValue & getPostprocessorValueInternal(const std::string & param_name,
                                                           unsigned int index,
                                                           std::size_t t_index) const;
  const PostprocessorValue & getPostprocessorValueByNameInternal(const PostprocessorName & name,
                                                                 std::size_t t_index) const;
  ///@}

  /**
   * @returns True if the PostprocessorName \p name repesents a default value: the name
   * converts to a value (set by addParam or set via input), and a Postprocessor does not
   * exist with the same name (we do allow Postprocessors with numbered names...)
   */
  bool isDefaultPostprocessorValueByName(const PostprocessorName & name) const;

  /**
   * @returns The default value stored in the PostprocessorName \p name.
   */
  PostprocessorValue getDefaultPostprocessorValueByName(const PostprocessorName & name) const;

  /**
   * Checks the parameters relating to a Postprocessor. If \p index is not set, index
   * checking is not performed.
   */
  void checkParam(const std::string & param_name,
                  const unsigned int index = std::numeric_limits<unsigned int>::max()) const;

  /**
   * @returns True if all pps have been added (the task associated with adding them is complete)
   */
  bool postprocessorsAdded() const;
};
