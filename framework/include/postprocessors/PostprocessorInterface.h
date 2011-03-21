#ifndef POSTPROCESSORINTERFACE_H
#define POSTPROCESSORINTERFACE_H

#include <map>
#include <string>

#include "Moose.h"
#include "InputParameters.h"
#include "ParallelUniqueId.h"

// Forward Declarations
class SubProblem;

class PostprocessorInterface
{
public:
  PostprocessorInterface(InputParameters & params);

  /**
   * Retrieve the value named "name"
   */
  PostprocessorValue & getPostprocessorValue(const std::string & name);

private:
  SubProblem & _pi_problem;
  THREAD_ID _pi_tid;
};

#endif //POSTPROCESSORINTERFACE_H_
