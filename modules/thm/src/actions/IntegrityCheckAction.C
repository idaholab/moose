#include "IntegrityCheckAction.h"
#include "Relap7App.h"

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
  Relap7App & app = dynamic_cast<Relap7App &>(_app);

  if (!app.checkJacobian())
    _simulation.integrityCheck();
}

