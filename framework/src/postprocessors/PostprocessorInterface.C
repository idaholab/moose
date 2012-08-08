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

#include "PostprocessorInterface.h"
#include "PostprocessorData.h"
#include "FEProblem.h"

PostprocessorInterface::PostprocessorInterface(InputParameters & params) :
    _pi_feproblem(*params.get<FEProblem *>("_fe_problem")),
    _pi_tid(params.have_parameter<THREAD_ID>("_tid") ? params.get<THREAD_ID>("_tid") : 0)
{}

PostprocessorValue &
PostprocessorInterface::getPostprocessorValue(const std::string & name)
{
  return _pi_feproblem.getPostprocessorValue(name, _pi_tid);


  // std::map<std::string, Real>::iterator it = _postprocessor_data._values.find(name);

//   if (it != _postprocessor_data._values.end())
//     return it->second;

  //mooseError("No Postprocessor named: " + name);
}
