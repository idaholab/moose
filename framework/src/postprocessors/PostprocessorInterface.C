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

#include "PostprocessorInterface.h"

#include "PostprocessorData.h"
#include "MooseSystem.h"

PostprocessorInterface::PostprocessorInterface(PostprocessorData & postprocessor_data):
  _postprocessor_data(postprocessor_data)
{}

PostprocessorValue &
PostprocessorInterface::getPostprocessorValue(const std::string & name)
{
  return _postprocessor_data.getPostprocessorValue(name);
  
  
  // std::map<std::string, Real>::iterator it = _postprocessor_data._values.find(name);

//   if (it != _postprocessor_data._values.end())
//     return it->second;

  //mooseError("No Postprocessor named: " + name);
}
