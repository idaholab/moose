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

// Standard includes
#include <string>

// MOOSE includes
#include "MooseTypes.h"

// Forward Declarations
class FEProblemBase;
class InputParameters;
class PostprocessorName;
class MooseObject;

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
   * Retrieve the value of a Postprocessor or one of it's old or older values
   * @param name The name of the Postprocessor parameter (see below)
   * @return A reference to the desired value
   *
   * The name required by this method is the name that is hard-coded into
   * your source code. For example, if you have a Kernel that requires
   * a Postprocessor you may have an input file with "pp = my_pp", this function
   * requires the "pp" name as input (see .../moose_test/functions/PostprocessorFunction.C)
   *
   * see getPostprocessorValueByName getPostprocessorValueOldByName getPostprocessorValueOlderByName
   */
  const PostprocessorValue & getPostprocessorValue(const std::string & name);
  const PostprocessorValue & getPostprocessorValueOld(const std::string & name);
  const PostprocessorValue & getPostprocessorValueOlder(const std::string & name);
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
  const PostprocessorValue & getPostprocessorValueByName(const PostprocessorName & name);
  const PostprocessorValue & getPostprocessorValueOldByName(const PostprocessorName & name);
  const PostprocessorValue & getPostprocessorValueOlderByName(const PostprocessorName & name);
  ///@}

  ///@{
  /**
   * Return the default postprocessor value
   * @param name The name of the postprocessor parameter
   * @return A const reference to the default value
   */
  const PostprocessorValue & getDefaultPostprocessorValue(const std::string & name);
  ///@}

  /**
   * Determine if the Postprocessor exists
   * @param name The name of the Postprocessor parameter
   * @return True if the Postprocessor exists
   *
   * @see hasPostprocessorByName getPostprocessorValue
   */
  bool hasPostprocessor(const std::string & name) const;

  /**
   * Determine if the Postprocessor exists
   * @param name The name of the Postprocessor
   * @return True if the Postprocessor exists
   *
   * @see hasPostprocessor getPostprocessorValueByName
   */
  bool hasPostprocessorByName(const PostprocessorName & name);

private:
  /// PostprocessorInterface Parameters
  const InputParameters & _ppi_params;

  /// Reference the the FEProblemBase class
  FEProblemBase & _pi_feproblem;
};

#endif // POSTPROCESSORINTERFACE_H
