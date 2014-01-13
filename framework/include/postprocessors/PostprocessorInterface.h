/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef POSTPROCESSORINTERFACE_H
#define POSTPROCESSORINTERFACE_H

#include <map>
#include <string>

#include "InputParameters.h"
#include "ParallelUniqueId.h"
#include "PostprocessorData.h"

// Forward Declarations
class FEProblem;

class PostprocessorInterface
{
public:
  PostprocessorInterface(InputParameters & params);

  /**
   * Retrieve the value of a Postprocessor
   * @param name The name of the Postprocessor (see below)
   * @return A reference to the desired value
   *
   * The name required by this method is the name that is hard-coded into
   * your source code. For example, if you have a Kernel that requires
   * a Postprocessor you may have an input file with "pp = my_pp", this function
   * requires the "pp" name as input (see .../moose_test/functions/PostprocessorFunction.C)
   *
   * see getPostprocessorValueOld getPostprocessorValueByName getPostprocessorValueOldByName
   */
  const PostprocessorValue & getPostprocessorValue(const std::string & name);

  /**
   * Retrieve value of a postprocessor or a default
   * @param name The name of the Postprocessor
   * @param d_value The default value
   *
   * This function will return a constant reference to the postprocessor value or in the
   * case that the postprocessor does not exists (hasPostprocessor(name) == false) a
   * constant reference to the value provided in d_value will be returned. The default value
   * returned is local to the object, that is, if getPostprocessorValue if called from another
   * object it will not return the default value set by another.
   *
   * @see getPostprocessorValue
   */
  const PostprocessorValue & getPostprocessorValue(const std::string & name, Real d_value);

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
   * see getPostprocessorValue getPostprocessorValueOldByName getPostprocessorValueByName
   */
  const PostprocessorValue & getPostprocessorValueByName(const PostprocessorName & name);

  /**
   * Retrieve value of a postprocessor or a default
   * @param name The name of the Postprocessor
   * @param d_value The default value
   *
   * This function will return a constant reference to the postprocessor value or in the
   * case that the postprocessor does not exists (hasPostprocessor(name) == false) a
   * constant reference to the value provided in d_value will be returned. The default value
   * returned is local to the object, that is, if getPostprocessorValue if called from another
   * object it will not return the default value set by another.
   *
   * @see getPostprocessorValueByName
   */
  const PostprocessorValue & getPostprocessorValueByName(const std::string & name, Real d_value);

  /**
   * Retrieve the old value of a Postprocessor
   * @param name The name of the Postprocessor
   * @return The value of the Postprocessor
   *
   * see getPostprocessorValue
   */
  const PostprocessorValue & getPostprocessorValueOld(const std::string & name);

  /**
   * Retrieve the old value of a Postprocessor
   * @param name The name of the Postprocessor
   * @param d_value The default value to utilize if the postprocessor does not exist
   * @return The value of the Postprocessor
   *
   * This function will return a constant reference to the postprocessor value or in the
   * case that the postprocessor does not exists (hasPostprocessor(name) == false) a
   * constant reference to the value provided in d_value will be returned. The default value
   * returned is local to the object, that is, if getPostprocessorValue if called from another
   * object it will not return the default value set by another.
   *
   * see getPostprocessorValue
   */
  const PostprocessorValue & getPostprocessorValueOld(const std::string & name, Real d_value);

  /**
   * Retrieve the old value of a Postprocessor
   * @param name The name of the Postprocessor
   * @return The value of the Postprocessor
   *
   * see getPostprocessorValueByName
   */
  const PostprocessorValue & getPostprocessorValueOldByName(const PostprocessorName & name);

  /**
   * Retrieve the old value of a Postprocessor
   * @param name The name of the Postprocessor
   * @param d_value The default value to utilize if the postprocessor does not exist
   * @return The value of the Postprocessor
   *
   * This function will return a constant reference to the postprocessor value or in the
   * case that the postprocessor does not exists (hasPostprocessor(name) == false) a
   * constant reference to the value provided in d_value will be returned. The default value
   * returned is local to the object, that is, if getPostprocessorValue if called from another
   * object it will not return the default value set by another.
   *
   * see getPostprocessorValueByName
   */
  const PostprocessorValue & getPostprocessorValueOldByName(const std::string & name, Real d_value);

  /**
   * Determine if the postprocessor exists
   * @param name The name of the Postprocessor
   * @return True if the Postprocessor exists
   */
  bool hasPostprocessor(const std::string & name);

private:

  /// Reference the the FEProblem class
  FEProblem & _pi_feproblem;

  /// Thread ID
  THREAD_ID _pi_tid;

  /// PostprocessorInterface Parameters
  InputParameters _ppi_params;

  /// Storage for default values
  std::map<std::string, PostprocessorValue> _default_postprocessor_value;

  /// Storage for default old values
  std::map<std::string, PostprocessorValue> _default_postprocessor_old_value;

};

#endif //POSTPROCESSORINTERFACE_H
