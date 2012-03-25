#include "AddBoundsVectorsAction.h"
#include "Parser.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddBoundsVectorsAction>()
{
  return validParams<Action>();
}

AddBoundsVectorsAction::AddBoundsVectorsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddBoundsVectorsAction::act()
{
  _parser_handle._problem->getNonlinearSystem().addVector("lower_bound", false, GHOSTED, true);
  _parser_handle._problem->getNonlinearSystem().addVector("upper_bound", false, GHOSTED, true);
}
