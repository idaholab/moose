# Postprocessor System

A postprocessor is an object that computes a single scalar (`Real`) value,
such as a value sampled from the solution at a point in the domain, or an integral/average
over some subdomain or boundary. This value may be used purely for output purposes,
or it may be retrieved by other systems via the `getPostprocessorValue` method,
which is available in most MOOSE objects. Furthermore, postprocessors are also
[functors](Functors/index.md), which allows them to be retrieved into various
objects via the `getFunctor<Real>` method.

MOOSE includes a large number of postprocessors within the framework, the complete list is
provided in [Available Objects list](#available-objects) section.

!alert note Consider using a Reporter Object
The [Reporters/index.md] is a newer, more flexible system for computing aggregate values. It is recommended
that new objects for aggregate calculations use the Reporter system.

## Example Input File

The following input file snippet demonstrates the use of the
[ElementExtremeValue](ElementExtremeValue.md) to compute the minimum and maximum of the solution
variable "u".

!listing element_extreme_value.i block=Postprocessors

This snippet is a part of a test that may be executed using the MOOSE test application as follows.

```bash
cd ~/projects/moose/test
make -j8
cd tests/postprocessors/element_extreme_value
~/projects/moose/test/moose_test-opt -i element_extreme_value.i
```

The data from this calculation is reported in the terminal output by default and if [Exodus.md]
output is enabled the values will automatically be included in the output file. It is also possible
to export the data to a comma separated value (csv) file by enabling the [CSV.md]
object within the [Outputs](syntax/Outputs/index.md) block.

```bash
Postprocessor Values:
+----------------+----------------+----------------+
| time           | max            | min            |
+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e+00 |   9.788675e-01 |   2.113249e-02 |
+----------------+----------------+----------------+
```

## Coupling Example Code

The values computed within a Postprocessor object may be used within other objects that inherit
from the [PostprocessorInterface](interfaces/PostprocessorInterface.md), which is nearly every
system within MOOSE. For example, the [PostprocessorNeumannBC.md] object allows for a
Neumann boundary condition to be set to a value computed from a postprocessor; this object will
be used as example to demonstrate how coupling is performed.

To understand how the coupling is coded it is beneficial to first see how the coupling is defined
via the input file. The following input file snippet shows that a [PointValue.md] postprocessor
is created and named "right_pp" and the [PostprocessorNeumannBC.md] uses this value to set the
boundary condition.

!listing pp_neumann.i block=Postprocessors BCs

This first step of coding this type of coupling begins by adding the necessary input file syntax to
the object that requires a postprocessor value, PostprocessorNeumannBC in this example. In all MOOSE
objects input file syntax is governed by the validParams function of an object. To add the ability
to couple a postprocessor, simply add a new parameter using the `PostprocessorName` type, as shown
below. Notice, that the add parameters call includes a default value that makes the use of the
postprocessor optional.

!listing PostprocessorNeumannBC.C start=template end=PostprocessorNeumannBC::

The actual postprocessor value must be assigned to a member variable of the class, thus in the header
a member variable must be created, which should always be a constant reference to a
`PostprocessorValue` type. Since this is a reference it must be initialized, this occurs in the
source file by calling the `getPostprocessorValue` method and providing the name used in the
validParams function. The following snippets show declaration of the reference in the header and
the initialization of this reference in the source file.  The `_value` member variable is then
available for use anywhere inside the object, for the case of the boundary condition it is utilized
in the computation of the residual.

!listing PostprocessorNeumannBC.h line=PostprocessorValue

!listing PostprocessorNeumannBC.C start=PostprocessorNeumannBC:: end=} include-end=true

### Coupling to other values

Just as Postprocessor values can be used in other objects, Postprocessors themselves can couple to
Functions and Scalar Variables. See the following example that couples a scalar variable into a
Postprocessor:

!listing scalar_coupled_postprocessor_test.i block=Postprocessors

## Creating a `Postprocessor` Object

In general, every Postprocessor object has two methods that must be defined "execute" and
"getValue".

First, consider the execute method. This method is called by MOOSE at different time
depending on the type of postprocessor object. Therefore, when creating a Postprocessor object
the new object should inherit from one of the following C++ classes:

- +GeneralPostprocessor+: "execute" is called once on each execution flag.
- +NodalPostprocessor+: "execute" is called for each +node+ within the mesh on each execution flag.
- +ElementalPostprocessor+: "execute" is called for each +element+ within the mesh on each execution
   flag.
- +InternalSidePostprocessor+: "execute" is called for each +side+, that is not on a boundary,
   within the mesh on each execution flag.
- +SidePostprocessor+: "execute" is called for each +side+, that is on a boundary, within the mesh
   on each execution flag.

The use of execution flags is discussed in the [Execute On](#execute-on) section.

The getValue method is responsible for returning the value of the postprocessor object, this
value is what is used by all objects that are coupled to the postprocessor. In some cases the
necessary communication is performed within this method, but in general this following is preferred.

### Parallel Considerations

When creating a postprocessor it is often necessary to perform some parallel communication
to ensure that the value being computed is correct across processes and threads. Three additional
methods exists for making this process as simple as possible.

- `initialize`: This is called prior to the execution of the postprocessor and should be used
   to setup the object to be in a known state. It is important to point out that execution
   in this context includes all calls to the execute method. For example, for a `NodalPostprocessor`
   object the initialize method is called and then the execute method is called for all nodes.
- `finalize`: This is called after the execution of the postprocessor and is intended to perform
   communication to prepare the object for the call to the getValue method.
- `threadJoin`: This is called after the execution of the postprocessor and is intended to perform
   aggregation for shared memory parallelism.

To understand the use of these methods the [AverageNodalVariableValue.md] postprocessor shall be
used as an example. As the name suggests this postprocessor computes the average of the value
of a variable at the nodes. To perform this calculation the variable values from each node
are summed as is the number of values within the execute method. Then the getValue method
returns the average by returning the sum divided by the count. The following snippet shows the
these two methods: the `_u[_qp]` is the value of the variable at the current node that comes
from a shared base class and  `_sum` and `_n` are a member variables defined within class for
performing the calculation.

!listing postprocessors/AverageNodalVariableValue.C start=doco-execute-get-start end=doco-execute-get-end include-start=false

In parallel, the calls to the execute method occur on each process or thread on a subset of the
domain, in this case nodes. Therefore, the computed values must be combined to get the actual
summations required to compute the average value. The first step is to setup the state
of this calculation within the initialize method, which in this example sets the
`_sum` and `_n` member variables to zero.

!listing postprocessors/AverageNodalVariableValue.C start=doco-init-start end=doco-init-end include-start=false

After the aforementioned execute method is called for each node the computed values for `_sum` and
`_n` must be aggregated from across processes to the root processes. For this problem a gather
operation is required to collect the values computed on all processes to the root process. This is
accomplished via the `gatherSum` method.

!listing postprocessors/AverageNodalVariableValue.C start=doco-final-start end=doco-final-end include-start=false

Of course, the type of communication necessary depends on the calculation being performed. The
[UserObject.md] base class includes helper methods for common parallel communications functions.

The initialize and finalize methods are utilized to aggregate for message passing (MPI) based
parallelism. For shared memory parallelism the threadJoin method is used. This method is called,
like finalize, after execution is complete and includes a single argument. This argument is a
reference to a UserObject, which is a base class of Postprocessor objects. The purpose of this
method is to enable the aggregation for the Postprocessor objects that were executed on other
threads to the object on the root thread. For the AverageNodalVariableValue postprocessor the
values for `_sum` and `_n` on the root process object are updated to include the these same values
from the other threads.

!listing postprocessors/AverageNodalVariableValue.C start=doco-thread-start end=doco-thread-end include-start=false

## Execute On... id=execute-on

Postprocessor objects inherit from the [SetupInterface.md] that allows the objects to execute and
varying and multiple times during a simulation, such as during initialization and at the end of
each time step. Refer to the [SetupInterface.md] for additional information.

## Using Old and Older values

MOOSE maintains previously computed values in the postprocessor system for using lagged information
in a computation. Both the previous time step's value and the value computed two steps back may
be retrieved. One reason you might use older values is to break cyclic dependencies. MOOSE does
not consider a dependence on an old value when considering the order of evaluation among objects
with dependencies.

!syntax list /Postprocessors objects=True actions=False subsystems=False

!syntax list /Postprocessors objects=False actions=False subsystems=True

!syntax list /Postprocessors objects=False actions=True subsystems=False
