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
    // if there's already a checkpoint object, we don't need to worry about creating a new
    // checkpoint
    if (!_app.getOutputWarehouse().getOutputs<Checkpoint>().empty())
    {

      // Take the first checkpoint object, since we only need to/should make one object the autosave
      _app.getOutputWarehouse().getOutputs<Checkpoint>()[0]->setAutosaveFlag(
          AutosaveType::MODIFIED_EXISTING);
      return;
    }

    // If there isn't an existing one, init a new one
    auto cp_params = _factory.getValidParams("Checkpoint");
    cp_params.setParameters("is_autosave", AutosaveType::SYSTEM_AUTOSAVE);
    cp_params.setParameters("file_base", std::string("autosave"));
    _problem->addOutput("Checkpoint", "autosave", cp_params);
  }
}
