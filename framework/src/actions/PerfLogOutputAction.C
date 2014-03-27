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

// MOOSE includes
#include "PerfLogOutputAction.h"
#include "OutputBase.h"
#include "Console.h"
#include "MooseApp.h"

template<>
InputParameters validParams<PerfLogOutputAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

PerfLogOutputAction::PerfLogOutputAction(const std::string & name, InputParameters params) :
  Action(name, params)
{
}

PerfLogOutputAction::~PerfLogOutputAction()
{
}

void
PerfLogOutputAction::act()
{

  // Search for the existence of a Console outputter
  bool has_console = false;
  const std::vector<OutputBase *> & ptrs = _app.getOutputWarehouse().getOutputs();
  for (std::vector<OutputBase *>::const_iterator it = ptrs.begin(); it != ptrs.end(); ++it)
  {
    Console * c_ptr = dynamic_cast<Console *>(*it);
    if (c_ptr != NULL)
    {
      has_console = true;
      break;
    }
  }

  /* If a Console outputter is found then all the correct handling of performance logs are
     handled within the object(s), so do nothing */
  if (!has_console)
  {
    Moose::perf_log.disable_logging();
    Moose::setup_perf_log.disable_logging();
#ifdef LIBMESH_ENABLE_PERFORMANCE_LOGGING
    libMesh::perflog.disable_logging();
#endif
  }

  // If the --timing option is used from the command-line, enable all logging
  if (_app.getParam<bool>("timing"))
  {
    Moose::perf_log.enable_logging();
    Moose::setup_perf_log.enable_logging();
#ifdef LIBMESH_ENABLE_PERFORMANCE_LOGGING
    libMesh::perflog.enable_logging();
#endif
  }
}
