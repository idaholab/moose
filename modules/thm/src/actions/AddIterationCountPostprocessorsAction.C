#include "AddIterationCountPostprocessorsAction.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "ActionWarehouse.h"

registerMooseAction("THMApp", AddIterationCountPostprocessorsAction, "meta_action");

template <>
InputParameters
validParams<AddIterationCountPostprocessorsAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<bool>(
      "count_iterations", false, "Add postprocessors for linear and nonlinear iterations");
  params.addClassDescription("Adds postprocessors for linear and nonlinear iterations");
  return params;
}

AddIterationCountPostprocessorsAction::AddIterationCountPostprocessorsAction(
    InputParameters parameters)
  : Action(parameters), _add_pps(getParam<bool>("count_iterations"))
{
}

void
AddIterationCountPostprocessorsAction::act()
{
  if (_add_pps)
  {
    const std::vector<std::string> it_per_step_class_names = {"NumLinearIterations",
                                                              "NumNonlinearIterations"};
    const std::vector<std::string> it_per_step_names = {"num_linear_iterations_per_step",
                                                        "num_nonlinear_iterations_per_step"};
    const std::vector<std::string> total_it_names = {"num_linear_iterations",
                                                     "num_nonlinear_iterations"};

    for (unsigned int i = 0; i < it_per_step_class_names.size(); i++)
    {
      // iterations per time step
      {
        const std::string class_name = "AddPostprocessorAction";
        InputParameters action_params = _action_factory.getValidParams(class_name);
        action_params.set<std::string>("type") = it_per_step_class_names[i];
        auto action = std::static_pointer_cast<MooseObjectAction>(
            _action_factory.create(class_name, it_per_step_names[i], action_params));
        _awh.addActionBlock(action);
      }
      // cumulative iterations
      {
        const std::string class_name = "AddPostprocessorAction";
        InputParameters action_params = _action_factory.getValidParams(class_name);
        action_params.set<std::string>("type") = "CumulativeValuePostprocessor";
        auto action = std::static_pointer_cast<MooseObjectAction>(
            _action_factory.create(class_name, total_it_names[i], action_params));
        action->getObjectParams().set<PostprocessorName>("postprocessor") = it_per_step_names[i];
        _awh.addActionBlock(action);
      }
    }
  }
}
