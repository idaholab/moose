[TOC]

## Supported Finite Element Types [(back to top)](#back_to_top)
The following is a list of finite element families and
their valid orders within the MOOSE framework.

Any families with a * next to their name haven't been used yet by
anyone using MOOSE (as far as the authors know). "ANY" means that
you can use any order from FIRST through FORTYTHIRD.

* `BERNSTEIN`*: ANY
* `LAGRANGE`: FIRST, SECOND, THIRD (THIRD only in 1D)
* `L2_LAGRANGE`*: FIRST, SECOND, THIRD (THIRD only in 1D)
* `CLOUGH`: SECOND, THIRD
* `HERMITE`: THIRD
* `HIERARCHIC`: ANY
* `L2_HIERARCHIC`*: ANY
* `MONOMIAL`: ANY
* `SCALAR`: ANY
* `SZABAB`*: FIRST through SEVENTH
* `XYZ`: ANY


## String-Derived Types [(back to top)](#back_to_top)
MOOSE has a number of string-derived types that are provided to
improve type safety when writing Custom Actions, and for use within
the Peacock GUI.  When writing Custom Actions, you will encounter
compile-time (rather than run-time) errors about missing Parameters if
you fail to provide parameters of the correct types.

Some of the more common types are:

* `NonlinearVariableName`: This is the type that represents one of the
nonlinear variable names in your system.  Usually these are defined in
the `[Variables]` block.  An example of using this type with a params
object is given below:
```C++
params.set<NonlinearVariableName>("variable") = "density";
```

* `AuxVariableName`: This is the type that represents one of the
auxiliary variable names in your system. Usually these are defined in
the `[AuxVariables]` block.

* `VariableName`: This is a type for a generic variable name. This
applies mainly to `ElementPostprocessors`, `SidesetPostprocessors`,
and `NodalPostprocessors` when a variable name is expected.


## MooseEnum [(back to top)](#back_to_top)
`MooseEnum` is a class designed to be a "smart" replacement for the
C++ `enum` type. `MooseEnums` are intended for use anywhere in a input
file where your parameter has some fixed number of options. By using a
`MooseEnum` in your MOOSE object, your options will be propertly read
from the input file and checked against a master list of valid
options. In addition, pick-lists will be automatically displayed when
using Peacock. Note: MooseEnum is case-preserving, but not
case-sensitive.

An example of declaring and immediately using a `MooseEnum` in a
`validParams()` function is given below:
```C++
#include "MooseEnum.h"

template<>
InputParameters validParams<MyObject>()
{
  // Create a list of options (comma separated list of options
  // in a single string), and optionally supply a default option
  // as the second argument in the constructor
  MooseEnum myOptions("Option1, Option2, Option3, ...OptionN", "Option1");

  InputParameters params = validParams<ParentObject>();
  params.addParam<MooseEnum>("myProperty",  myOptions, "A property used in my calculation");

  return params;
}
```

You can extract a MooseEnum parameter like any other type, and it can
be reassigned.  The assignment will throw an error if the assigned
string is not valid for the MooseEnum (as defined in its constructor).
```C++
MooseEnum foo = getParam<MooseEnum>("myProperty");
foo = "Option2";
```

The MooseEnum can be compared to string literals:
```C++
if (foo == "Option1")
{
  // Do something with "Option1"
}
```

Or used in a switch statement (by default, the enumeration numbering starts at 1):
```C++
MooseEnum test("first, second, third", "first");

switch (test)
{
case 1:
  // do something with "first"
  break;

case 2:
  // do something with "second"
  break;

case 3:
  // do something with "third"
  break;

default:
  mooseError("Unhandled enumeration!");
}
```

## Moose Setup [(back to top)](#back_to_top)
In this section, commands which are to be typed into the shell are
preceded by a dollar sign `$`, to represent the shell prompt.

### Building MOOSE

First, make sure you have the latest version of the repository. MOOSE is constantly being updated. Update your local repository:
```bash
$ cd ~/projects/trunk
$ svn up
```

If you are receiving errors during the building of MOOSE or your
application, there is a possibility that libMesh is not compiled
correctly.  To verify libMesh is not the issue, first make sure you
are up to date, and then try rebuilding it:
```bash
$ cd ~/projects/trunk
$ svn up
$ cd ~/projects/trunk/libmesh
$ ./build_libmesh_moose.sh
```

During the configure stage, verify you can find status messages
similar to the following somewhere in the output:
```bash
---------------------------------------------
----- Configuring for optional packages -----
---------------------------------------------
checking for /opt/packages/petsc/petsc-3.1-p8/gnu-opt/include/petsc.h... yes
<<< Configuring library with MPI from PETSC config >>>
<<< Configuring library with PETSc version 3.1.0 support >>>
<<< Configuring library with Hypre support >>>
```

If you don't see this text, or there are errors about not finding
PETSc, then please scroll down to the **Environment** section
below, and verify that your environment is correct.
Building libMesh may take a few minutes. Once it is complete, navigate to
your application and run make there:
```bash
$ cd ~/projects/trunk/<your_application>
$ make -j4
```

If there is an error during the building of your application, running
the `run_tests` script in each directory your application depends on
is a good way to isolate issues.  For MOOSE, for example, there is a
separate location where tests are run from:
```bash
$ cd ~/projects/moose_test
$ make -j4
$ ./run_tests -j4
```


### Environment

If something in your environment is not set correctly, check that the
following environment variables are set.

* Verify that PETSc is indeed installed in $PETSC_DIR:
```bash
$ echo $PETSC_DIR
/opt/packages/petsc/petsc-3.1-p8/gnu-opt
```

* Verify that MPICH is installed:
```bash
$ echo $MPI_HOME
/opt/packages/mpich2/mpich2-1.4.1p1/gnu-opt
```

* Verify that we will actually be using said MPI installation:
```bash
$ which mpicc
/opt/packages/mpich2/mpich2-1.4.1p1/gnu-opt/bin/mpicc
```

* Verify libMesh will be using mpicc during the build:
```bash
$ echo $CC
mpicc
```

* Verify that gfortran is in your PATH:
```bash
$ which gfortran
/usr/bin/gfortran
```

If each of these tests produces reasonable output, but libMesh still fails
to configure/build, then you should contact Jason Miller for further
assistance.

If you're in a hurry, a simple last-step cop-out is to try
reinstalling the MOOSE package. The MOOSE package may be re-installed
safely multiple times, and this procedure may solve a possible
environment issue.

If you are *not* using the MOOSE redistributable package, you are
responsible for building the following pieces of software yourself:
* C, C++, and Fortran compilers.  We recommend Clang with GNU Fortran or the GNU Compiler Collection.
* An MPI library.  We recommend either [mpich](http://www.mpich.org) or [openmpi](http://www.open-mpi.org).
* [HYPRE](http://computation.llnl.gov/casc/hypre/software.html)
* [PETSc](http://www.mcs.anl.gov/petsc)
* [libMesh](https://github.com/libMesh/libmesh)


## Coupling To an Arbitrary Number of Variables [(back to top)](#back_to_top)
The trick here is to do "vector coupling".  What this means is
coupling to an arbitrary number of variables simultaneously.

How do you do such a thing?  The input file syntax is as follows:
```puppet
[AuxKernels]
  [./]
    type = SummingAux
    variable = the_sum
    coupled_vars = 'var1 some_other_var var25'
  [../]
[]
```

That will couple `var1`, `some_other_var` and `var25` into `SummingAux` as
`coupled_vars`.  Note: `coupled_vars` is the name of the coupling
parameter that you added using `params.addCoupledVar()` like usual.

Now... how do you get out the value of each one?  This requires some
slightly more advanced C++ code involving pointers.  In your class you
do this:

In your header file:
```C++
class SummingAux : public AuxKernel
{
private:
  std::vector<const VariableValue *> _vals;
  std::vector<const VariableGradient *> _grad_vals;
...
};
```

In your source file:
```C++
SummingAux::SummingAux(const InputParameters & parameters)
  : AuxKernel(parameters)
{
  int n = coupledComponents("coupled_vars");

  _vals.resize(n);
  _grad_vals.resize(n);

  for (unsigned int i=0; i<_vals.size(); ++i)
  {
    _vals[i] = &coupledValue("coupled_vars", i);
    _grad_vals[i] = &coupledGradient("coupled_vars", i);
  }
}
```

What this is doing is filling up vectors of `VariableValue` and
`VariableGradient` pointers.  You can then loop over them as follows:
```C++
for (unsigned int i=0; i<_vals.size(); ++i)
  the_sum += (*_vals[i])[_qp]
```
Or whatever your particular application requires.  The trick is that
you have to dereference the pointers in the vectors (that's the
`(*_vals[i])` part) and then index by `_qp` like you normally do for a
coupled variable.

## AuxKernel Restrictions [(back to top)](#back_to_top)
* Nodal `AuxKernels` can't use `MaterialProperties`.
* This is *not* an arbitrary restriction.  If a Node falls on a block
  boundary... which Material object would you use for the material
  property value at that point?


## BoundaryConditions [(back to top)](#back_to_top)

### What are the names of the boundaries for 3D Generated Mesh?

* X_min face: left
* X_max face: right
* Y_min face: bottom
* Y_max face: top
* Z_min face: back
* Z_max face: front




### Is there a difference between sideset "n" and nodeset "n"?

It is the "boundary" n... we don't distinguish between sidesets and
nodesets as far as numbering goes... that has the side effect that
sidesets and nodesets MUST have unique numbers... i.e. you _cannot_
have a sideset with ID 1 and a nodeset with ID 1.  (Technically you
can, but it probably won't mean what you think it will
mean... "boundary 1" will be the _union_ of the sideset 1 and
nodeset 1.)

Come up with a scheme and stick to it.  I usually recommend numbering
sidesets from 1 (like 1, 2, 3, 4, 5, etc.) and nodesets from 100 (like
100, 101, 102, etc.) so that you won't have any collisions.  We use a
similar scheme to this in BISON (although I think nodesets start at
1,000 or 10,000 to give them more room).

Further, I _highly_ recommend using boundary "naming".  In that case
you can say "left\_nodes" or "right\_side".  To keep things straight.
If you have a limited number of boundaries this is a good idea.  You
can either name them in Cubit when you generate the mesh, use the
automatic names from `GeneratedMesh` or assign names to numbers in the
Mesh block (look at the input file syntax dump or the MOOSE manual to
see how to do that).



### Can we specify DirichletBC for a nodeset which might not be on the boundary?

Yes.  Just give it a nodeset ID (like 105 or something) and then just
say "boundary = 105" in your DirichletBC block in your input file.
You can do this for "internal" NeumanBCs as well but it's trickier.
In Cubit you have to assign one side of the boundary that the sideset
is "with respect to"... which you can do right in the Sideset section
of Cubit.


### What's the difference between PresetBC and DirichletBC?

`PresetBC` is a special type of `DirichletBC`.  It is useful in the
case where you _know_ what the value of the variable should be at the
beginning of the timestep.  It _forces_ the value of the variable at
that boundary to be the value it is supposed to be before the solve
even starts.  In other words, it sets the initial guess for the value of
the variable on that boundary to be exactly the right value.

DirichletBC on the other hand just uses the value from the last
timestep (or the initial condition for the first timestep) as the
initial guess.  This might not be a good guess (especially for the
first timestep)... but on the other hand you might not know at all
what the value is supposed to be (like in the case of nonlinear
coupled DirichletBCs).

So think of it like this:

`PresetBC`: Set the value of the variable to what it's supposed to be and hold it there.
`DirichletBC`: Use a guess... but really solve for the value of the variable

A couple more notes on this:

1. `DirichletBC` is older... so it is just used more often (even in
   cases where a PresetBC could be used instead).

2. If your problem is boundary condition driven (like a prescribed
   displacement) you will really want to use PresetBC because that will
   set the "forces" (i.e. residuals) in your boundary elements correctly
   at the beginning of each solve (instead of having to "solve" for the
   movement of the boundary nodes like with `DirichletBC`).


## Functions [(back to top)](#back_to_top)

### What functions are available for the MooseParsedFunction class?

The function parsing capability in MOOSE comes from an external
library called FunctionParser that is distributed with libMesh.  For
more information, see the FunctionParser [website](http://warp.povusers.org/FunctionParser).



## UserObjects [(back to top)](#back_to_top)

This is still a new system - and we haven't done documentation for it
yet because it's been in quite a bit of flux.  I still expect at least
one significant API change for this system (having to do with
restart)... BUT it has stabilized enough that I think it's useful.

For now, the best documentation is in the tests and in the `UserObjects`
that are in MOOSE.  But here's a quick synopsis:

You could think of a `UserObject` as a generalization of Postprocessors.
Indeed... if you look at Doxygen you will see that Postprocessors
_are_ `UserObjects` and `UserObjects` can be used
as Postprocessors!

What's the difference then?

Postprocessors compute *one* scalar value. `UserObjects`, on the
other hand, can compute whatever they want and provide whatever kind
of interface they want to provide so other MOOSE objects can get
access to what they computed.

Just like Postprocessors, there are 4 "kinds" of `UserObjects`: Nodal, Elemental,
Side and General (which correspond to the 4 base classes for
`UserObjects`).  When you create a `UserObject` you *must* override some
functions:

```C++
virtual void initialize() = 0
virtual void execute() = 0
virtual void threadJoin(const UserObject &uo) = 0
virtual void finalize() = 0
virtual void destroy() = 0
```

Here's a short synopsis of each:

* `initialize()`: Called before looping begins (i.e., before looping over nodes for a NodalUserObject).  Usually where you want to initialize some values.
* `execute()`: Called _on_ each geometric object (i.e., each Node for a NodalUserObject).  This is where you want to do your main computation.
* `threadJoin()`: Called during threaded loop execution.  You must take the data from "uo" and "merge" it into the data for "this" object.
* `finalize()`: Called after the geometric loop.  Usually do some final computation here.
* `destroy()`: Called when you need to release any dynamically created memory (i.e., when the object is being destroyed).  Basically, a destructor.

So a `UserObject` will do some computation on a bunch of geometric
objects (like on all Elements, or all Sides in a Sideset) and store
the results of that computation _internally_.  At that point, it is up
to you to provide an interface (functions on your `UserObject`) to
expose that data.

When another MOOSE object wants to use a `UserObject` you do that by
calling `getUserObject<UserObjectType>("name")` in your initialization
list... similar to getting a Function object.  Note that you have to
provide the UserObjectType and the _reference_ that comes back to you
from that call is of type UserObjectType.  What this means is that you
can call functions on that object that were defined in that object.

Let's look at an example.  Let's say that I need the average
temperature on the current Block in the mesh to compute a material
property.  To implement this without a `UserObject`, you would have
had to create an ElementAverageValue Postprocessor for _every_ Block
in your mesh, then create a Material that would get ALL of those
Postprocessor values and handle them differently depending on what
subdomain the Material was currently being evaluated on.  It would
have worked... but it would have been very messy.

You can now solve this problem by creating a new `UserObject` that
inherits from ElementAverageValue, and call it, say,
BlockAverageValue.  What BlockAverageValue is going to do is
accumulate separate integrals of the value and volume in _each_ block
and store them in the std::maps

```C++
std::map<unsigned int, Real> _block_to_integral;
std::map<unsigned int, Real> _block_to_volume;
```

Then, inside BlockAverageValue you would create a function like this:

```C++
Real blockAverage(unsigned int subdomain_id);
```

Then, in your Material, you would get this object by name by taking
the name in from the input file (just like you get a Function):
```C++
MyMaterial::MyMaterial(name, params) :
  ...
  _block_average_value(getUserObject<BlockAverageValue>("block_average_value_user_object")),
  ...
{}
```

And then in `computeQpProperties()` you would be able to do this:
```C++
_my_prop[_qp] = 5.9 * _block_average_value.blockAverage(_current_elem->subdomain_id());
```

To make this work in the input file you would have:
```puppet
[UserObjects]
  [./abav]
    type = BlockAverageValue
    variable = u
    other_param = stuff
  [../]
[]

[Materials]
  [./my_mat]
    type = MyMaterial
    ...
    block_average_value_user_object = abav
  [../]
[]
```

Basically, a `UserObject` can do whatever it wants and provide whatever
data it wants.  It's a completely open-ended system with unlimited
possibilities.  But, with great power comes great responsibility!  *do
not overuse this system*.  If your computation fits into one of the
other MOOSE systems, do it there instead.

Also, don't get sloppy with the `UserObject` system!  You can actually
create your own object-oriented hierarchy with a base class that
inherits from ElementUserObject and then have multiple implementations
of that base class and your other MOOSE objects retrieve your
`UserObjects` by calling:
```C++
getUserObject<MyBaseClass>()
```
Then you can use the interface defined in MyBaseClass.  This will give you
the ability to swap in and out your _own_ `UserObjects`... providing
tons of flexibility.

For more explanation look at `LayeredIntegral` in MOOSE and the
corresponding layered\_integral\_test in moose_test.


## Using HYPRE [(back to top)](#back_to_top)

Hypre is an Algebraic Multigrid Preconditioner suitable for preconditioning a wide range of MOOSE-based simulations.

These basic options work for several 2D problems:
```puppet
  petsc_options = '-snes_mf_operator -ksp_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
```

When running in 3D, you need to set `-pc_hypre_boomeramg_strong_threshold`
to 0.7 manually for best performance:
```puppet
  petsc_options = '-snes_mf_operator -ksp_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre boomeramg 0.7'
```


## Restart & Recover [(back to top)](#back_to_top)

It is now possible to "continue" MOOSE solves.  "Continue"
is in quotes because it means different things to different people.
Within MOOSE, we distinguish between two different types of
"continuation": Recover and Restart.

### Recover

Recovering a simulation may be required because a simulation ended
prematurely (for instance a node went down on your cluster -- or you
hit Ctrl-C by accident -- or maybe you had num\_steps set too small)
and you just want to continue the solve exactly as it was prior to its
premature end.

To do this, you just need to have checkpoint files turned on (more
about that in a moment) and run the same command line you did to start
the original solve, but append the `--recover` command line argument:

```bash
./bison-opt -i awesome_fuel.i --recover
```

You can make some small modifications to the input file (like
increasing num\_steps or switching a preconditioner) but for the most
part it's going to continue doing what it was doing.  You can also
optionally pass a checkpoint file name base to start recovering from a
few steps back (more on that below).

The `--recover` option will *append* to the same Exodus file you were
writing to in the original simulation.  The idea is that you should be
able to obtain the same Exodus file whether you run the entire simulation
all the way through, or you stop it halfway through and run it again
with `--recover`.

The `run_tests` script itself also accepts the `--recover` option.  Executing
```bash
./run_tests --recover
```
will run each test halfway, re-run it with the `--recover` command
line option, and finally compare the result to an existing "gold" file
obtained from running the simulation all the way through from start to
finish.  You can use this feature to test whether your application has
any issues with recovery, for example, state data not being saved and
loaded properly. It is likely there will be issues...  more on that
later.


### Restart

A "Restart" in MOOSE occurs when you want to start a new calculation
from the end of a previous one.  You accomplish this via the
"restart\_file\_base" parameter in the Executioner block.  With a
Restart, it is possible to solve new equations and use new Kernels,
Materials, etc.

### To use either Recover or Restart:

1.  You will need "Checkpoint" files.  Checkpoint files are "state"
    files that get written into a directory alongside your normal
    output file.  Let's say you have specified `Output/file_base=out`.
    The Checkpoint files will then be written int a directory called
    "out\_cp".  There will be LOTS of different types of state files
    in that directory: mesh (xdr), EquationSystems (xdr),
    RestartableData (rd) and Material (msmp)... and they will be
    numbered according to the time step they are holding the state
    for.

    To turn on writing of Checkpoint files, you use
    `Output/num_checkpoint_files` in your input file.  The number that
    you set that to will determine the number of timesteps worth of
    Checkpoint files to keep around.  Thus, if you set it to "1" only
    the last timestep will have a Checkpoint file.  If you set it to
    two, you will have Checkpoint files for the last 2 timesteps, etc.

1.  When using `--recover` or restart\_file\_base you can pass in a Checkpoint file "base" to use as follows:
    ```bash
    ./bison-opt -i awesome_fuel.i --recover out_cp/0023
    ```

    or (in your input file for restart)

    ```puppet
    [Executioner]
      restart_file_base = out_cp/0023
    []
    ```

    That will Recover or Restart (respectively) the simulation using Checkpoint file 0023.

1.  Finally, to make all of this work you need to be saving and loading any "state" data in your application.

    In any MOOSE-based object (Kernels, etc.) you now have access to a couple of new functions:

    ```C++
      T & declareRestartableData(std::string data_name);
      T & declareRestartableData(std::string data_name, const T & init_value);
    ```

    Notice that these return *C++ references*!

    Thus, if you had a member variable in your Postprocessor that was holding some state:

    ```C++
    class Stuff
    {
      Real _my_state;
    };
    ```

    To make sure that `_my_state` gets stored and loaded properly when
    doing a Recover or Restart, you must first turn it into a
    *reference*:

    ```C++
    class Stuff
    {
      Real & _my_state;
    };
    ```

    And then, in the initialization list of your constructor for that class, initialize your reference:

    ```C++
    Stuff::Stuff() :
      _my_state(declareRestartableData<Real>("my_state", 3.7)
    {}
    ```

    This tells MOOSE about your state data and returns a *reference* that
    you then hold onto in `_my_state` and use in a normal way (nothing
    else about your code needs to change).  The "3.7" there is just
    setting the initial value of the data, this part is optional.

    In this way, you can tell MOOSE about all of the state data you
    might have in any of your objects and then MOOSE will handle the
    storing and loading of that data automatically.  Note that not all
    class data is automatically required to be "state" data.  In
    particular, any class data which can be recomputed from another
    piece of class data, the nonlinear solution vector, etc. does not
    need to be treated as "state".

    The Restart system can also deal with nested data types like
    `std::vector<std::map<std::string, Real> >`.  However, there are
    also ways to create special data writers and readers for unique
    objects.  Custom data writers and readers are beyond the scope of
    this article, however, since they will typically not be required.


## Searchable Dump [(back to top)](#back_to_top)

You can search through your application's registered syntax using the
`--dump` command line option.  If you provide an extra search string
after `--dump`, only parameters, blocks, and values matching the
search string are returned.

Example 1 - Look for all parameters in the block named ElementL2Error.
```bash
./appname-opt --dump ElementL2Error
```

```puppet
[Postprocessors]
  [./ElementL2Error]
    block      = ANY_BLOCK_ID   # block ID or name where the postprocessor works
    execute_on = residual       # Set to (residual|timestep|timestep_begin) to execute only at that moment
    function   = (required)     # The analytic solution to compare against
    output     = both           # The values are: none, screen, file, both (no output, output to screen ...
                                # only, output to files only, output both to screen and files)
    type       = ElementL2Error
    variable   = (required)     # The name of the variable that this postprocessor operates on
  [../]
[]
```


Example 2 - Look for all places in the input file that have a
parameter that begins with "output_" Note the use of the asterisk as a
wildcard character.
```bash
./appname-opt --dump output_*
```

```puppet
[BCs]
  [./PlenumPressure]
    output_initial_moles   =      # The reporting postprocessor to use for the initial moles of gas.

    [./*]
      output_initial_moles =      # The reporting postprocessor to use for the initial moles of gas.
    [../]
  [../]
[]

[Materials]
  [./CreepUO2]
    output_iteration_info  = 0    # Set true to output sub-newton iteration information
  [../]

  [./PLC_LSH]
    output_iteration_info  = 0    # Set true to output sub-newton iteration information
  [../]

  [./PowerLawCreep]
    output_iteration_info  = 0    # Set true to output sub-newton iteration information
  [../]

  [./ThermalIrradiationCreepPlasZr4]
    output_iteration_info  = 0    # Set true to output sub-newton iteration information
  [../]

  [./ThermalIrradiationCreepZr4]
    output_iteration_info  = 0    # Set true to output sub-newton iteration information
  [../]
[]

[Output]
  output_displaced         = 0    # Requests that displaced mesh files are written at each solve
  output_initial           = 0    # Requests that the initial condition is output to the solution file
  output_solution_history  = 0    # Requests that the 'solution history' is output, the solution history ...
                                  # is the number of nonlinear / linear solves that are done for each step.
  output_variables         =      # A list of the variables that should be in the Exodus output file.  If ...
                                  # this is not provided then all variables will be in the output.

  [./OverSampling]
    output_initial         = 0    # Requests that the initial condition is output to the solution file
    output_variables       =      # A list of the variables that should be in the Exodus output file.  If ...
                                  # this is not provided then all variables will be in the output.
  [../]
[]
```





## Line Search [(back to top)](#back_to_top)

Since the nonlinear solver line search option is crucial to the
performance of MOOSE, and because the way one sets these options has
changed a few times over the life of the PETSc project, the MOOSE team
has provided a uniform (command line/input file) interface for setting
these options.  Specifically, the user may specify:
```puppet
[Executioner]
  # PETSc < 3.3.0
  line_search = 'default/cubic/quadratic/none/basic/basicnonorms'

  # PETSc >= 3.3.0
  line_search = 'default/shell/none/basic/l2/bt/cp'
[]
```
With the exception of `none`, which implies `basic`, the user should
refer to the PETSc documentation for the meaning of these strings:
* [PETSc 3.2](http://www.mcs.anl.gov/petsc/petsc-3.2/docs/manualpages/SNES/SNESLineSearchSet.html), old style
* [PETSc 3.3](http://www.mcs.anl.gov/petsc/petsc-3.3/docs/manualpages/SNES/SNESLineSearchType.html), new style
* [PETSc current](http://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/SNES/SNESLineSearchType.html)

<!--- * [PETSc 3.0.0](http://www.mcs.anl.gov/petsc/petsc-3.0.0/docs/manualpages/SNES/SNESLineSearchSet.html) -->
<!--- * [PETSc 3.1.0](http://www.mcs.anl.gov/petsc/petsc-3.1.0/docs/manualpages/SNES/SNESLineSearchSet.html) -->
<!--- * [PETSc 3.4](http://www.mcs.anl.gov/petsc/petsc-3.4/docs/manualpages/SNES/SNESLineSearchType.html) -->

## Scripted Parameter Studies

The easiest way is to use the ability to override input file parameters using the command-line... then just write a script around the execution of your program.  In addition, you'll want to output CSV files so you can easily slurp them up and produce plots with them.

For instance, here is a quick Bash snippet (put it in a file with a `.sh` extension and run it like `bash something.sh`) that will run the [test/tests/postprocessor/element_integral/element_integral_test.i](https://github.com/idaholab/moose/blob/devel/test/tests/postprocessors/element_integral/element_integral_test.i) input file 5 times, varying the boundary condition and outputting the integral to a CSV file:

```
for ((bc_val=0; bc_val<5 ; bc_val++))
do
  ../../../moose_test-opt -i element_integral_test.i BCs/right/value=$bc_val Outputs/csv=true Outputs/file_base=out_$bc_val
done
```

This will produce `out_0.csv` through `out_4.csv` files that have the right data in them.  All that's left is to pick them up and parse them using something like Python to produce plots.

