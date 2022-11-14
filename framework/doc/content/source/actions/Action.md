# MOOSE Action System

MOOSE *Actions* are used to execute *tasks*. Each application registers
numerous *actions*, *tasks*, and *syntax*. Each task is associated with one or more
actions, and each action may perform one or more tasks. Syntax is used by the
input file parser to generate actions/tasks.

Common uses for actions are to perform setup tasks and create MOOSE objects.

## Creating Actions

To create a new action, first derive from the appropriate base class: if the new action
is to correspond to creating MOOSE objects from an input file, then derive
from `MooseObjectAction`; else, derive from `Action`.

The `act()` method must be implemented to perform the associated task(s). If the
action will be registered to multiple tasks, then the variable `_current_task`
can be queried to determine the current task, for example,

``` language=cpp
void
ExampleAction::act()
{
  if (_current_task == "example_task_a")
  {
    // "example_task_a" execution
  }
  else if (_current_task == "example_task_b")
  {
    // "example_task_b" execution
  }
}
```

`MooseObjectAction`s, have the member variables `_type` and `_moose_object_pars`,
which correspond to the type and [InputParameters.md] of the MOOSE object to
be created, respectively. For example, the action to create a BC object has
the following `act()` method:

!listing framework/src/actions/AddBCAction.C re=void\sAddBCAction::act.*?^}

The action should be registered to one or more tasks using the `registerMooseAction`
macro (conventionally in the action source file), for example,

``` language=cpp
registerMooseAction("ExampleApp", ExampleAction, "example_task_a");
registerMooseAction("ExampleApp", ExampleAction, "example_task_b");
```

## Registering Tasks

Like MOOSE objects, tasks and syntax are registered in an application's constructor,
conventionally from a static method called `registerAll`. MOOSE's tasks, actions,
and syntax are defined in [Moose.C](framework/src/base/Moose.C), for example.

Several macros are relevant for registration of tasks and syntax.

Tasks must be registered using the `registerTask` macro:

``` language=cpp
registerTask("task_name", is_required)
```

where `task_name` is the name of the new task, and `is_required` should be set
to `true` if the task is required by the application. A required task always
has all of its associated actions executed, even if no syntax triggers it.

Tasks may have dependencies between them. The macro `addTaskDependency` is
used to declare that a task depends on another, for example,

``` language=cpp
addTaskDependency("secondary_task", "primary_task")
```

Here a task called "secondary_task" will be executed sometime after the task
called "primary_task".

## Registering Syntax

There are two macros associated with registering syntax to an action/task:
`registerSyntax` and `registerSyntaxTask`:

``` language=cpp
registerSyntax(action, syntax);
registerSyntaxTask(action, syntax, task);
```

The difference between these is only apparent when the action has more than one
task registered to it; in this case, the additional argument in
`registerSyntaxTask` specifies which task of the specified action to execute.

For example, the [AddKernelAction.md] is registered to tasks for adding kernels
and aux kernels:

``` language=cpp
registerSyntaxTask("AddKernelAction", "Kernels/*", "add_kernel");
registerSyntaxTask("AddKernelAction", "AuxKernels/*", "add_aux_kernel");
```

The syntax need not be associated only to sub-blocks in the input file. For example,
the existence of a `Mesh` block triggers [SetupMeshCompleteAction.md]:

``` language=cpp
registerSyntax("SetupMeshCompleteAction", "Mesh");
```

## Relationship Managers and Actions

If adding any `MooseObjects` in a custom action and those objects have
associated relationship managers, then the
`addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)` must be
overridden. Both
the `ContactAction` in the contact module, and `PorousFlowActionBase` in the
porous flow module provide examples of overriding this method. For the reasons
behind why this must be done in the action system, please see [RelationshipManager.md#rm_action].

## Troubleshooting Actions

To see the list of actions as they execute, use `Debug/show_actions=true`:

```
[Debug]
  show_actions = true
[]
```
