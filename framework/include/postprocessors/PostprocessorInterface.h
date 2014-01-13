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

  /**,
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
   * @return The value of the Postprocessor
   *
   * see getPostprocessorValueByName
   */
  const PostprocessorValue & getPostprocessorValueOldByName(const PostprocessorName & name);

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
};

#endif //POSTPROCESSORINTERFACE_H
