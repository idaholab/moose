# Physics system

The `Physics` system is meant to standardize the process of adding an equation and its discretization
to a simulation. The [Action system](syntax/Actions/index.md) successfully enabled the definition of equations
in a compact and user-friendly way, but it lead to poor code re-use across physics.

The `Physics` system currently still ties equations and discretizations. The equations are usually described in
a base class using the parameters, and the discretization is specified in the derived classes.

## Implementing your own Physics

If you have *not* created the kernels, boundary conditions, and so on, the `Physics` system is not a good place
to start. You must start with a working implementation of your equations before attempting to create a `Physics` object.

If you do have a working set of kernels, boundary, and other MOOSE objects, that let you solve an equation in MOOSE, you
must consider the following before implementing a `Physics`:

- is user-friendliness a priority for the expansion of my work?
- is the current workflow unsatisfactory in that regard?
- would creating objects programmatically reduce the potential for user-error while allowing sufficient flexibility?

If the answer is yes to all three, then you may start implementing a `Physics` object for your equation.
The simple concepts behind the simulation setup in a `Physics` is that the `add<various MOOSE object>` routines
are all called on the `Physics` and they are all called at the time in the setup as with a regular input file.

So for example, to make a `AAAPhysics` create a finite element diffusion kernel, one can override `addFEKernels` like this:

```
void
AAAPhysics::addFEKernels()
{
  {
    const std::string kernel_type = "ADDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;  // we saved the name of the variable in this class attribute
    getProblem().addKernel(kernel_type, name() + "_diffusion", params);
  }
}
```

Notice how we use the `PhysicsBase::getFactory()` routine to get access to the `Factory` that will get the parameters we
need to fill, and the `PhysicsBase::getProblem()` to get access to the `Problem` which stores the objects created.

If you already an `Action` defined for your `Physics`, implementing a `Physics` mirroring it should be fairly
straightforward. The principal advantages of doing so are:

- benefit from new APIs implemented in the `Physics` system
- being able to leverage the `Components` system to define a complex system
- being able to hard-code the interaction with known `Physics`, leveraging the [Functor](syntax/Functors/index.md)
  to form boundary conditions for example.

You may consider using the `InputParameters::transferParam` routine to easily transfer all the parameters from
the still-existing `Action` to the new `Physics` object.


### Interaction with Components

TODO

### Advice on implementation

- Use as much parameter checking as you. [PhysicsBase.md] defined utilities such as the ones below
  that let you check that the user inputs to your physics are correct.

```
  void checkParamsBothSetOrNotSet(std::string param1, std::string param2) const;
  template <typename T, typename S>
  void checkVectorParamsSameLength(std::string param1, std::string param2) const;
  template <typename T>
  void checkVectorParamsNoOverlap(std::vector<std::string> param_vec) const;
```

The Physics class you will create will hold the parameters that are shared between all the
discretized versions of it.

Physics and spatial discretizations are as separated as we could, but they are still very much intertwined. So
when you are adding a parameter you need to think about:

- is this more tied to the Physics? If so then it likely belongs in a `PhysicsBase` base class
- is this more tied to the discretization of the equation? If so then it likely belong in the derived, user-instantiated,
  `XYZPhysics` class.
- do you need to keep track of it to retrieve it from another Physics? Or is it very local to this one physics you are
  writing? If it needs to be accessible to unrelated Physics, consider storing it as an attribute and writing a public getter.
