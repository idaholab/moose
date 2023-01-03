# Controls System

The control system in MOOSE has one primary purpose: +to modify input parameters during runtime
of a MOOSE-based simulation+.

## Creating a Controllable Parameter id=sec:control-param

The input parameters of objects you which to be controlled must:

- Store parameter as a `const` reference; in your *.h files, declare storage for the parameters as
  follows.

  !listing framework/include/bcs/DirichletBC.h line=_value

- Initialize the reference in the *.C file as follows.

  !listing framework/src/bcs/DirichletBC.C line=_value(getParam

In order to "control" a parameter it must be communicated that the parameter is allowed to be
controlled, this is done in the `validParams` function as in [declare_controllable]. The input can
be a single value or a space separated list of parameters.

!listing framework/src/bcs/DirichletBC.C
         start=InputParameters
         end=DirichletBC::DirichletBC
         id=declare_controllable
         caption=Example `validParams` method that declares a parameter as controllable.

!alert tip
The documentation for a given parameter will indicate whether it is controllable. For example, see
the [DirichletBC](source/bcs/DirichletBC.md#input-parameters) page.

## Create a Control object

`Control` objects are similar to other systems in MOOSE. You create a control in your application
by inheriting from the `Control` C++ class in MOOSE. It is required to override the `execute`
method in your custom object. Within this method the following methods are generally used to get
or set controllable parameters:

- `getControllableValue` <br>
  This method returns the current controllable parameter, in the case that multiple parameters are
  being controlled, only the first value will be returned and a warning will be produced if the
  values are differ (this warning may be disabled).

- `setControllableValue` <br>
  This method allows for a controllable parameter to be changed, in the case that multiple
  parameters are being controlled, all of the values will be set.

These methods operator in a similar fashion as
other systems in MOOSE (e.g., `getPostprocessorValue` in the [Postprocessors] system), each
expects an input parameter name (`std::string`) that is prescribed in the `validParams` method.

There are additional overloaded methods that allow for the setting and getting of controllable values
with various inputs for prescribing the parameter name, but the the two listed above are generally
what is needed.  Please refer to the source code for a complete list.

## Controls Block

`Control` objects are defined in the input file in the Controls block, similar to other systems
in MOOSE. For example, the following input file snippet shows the use of the
[RealFunctionControl](/RealFunctionControl.md) object.

!listing test/tests/controls/real_function_control/real_function_control.i
         block=Controls
         id=controls_example
         caption=Example of a Control object used in a MOOSE input file.

## Object and Parameter Names id=object-and-parameter-names

Notice that in [controls_example] the syntax for specifying a parameter is shown. In general,
the syntax for a parameter name is specified as: `block/object/name`.

- +`block`+: specifies the input file block name (e.g., "[Kernels]", "[BCs]").
- +`object`+: specifies the input file sub-block name (e.g., "diff" in [controls_example2]).
- +`name`+: specifies the parameter name (e.g., "coef" in [controls_example2]).

!listing test/tests/controls/real_function_control/real_function_control.i
         block=Kernels
         id=controls_example2
         caption=Example of a "Kernel" block that contains a parameter that is controlled via a
                 MOOSE Control object.

As shown in [controls_example] an asterisk ("*") can be substituted for any one of these three
"names", doing so allows multiple parameters to match and be controlled simultaneously.

In similar fashion, object names can be requested by controls (e.g., as in the
[`TimePeriod`](/TimePeriod.md)). In this case, the general name scheme is the same
as above but the parameter name is not included.

In both cases there is an alternative form for defining an object and parameter names:
`base::object/name`. In this case "base" is the MOOSE base system that the object is derived from.
For example, `Kernel::diff/coef`. All MOOSE "bases" are listed bellow:

- ArrayAuxKernel,
- ArrayKernel,
- AuxKernel,
- AuxScalarKernel,
- BoundaryCondition,
- Constraint,
- Damper,
- DGKernel,
- DiracKernel,
- Distribution,
- EigenKernel,
- Executioner,
- Executor,
- Function,
- FVBoundaryCondition,
- FVInterfaceKernel,
- FVKernel,
- Indicator,
- InitialCondition,
- InterfaceKernel,
- Kernel,
- LineSearch,
- Marker,
- MaterialBase,
- MeshGenerator,
- MooseMesh,
- MoosePartitioner,
- MoosePreconditioner,
- MooseVariableBase,
- MultiApp,
- NodalKernel,
- Output,
- Postprocessor,
- Predictor,
- Problem,
- RelationshipManager.,
- Reporter,
- Sampler,
- ScalarInitialCondition,
- ScalarKernel,
- Split,
- TimeIntegrator,
- TimeStepper,
- Transfer,
- UserObject,
- VectorAuxKernel,
- VectorInterfaceKernel,
- VectorKernel,
- VectorPostprocessor,

MOOSE allows objects to define a `tag` name to access its controllable parameters with their `control_tags` parameter.

!listing test/tests/controls/tag_based_naming_access/param.i
         block=Postprocessors
         id=controls_tags
         caption=Example of the parameter control_tags.

The two postprocessors in [controls_tags] declare the same control tag `tag`.
Thus their controllable parameter `point` can be set by controls simultaneously with `tag/*/point` as in [controls_tags_use].

!listing test/tests/controls/tag_based_naming_access/param.i
         block=Controls
         id=controls_tags_use
         caption=Example of usinging the tagged controllable parameters.

!alert note
The tag name does not include the object name although the tag name is added by an object.
To access a controllable parameter, the sytax is `tag/object/name`.
Internally, MOOSE adds the input block name as a special tag name.

## Controllable Parameters Added by Actions id=controllable_params_added_by_actions

MOOSE also allows parameters in [Actions](Action.md) to be controllable.
The procedure for making a parameter in an [Action](Action.md) controllable is the same as documented in [syntax/Controls/index.md#sec:control-param].
It is important that this controllable parameter must be directly connected with the parameters of MOOSE objects, such as kernels, materials, etc., using this parameter.

!listing test/src/actions/AddLotsOfDiffusion.C
         start=GenericConstantArray
         end=connectControllableParams
         id=connect_controllable
         caption=Example of connecting controllable parameters in an action and the objects added by the action.

The action controllable parameter can be referred as usual in an input file. For example,

!listing test/tests/controls/action_control/action_control_test.i
         block=Controls
         id=controls_example3
         caption=Example of a "Action" block that contains a parameter that is controlled via a
                 MOOSE Control object.

## Child Objects

!syntax list /Controls objects=True actions=False subsystems=False

## Associated Actions

!syntax list /Controls objects=False actions=True subsystems=False
