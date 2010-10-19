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

#include "Moose.h"
#include "MooseSystem.h"
#include "InitialCondition.h"
#include "ParallelUniqueId.h"

// libMesh includes
#include "libmesh.h"
#include "point.h"
#include "InputParameters.h"
#include "vector_value.h"
#include "equation_systems.h"
#include "explicit_system.h"

namespace Moose
{
  Number initial_value (const Point& p,
                        const Parameters& parameters,
                        const std::string& sys_name,
                        const std::string& var_name)
  {
    MooseSystem * moose_system = parameters.get<MooseSystem *>("moose_system");
    mooseAssert(moose_system != NULL, "Internal pointer to MooseSystem was not set");
    return moose_system->initialValue(p, parameters, sys_name, var_name);
  }

  Gradient initial_gradient (const Point& p,
                             const Parameters& parameters,
                             const std::string& sys_name,
                             const std::string& var_name)
  {
    MooseSystem * moose_system = parameters.get<MooseSystem *>("moose_system");
    mooseAssert(moose_system != NULL, "Internal pointer to MooseSystem was not set");
    return moose_system->initialGradient(p, parameters, sys_name, var_name);
  }

  void initial_condition(EquationSystems& es, const std::string& system_name)
  {
    MooseSystem * moose_system = es.parameters.get<MooseSystem *>("moose_system");
    mooseAssert(moose_system != NULL, "Internal pointer to MooseSystem was not set");
    moose_system->initialCondition(es, system_name);
  }
}


Number MooseSystem::initialValue (const Point& p,
                      const Parameters& parameters,
                      const std::string& /*sys_name*/,
                      const std::string& var_name)
{
  ParallelUniqueId puid;
  unsigned int tid = puid.id;

  // Try to grab an InitialCondition object for this variable.
  InitialCondition * ic = _ics[tid].getInitialCondition(var_name);

  if(ic)
    return ic->value(p);

  if(parameters.have_parameter<Real>("initial_"+var_name))
    return parameters.get<Real>("initial_"+var_name);

  return 0;
}

Gradient MooseSystem::initialGradient (const Point& p,
                           const Parameters& /*parameters*/,
                           const std::string& /*sys_name*/,
                           const std::string& var_name)
{
  ParallelUniqueId puid;
  unsigned int tid = puid.id;

  // Try to grab an InitialCondition object for this variable.
  InitialCondition * ic = _ics[tid].getInitialCondition(var_name);

  if(ic)
    return ic->gradient(p);

  return RealGradient();
}

void MooseSystem::initialCondition(EquationSystems& es, const std::string& system_name)
{
  ExplicitSystem & system = _es->get_system<ExplicitSystem>(system_name);

  system.project_solution(Moose::initial_value, Moose::initial_gradient, es.parameters);
}
