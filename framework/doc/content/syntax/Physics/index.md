# Physics system

The `Physics` system is meant to standardize the process of adding an equation and its discretization
to a simulation. It is based on the [Action system](source/actions/Action.md), with additional APIs
defined to support the definition of an equation.

## Interaction with Components

The interaction with Components is one of the main goals of the Physics system. Stay tuned for future developments.

## Generating a traditional input from a Physics input

By substituting the traditional [Problem](syntax/Problem/index.md) in your simulation for the [DumpObjectsProblem.md],
you can generate the equivalent input using the traditional Kernel/BCs/etc syntax to an input using `Physics`.
This is useful for debugging purposes.

!alert note
This is not currently possible for thermal hydraulics inputs which use a specific problem.

## Implementing your own Physics

If you have *not* created the kernels, boundary conditions, and so on, the `Physics` system is not a good place
to start. You must start with a working implementation of your equations before attempting to create a `Physics` object.

If you do have a working set of kernels, boundary conditions, and other MOOSE objects, that let you solve an equation in MOOSE,
you should consider the following before implementing a `Physics`:

- is user-friendliness a priority for the expansion of my work?
- is the current workflow unsatisfactory in that regard?
- would creating objects programmatically reduce the potential for user-error while allowing sufficient flexibility?

If the answer is yes to all three, then you may start implementing a `Physics` object for your equation.
The simple concepts behind the simulation setup in a `Physics` is that the `add<various MOOSE object>` routines
are all called on the `Physics` and they are all called at the same time in the setup as with a regular input file.

So for example, to make a `DiffusionPhysics` create a finite element diffusion kernel, one can override `addFEKernels` like this:

```
void
DiffusionPhysics::addFEKernels()
{
  {
    const std::string kernel_type = "ADDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;  // we saved the name of the variable as a class attribute
    getProblem().addKernel(kernel_type, name() + "_diffusion", params);
  }
}
```

Notice how we use the `PhysicsBase::getFactory()` routine to get access to the `Factory` that will get the parameters we
need to fill, and the `PhysicsBase::getProblem()` to get access to the `Problem` which stores the objects created.
We want the `Physics` to be able to be created with various types of `Problem` classes.

If you already have an `Action` defined for your equations, converting it to a `Physics` should be fairly straightforward. The principal advantages of doing so are:

- benefit from new APIs implemented in the `Physics` system
- a standardized definition of the equation, which will help others maintain your `Action`
- future ability to leverage the `Components` system to define a complex system

### Advice on implementation

#### Add a lot of checks

Please add as much parameter checking as you can. The `PhysicsBase` class inherits the `InputParameterCheckUtils` that implements
routines like the ones below that let you check that the user inputs to your physics are correct.

!listing InputParametersChecksUtils.h start=InputParametersChecksUtils<C>::checkVectorParamsSameLength end=} include-end=true

Using this utility, consider checking that:

- the size of vector, vector of vectors, `MultiMooseEnum` and map parameters are consistent
- if a parameter is passed it must be used, for example if one parameter conditions the use of other parameters
- the block restrictions are consistent between the `Physics` and objects it defines

#### Separate the definition of the equation from its discretization

You may consider creating a `PhysicsBase` class to hold the parameters that are shared between all the
implementations of the equations with each discretization. This will greatly facilitate switching between discretizations
for users. It will also maximize code re-use in the definition and retrieval of parameters, and in the attributes of the
various discretized `Physics` classes.

Physics and spatial discretizations are as separated as we could make them, but they are still very much intertwined. So
when you are adding a parameter you need to think about:

- is this more tied to the strong form of the equation? If so then it likely belongs in a `XYZPhysicsBase` base class
- is this more tied to the discretization of the equation? If so then it likely belong in the derived, user-instantiated,
  `XYZPhysics(CG/DG/HDG/FV/LinearFV)` class.

#### Rules for implementation of Physics with regards to restarting variables or using initial conditions

It is often convenient to define initial conditions in the `Physics`, and also to be able to
restart the variables defined by the `Physics` automatically with minimal user effort. User-defined initial conditions
are convenient to keep the input syntax compact, and default initial conditions are useful to avoid
non-physical initial states. However, all these objectives conflict when the user defines parameters for initialization in
a restarted simulation. To make things simple, developers of `Physics` should follow these rules, which we developed based on user
feedback.

- if the `initialize_variables_from_mesh_file` parameter is set to true, then:
  - skip adding initial conditions
  - error if an initial condition parameter is passed by the user to the `Physics`
- if the `Physics` is set to use (define kernels for) variables that are defined outside the `Physics`, then:
  - skip adding initial conditions
  - error if an initial condition parameter is passed by the user to the `Physics`
- else, if the user specifies initial conditions for variables in the `Physics`
  - always obey these parameters and add the initial conditions, even if the simulation is restarting
  - as a sanity check, the [FEProblemBase.md] will error during restarts, unless [!param](/Problem/FEProblem/allow_initial_conditions_with_restart) is set to true
- else, if the user does not specify initial conditions in the `Physics`, but the `Physics` does define default values for the initial conditions
  - if the simulation is restarting (from [Checkpoint.md] notably), skip adding the default initial conditions
  - (redundant due to the first rule) if the `initialize_variables_from_mesh_file` parameter is set to true, skip adding the default initial conditions
  - (redundant due to the second rule) if the `Physics` is set to use (define kernels for) variables that are defined outside the `Physics`, skip adding the default initial conditions

!alert note
For `initialize_variables_from_mesh_file` to work correctly, you must use the `saveNonlinearVariable()` and `saveAuxiliaryVariable()` `Physics` routines
in the constructor of your `Physics` on any variable that you desire to be restarted.
