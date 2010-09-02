/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
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
