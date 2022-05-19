# MOOSE Action System

MOOSE actions, derived from the base class *Action* are used to set up a problem with other MOOSE objects including mesh generators, kernels, materials, user objects, etc.
An action can be optionally linked to an input syntax, which provides the valid parameters to the action through the MOOSE input parser behind the scene.
An action is assocated with tasks, which determine when the action is to be acted, or the *act* function of an action is to be called by MOOSE.
Association with multiple tasks is allowed. In such case, it is developers' responsibility to switch the functions based on the current task in the act function.
A full list of tasks is available in Moose.C file.
Developers are allowed to register new tasks with MOOSE Action system.
When actions are built either through the input parser or internally by MOOSE, they will exist during the entire simulation.

MOOSE has lots of built-in actions for adding individual objects through input files.
But MOOSE action system is far beyond the basic capabilities provided by these built-in actions.
Developers are encouraged to explore the MOOSE actions system to create their own actions in order to perform problem setup in one place and potentially simplify the input syntax significantly.
One example is to use an action to append a mesh generator that generates a
boundary surrounding a particular subdomain for imposing specific boundary
conditions, based on the information not necessarily inside of a mesh generator
block.

MOOSE actions supports the capability of data file searching as [MooseObject.md] with the same two functions `getDataFileName` and `getDataFileNameByName`.

## Relationship Managers and Actions

If adding any `MooseObjects` in a custom action and those objects have
associated relationship managers, then the
`addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)` must be
overridden. Both
the `ContactAction` in the contact module, and `PorousFlowActionBase` in the
porous flow module provide examples of overriding this method. For the reasons
behind why this must be done in the action system, please see [RelationshipManager.md#rm_action].
