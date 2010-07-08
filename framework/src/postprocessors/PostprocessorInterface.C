#include "PostprocessorInterface.h"

#include "PostprocessorData.h"
#include "MooseSystem.h"

PostprocessorInterface::PostprocessorInterface(PostprocessorData & postprocessor_data):
  _postprocessor_data(postprocessor_data)
{}

PostprocessorValue &
PostprocessorInterface::getPostprocessorValue(const std::string & name)
{
  std::map<std::string, Real>::iterator it = _postprocessor_data._values.find(name);

  if (it != _postprocessor_data._values.end())
    return it->second;

  mooseError("No Postprocessor named: " + name);
}
