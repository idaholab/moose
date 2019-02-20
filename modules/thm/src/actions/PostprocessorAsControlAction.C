#include "PostprocessorAsControlAction.h"
#include "Simulation.h"

registerMooseAction("THMApp", PostprocessorAsControlAction, "add_postprocessor");

template <>
InputParameters
validParams<PostprocessorAsControlAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.addClassDescription(
      "This action adds a control object that copies a postprocessor value into the control "
      "system so that users can work with the postprocessor name directly.");
  return params;
}

PostprocessorAsControlAction::PostprocessorAsControlAction(InputParameters params)
  : MooseObjectAction(params)
{
}

void
PostprocessorAsControlAction::act()
{
  const std::string class_name = "AddControlAction";
  InputParameters params = _action_factory.getValidParams(class_name);
  params.set<std::string>("type") = "CopyPostprocessorValueControl";

  std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
      _action_factory.create(class_name, _name + "_copy_ctrl", params));

  action->getObjectParams().set<PostprocessorName>("postprocessor") = _name;

  _awh.addActionBlock(action);
}
