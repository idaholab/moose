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

#include "Moose.h"
#include "InputParameters.h"
#include "ParallelUniqueId.h"

// Forward Declarations
class Problem;

class PostprocessorInterface
{
public:
  PostprocessorInterface(InputParameters & params);

  /**
   * Retrieve the value named "name"
   */
  PostprocessorValue & getPostprocessorValue(const std::string & name);

private:
  Problem & _pi_problem;
  THREAD_ID _pi_tid;
};

#endif //POSTPROCESSORINTERFACE_H
