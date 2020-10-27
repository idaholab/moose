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

  ///@{
  /**
   * doco-normal-methods-begin
   * Retrieve the value of a Postprocessor or one of it's old or older values
   * @param name The name of the Postprocessor parameter (see below)
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
  const PostprocessorValue & getPostprocessorValue(const std::string & name,
                                                   unsigned int index = 0) const;
  const PostprocessorValue & getPostprocessorValueOld(const std::string & name,
                                                      unsigned int index = 0) const;
  const PostprocessorValue & getPostprocessorValueOlder(const std::string & name,
                                                        unsigned int index = 0) const;
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
  const PostprocessorValue & getPostprocessorValueByName(const PostprocessorName & name) const;
  const PostprocessorValue & getPostprocessorValueOldByName(const PostprocessorName & name) const;
  const PostprocessorValue & getPostprocessorValueOlderByName(const PostprocessorName & name) const;
  ///@}

  ///@{
  /**
   * Return the default postprocessor value
   * @param name The name of the postprocessor parameter
   * @return A const reference to the default value
   */
  const PostprocessorValue & getDefaultPostprocessorValue(const std::string & name) const;
  ///@}

  /**
   * Determine if the Postprocessor data exists
   * @param name The name of the Postprocessor parameter
   * @param index The index of the Postprocessor
   * @return True if the Postprocessor exists
   *
   * @see hasPostprocessorByName getPostprocessorValue
   */
  bool hasPostprocessor(const std::string & name, unsigned int index = 0) const;

  /**
   * Determine if the Postprocessor data exists
   * @param name The name of the Postprocessor
   * @return True if the Postprocessor exists
   *
   * @see hasPostprocessor getPostprocessorValueByName
   */
  bool hasPostprocessorByName(const PostprocessorName & name) const;

  /**
   * Determine if the Postprocessor object exists
   * @param name The name of the Postprocessor parameter
   * @param index The index of the Postprocessor
   * @return True if the Postprocessor exists
   */
  bool hasPostprocessorObject(const std::string & name, unsigned int index = 0) const;

  /**
   * Determine if the Postprocessor object exists
   * @param name The name of the Postprocessor
   * @return True if the Postprocessor exists
   */
  bool hasPostprocessorObjectByName(const PostprocessorName & name) const;

  /**
   * Returns number of Postprocessors coupled under parameter name
   * @param name The name of the Postprocessor parameter
   * @return Number of coupled post-processors, 1 if it's a single
   *
   */
  unsigned int coupledPostprocessors(const std::string & name) const;

  /**
   * Checks if there is a single postprocessor coupled by parameter name
   * @param name The name of the Postprocessor parameter
   * @return Number of coupled post-processors, 1 if it's a single
   *
   */
  bool singlePostprocessor(const std::string & name) const;

private:
  /// PostprocessorInterface Parameters
  const InputParameters & _ppi_params;

  /// Reference the the FEProblemBase class
  FEProblemBase & _pi_feproblem;

  /// Extract the value using parameter name
  const PostprocessorValue & getPostprocessorValueHelper(const std::string & name,
                                                         unsigned int index,
                                                         std::size_t t_index) const;

  /// Extract the value using stored name
  const PostprocessorValue & getPostprocessorValueByNameHelper(const PostprocessorName & name,
                                                               std::size_t t_index) const;
};
