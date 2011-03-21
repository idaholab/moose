#include "PostprocessorInterface.h"
#include "PostprocessorData.h"
#include "SubProblem.h"

namespace Moose {

PostprocessorInterface::PostprocessorInterface(InputParameters & params) :
    _pi_problem(*params.get<Moose::SubProblem *>("_problem")),
    _pi_tid(params.have_parameter<THREAD_ID>("_tid") ? params.get<THREAD_ID>("_tid") : 0)
{}

PostprocessorValue &
PostprocessorInterface::getPostprocessorValue(const std::string & name)
{
  return _pi_problem.getPostprocessorValue(name, _pi_tid);
  
  
  // std::map<std::string, Real>::iterator it = _postprocessor_data._values.find(name);

//   if (it != _postprocessor_data._values.end())
//     return it->second;

  //mooseError("No Postprocessor named: " + name);
}

} // namespace
