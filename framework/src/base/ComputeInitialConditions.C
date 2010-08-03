#include "Moose.h"
#include "MooseSystem.h"
#include "InitialCondition.h"

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
    return moose_system->initial_value(p, parameters, sys_name, var_name);
  }

  Gradient initial_gradient (const Point& p,
                             const Parameters& parameters,
                             const std::string& sys_name,
                             const std::string& var_name)
  {
    MooseSystem * moose_system = parameters.get<MooseSystem *>("moose_system");
    mooseAssert(moose_system != NULL, "Internal pointer to MooseSystem was not set");
    return moose_system->initial_gradient(p, parameters, sys_name, var_name);
  }

  void initial_condition(EquationSystems& es, const std::string& system_name)
  {
    MooseSystem * moose_system = es.parameters.get<MooseSystem *>("moose_system");
    mooseAssert(moose_system != NULL, "Internal pointer to MooseSystem was not set");
    moose_system->initial_condition(es, system_name);
  }
}


Number MooseSystem::initial_value (const Point& p,
                      const Parameters& parameters,
                      const std::string& /*sys_name*/,
                      const std::string& var_name)
{
  // Try to grab an InitialCondition object for this variable.
  InitialCondition * ic = _ics[0].getInitialCondition(var_name);

  if(ic)
    return ic->value(p);

  if(parameters.have_parameter<Real>("initial_"+var_name))
    return parameters.get<Real>("initial_"+var_name);

  return 0;
}

Gradient MooseSystem::initial_gradient (const Point& p,
                           const Parameters& /*parameters*/,
                           const std::string& /*sys_name*/,
                           const std::string& var_name)
{
  // Try to grab an InitialCondition object for this variable.
  InitialCondition * ic = _ics[0].getInitialCondition(var_name);

  if(ic)
    return ic->gradient(p);

  return RealGradient();
}

void MooseSystem::initial_condition(EquationSystems& es, const std::string& system_name)
{
  ExplicitSystem & system = _es->get_system<ExplicitSystem>(system_name);

  system.project_solution(Moose::initial_value, Moose::initial_gradient, es.parameters);
}
