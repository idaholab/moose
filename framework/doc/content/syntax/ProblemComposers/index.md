# ProblemComposers System

!if! function=hasCapability('mfem')

The `ProblemComposers` system allows the user to construct arbitrary
problem operators with custom inputs. This class is specifically intended for composing
user-defined custom operators which may be raw `mfem::Operator`'s optimized for specific
purposes, e.g. a physics application, a bespoke solver or preconditioner implementation, usage of an optimised third-party library, or cutting-edge mfem functionality. Users' operators may still access the wider MOOSE
multi-physics system. As of yet, the user may only provide a single, but arbitrarily convoluted, problem composer (and thus operator) object per [MFEMProblem.md]. The problem composer classes are built within the [MFEMProblem.md] class,
however the [ProblemOperator.md]s are built by MFEM executioners.

## Using a custom problem composer to plug in a custom problem operator

This custom problem operator example will follow the `CustomProblemOperator.C` test which
is based on MFEM's [ex0p](https://github.com/mfem/mfem/blob/master/examples/ex0p.cpp). Firstly, a
custom [ProblemOperator.md] class must be built. If the problem is steady, i.e exercised by the [MFEMSteady.md]
executioner, then the problem operator class inherits from `Moose::MFEM::ProblemOperator`. If the
problem is transient, i.e. exercised by the [MFEMTransient.md] executioner, then
the class must inherit from `Moose::MFEM::TimeDependentProblemOperator`. For this case, the problem
is steady and an example class may look as follows:

!listing test/include/mfem/problem_operators/CustomProblemOperator.h
         start=CustomProblemOperator
         end=};

As any typical mfem example, the class needs `Form`'s, `Coefficient`'s, BC `Array`s,
`Operator`s and solution/forcing `Vector`s, though an [EquationSystem.md] can also be used instead of or
along side the previous objects.
The case in this example is linear and the operator is only built once in the `Init()` function:

!listing test/src/mfem/problem_operators/CustomProblemOperator.C
         start=CustomProblemOperator::Init
         end=}

To set-up the problem, the
`Forms` need access to the `FESpace`s and the [PostProcessor.md] may need access to the `GridFunction`s.
Both the `FESpace`s and `GridFunction`s are owned by the `MFEMProblem` and need to be retrieved
for usage in the `ProblemOperator`. The constructor should only be used to retrieve custom
input parameters, retrieving `FESpace`s and `GridFunction`s has to be done no earlier than `Init()`.

The rest of the `Init()` function mirrors the MFEM ex0p example, i.e. build the forms, add the
integrators, assemble the forms and form the linear system.

The `Solve()` method solves the linear/non-linear system that has been setup and passes the
data to the mfem `GridFunction`s so that the [Postprocessor.md] can view the results.
The class inherits a reference to the `MFEMProblem` and `MFEMProblemData` from
`Moose::MFEM::ProblemOperator` meaning the they can be used to access the gridfunctions
and solvers.

!listing test/src/mfem/problem_operators/CustomProblemOperator.C
         start=CustomProblemOperator::Solve
         end=}

Once the [ProblemOperator.md] has been written, an [MFEMProblemComposer.md] class is needed. The
composer class must inherit from `MFEMProblemComposer` making it an `MFEMObject` and by proxy a
`MooseObject`, thus it has a fixed signature constructor and destructor, plus a `createProblemOperator()` method it must override. An example minimal class looks like:

!listing test/include/mfem/problem_composers/CustomProblemComposer.h
         start=CustomProblemComposer
         end=};

The `validParams()` method can be used to generate custom inputs for the problem composer,
the inputs can be then put in the `ProblemComposers` block of the input file. The custom inputs
in problem composer can then be passed to the problem operator in the `createProblemOperator()` method.

!listing test/src/mfem/problem_composers/CustomProblemComposer.C
         start=InputParameters
         end=}

The constructor can be left more or less empty if the operator being built has no custom
options associated with it directly (e.g. solver customisations may occur in the `MFEMProblem`
class), but in the case there are custom inputs it can be used to retrieve the input params
and store them.

!listing test/src/mfem/problem_composers/CustomProblemComposer.C
         start=CustomProblemComposer::CustomProblemComposer
         end=}

The last method to be built is `createProblemOperator()`: it simply returns a shared pointer
to the `ProblemOperator` that was defined earlier.

!listing test/src/mfem/problem_composers/CustomProblemComposer.C
         start=std::shared_ptr
         end=}

Once the object has been defined, the new `MFEMProblemComposer` object must be registered
to the MooseApp system:

!listing test/src/mfem/problem_composers/CustomProblemComposer.C
         start=registerMooseObject
         end=registerMooseObject
         include-end=True
!if-end!

!else
!include mfem/mfem_warning.md
