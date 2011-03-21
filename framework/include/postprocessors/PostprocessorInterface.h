#ifndef POSTPROCESSORINTERFACE_H_
#define POSTPROCESSORINTERFACE_H_

#include <map>
#include <string>

#include "Moose.h"
#include "InputParameters.h"
#include "ParallelUniqueId.h"

// Forward Declarations
namespace Moose {
  class Problem;
}

class PostprocessorInterface
{
public:
  PostprocessorInterface(InputParameters & params);

  /**
   * Retrieve the value named "name"
   */
  PostprocessorValue & getPostprocessorValue(const std::string & name);

private:
  Moose::Problem & _pi_problem;
  THREAD_ID _pi_tid;
};

#endif //POSTPROCESSORINTERFACE_H_
