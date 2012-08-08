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

#ifndef FUNCTIONINTERFACE_H
#define FUNCTIONINTERFACE_H

#include "InputParameters.h"
#include "ParallelUniqueId.h"

class Function;
class FEProblem;

/**
 * Interface for objects that need to use functions
 *
 * Inherit from this class at a very low level to make the getFunction method
 * available.
 */
class FunctionInterface
{
public:
  /**
   * @param params The parameters used by the object being instantiated. This
   *        class needs them so it can get the function named in the input file,
   *        but the object calling getFunction only needs to use the name on the
   *        left hand side of the statement "function = func_name"
   */
  FunctionInterface(InputParameters & params);

  /**
   * Get a function with a given name
   * @param name The name of the parameter key of the function to retrieve
   * @return The function with name associated with the parameter 'name'
   */
  Function & getFunction(const std::string & name);

  /**
   * Get a function with a given name
   * @param name The name of the function to retrieve
   * @return The function with name 'name'
   */
  Function & getFunctionByName(const std::string & name);

private:
  FEProblem & _fni_feproblem;
  /// Thread ID
  THREAD_ID _fni_tid;
  /// Parameters of the object with this interface
  InputParameters _fni_params;
};

#endif //FUNCTIONINTERFACE_H
