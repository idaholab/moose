#include "IntegrityCheckAction.h"

template<>
InputParameters validParams<IntegrityCheckAction>()
{
  InputParameters params = validParams<R7Action>();

  return params;
}

IntegrityCheckAction::IntegrityCheckAction(const std::string & name, InputParameters parameters) :
    R7Action(name, parameters)
{
}

IntegrityCheckAction::~IntegrityCheckAction()
{
}

void
IntegrityCheckAction::act()
{
  _simulation.integrityCheck();
}

