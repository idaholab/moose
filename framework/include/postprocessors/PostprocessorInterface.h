#ifndef POSTPROCESSORINTERFACE_H
#define POSTPROCESSORINTERFACE_H

#include <map>
#include <string>

#include "Moose.h"
#include "PostprocessorData.h"

// Forward Declarations

class PostprocessorInterface
{
public:
  PostprocessorInterface(PostprocessorData & postprocessor_data);

  /**
   * Retrieve the value named "name"
   */
  PostprocessorValue & getPostprocessorValue(const std::string & name);

private:
  PostprocessorData & _postprocessor_data;
};

#endif //POSTPROCESSORINTERFACE_H
