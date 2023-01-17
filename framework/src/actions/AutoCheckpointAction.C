#include "AutoCheckpointAction.h"
#include "FEProblem.h"
#include "Checkpoint.h"
registerMooseAction("MooseApp", AutoCheckpointAction, "auto_checkpoint_action");

InputParameters
AutoCheckpointAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

AutoCheckpointAction::AutoCheckpointAction(const InputParameters & params) : Action(params) {}

void
AutoCheckpointAction::act()
{
  if (_app.isUltimateMaster())
  {
    if (!_app.getOutputWarehouse().getOutputs<Checkpoint>().empty())
      // if there's already a checkpoint object, we don't need to worry about creating a new
      // checkpoint
      return;

    auto cp_params = _factory.getValidParams("Checkpoint");
    cp_params.setParameters("should_output", false);
    cp_params.setParameters("is_autosave", true);
    _problem->addOutput("Checkpoint", "autosave", cp_params);
  }
}
