#include "SubProblem.h"
#include "Factory.h"
#include "ImplicitSystem.h"

// libMesh includes

namespace Moose
{

Number initial_value (const Point& p,
                      const Parameters& parameters,
                      const std::string& sys_name,
                      const std::string& var_name)
{
  Problem * problem = parameters.get<Problem *>("_problem");
  mooseAssert(problem != NULL, "Internal pointer to _problem was not set");
  return problem->initialValue(p, parameters, sys_name, var_name);
}

Gradient initial_gradient (const Point& p,
                           const Parameters& parameters,
                           const std::string& sys_name,
                           const std::string& var_name)
{
  Problem * problem = parameters.get<Problem *>("_problem");
  mooseAssert(problem != NULL, "Internal pointer to _problem was not set");
  return problem->initialGradient(p, parameters, sys_name, var_name);
}

void initial_condition(EquationSystems& es, const std::string& system_name)
{
  Problem * problem = es.parameters.get<Problem *>("_problem");
  mooseAssert(problem != NULL, "Internal pointer to MooseSystem was not set");
  problem->initialCondition(es, system_name);
}


SubProblem::SubProblem(Mesh &mesh, Problem * parent) :
    _parent(parent == NULL ? this : parent),
    _mesh(mesh),
    _eq(_mesh),
    _transient(false),
    _time(_parent ? _parent->time() : _eq.parameters.set<Real>("time")),
    _t_step(_parent ? _parent->timeStep() : _eq.parameters.set<int>("t_step")),
    _dt(_parent ? _parent->dt() : _eq.parameters.set<Real>("dt"))
{
  if (!_parent)
  {
    _time = 0.0;
    _t_step = 0;
    _dt = 0;
    _dt_old = _dt;
  }

//  _eq.parameters.set<SubProblem *>("_subproblem") = this;
//  _eq.parameters.set<Problem *>("_problem") = this;

  _functions.resize(libMesh::n_threads());
}

SubProblem::~SubProblem()
{
}

void
SubProblem::init()
{
  _eq.init();
  _eq.print_info();
}

void
SubProblem::update()
{
  for (std::vector<System *>::iterator it = _sys.begin(); it != _sys.end(); ++it)
    (*it)->update();
}

void
SubProblem::solve()
{
  for (std::vector<System *>::iterator it = _sys.begin(); it != _sys.end(); ++it)
    (*it)->solve();
}

bool
SubProblem::hasVariable(const std::string & var_name)
{
  for (unsigned int i = 0; i < _sys.size(); i++)
  {
    if (_sys[i]->hasVariable(var_name))
      return true;
  }
  return false;
}

Variable &
SubProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  for (unsigned int i = 0; i < _sys.size(); i++)
    if (_sys[i]->hasVariable(var_name))
      return _sys[i]->getVariable(tid, var_name);
  mooseError("Unknown variable " + var_name);
}

void
SubProblem::copySolutionsBackwards()
{
}

// Initial Conditions /////

void
SubProblem::addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters, std::string var_name)
{
  parameters.set<std::string>("var_name") = var_name;
  _ics[var_name] = static_cast<InitialCondition *>(Factory::instance()->create(ic_name, name, parameters));
}

void
SubProblem::addInitialCondition(const std::string & var_name, Real value)
{
  _pars.set<Real>("initial_" + var_name) = value;
}

void
SubProblem::addFunction(std::string type, const std::string & name, InputParameters parameters)
{
//  parameters.set<SubProblem *>("_subproblem") = this;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    Function * func = static_cast<Function *>(Factory::instance()->create(type, name, parameters));
    _functions[tid][name] = func;
  }
}

Function &
SubProblem::getFunction(const std::string & name, THREAD_ID tid)
{
  return *_functions[tid][name];
}

Number
SubProblem::initialValue (const Point& p,
                       const Parameters& /*parameters*/,
                       const std::string& /*sys_name*/,
                       const std::string& var_name)
{
//  ParallelUniqueId puid;
//  unsigned int tid = puid.id;

  // Try to grab an InitialCondition object for this variable.
  if (_ics.find(var_name) != _ics.end())
    return _ics[var_name]->value(p);

  if (_pars.have_parameter<Real>("initial_"+var_name))
    return _pars.get<Real>("initial_"+var_name);

  return 0;
}

Gradient
SubProblem::initialGradient (const Point& p,
                          const Parameters& /*parameters*/,
                          const std::string& /*sys_name*/,
                          const std::string& var_name)
{
//  ParallelUniqueId puid;
//  unsigned int tid = puid.id;

  // Try to grab an InitialCondition object for this variable.
  if (_ics.find(var_name) != _ics.end())
    return _ics[var_name]->gradient(p);

  return RealGradient();
}

void
SubProblem::initialCondition(EquationSystems& es, const std::string& system_name)
{
  ExplicitSystem & system = parent()->es().get_system<ExplicitSystem>(system_name);
  system.project_solution(Moose::initial_value, Moose::initial_gradient, es.parameters);
}

void
SubProblem::updateMaterials()
{

}

void
SubProblem::dump()
{
}

} // namespace
