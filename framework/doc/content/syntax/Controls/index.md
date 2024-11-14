# Controls System

The Controls system in MOOSE has one primary purpose: +to modify input parameters during runtime
of a MOOSE-based simulation+.

If a developer has marked a MOOSE object input parameter as *controllable*, that
parameter may be controlled during a simulation using `Control` objects.
`Control` objects are defined in the input file in the `Controls` block, similar to other systems
in MOOSE. For example, the following input file snippet shows the use of the
[RealFunctionControl](/RealFunctionControl.md) object.

!listing test/tests/controls/real_function_control/real_function_control.i
         block=Kernels Controls
         id=controls_example
         caption=Example of a Control object used in a MOOSE input file.

Here, `func_control` controls the `coef` parameter of the `diff` object. See
[#object-and-parameter-names] for the allowable syntax to specify controlled
parameters.

## Making a Parameter Controllable id=sec:control-param

The input parameters of objects you wish to be controlled must:

- Be marked as "controllable". In the `validParams()` method for the class,
  the `InputParameters::declareControllable(param_names)` method is used as
  shown in [declare_controllable_listing]. Note that `param_names` may be a
  list of parameter names separated by spaces, e.g., `"param1 param2 param3"`.

  !listing framework/src/bcs/DirichletBC.C
          start=InputParameters
          end=DirichletBC::DirichletBC
          id=declare_controllable_listing
          caption=Example `validParams` method that declares a parameter as controllable.
- Be stored as `const` references; for example, in the `.h` file,

  !listing framework/include/bcs/DirichletBC.h line=_value

  which is initialized in the `.C` file using `getParam<T>(param)`, as usual:

  !listing framework/src/bcs/DirichletBC.C line=_value(getParam

!alert tip title=Parameter controllability listed on documentation pages
Each class documentation page lists whether each of its input parameters are controllable.
For example, see the [DirichletBC](source/bcs/DirichletBC.md#input-parameters) page.

## Developing a New Control

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
with various inputs for prescribing the parameter name, but the two listed above are generally
what is needed.  Please refer to the source code for a complete list.

## Object and Parameter Names id=object-and-parameter-names

The objective of a `Control` object is to control parameters of one or more other
objects; these parameters to control are specified by input parameters of the `Control`.
[controls_example] shows an example syntax for specifying input parameters in the
`parameter` parameter. In this example, `*/*/coef` is specified, which would
match any controllable parameter named `coef` at that nesting level. In the example, there
is only one parameter that the pattern matches, so `Kernels/diff/coef` would
be equivalent. The "/"-separated path preceding the parameter name corresponds
to the syntax blocks under which the parameter is located, such as for the system name and object name.

In similar fashion, object names can be requested by controls (e.g., as in the
[`TimePeriod`](/TimePeriod.md)). In this case, the general naming scheme is the same
as above but the parameter name is not included, e.g., `Kernels/diff`.

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
         caption=Example of using the tagged controllable parameters.

!alert note
The tag name does not include the object name although the tag name is added by an object.
To access a controllable parameter, the syntax is `tag/object/name`.
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
