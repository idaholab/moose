# AbaqusUserElement

!syntax description /UserObjects/AbaqusUserElement

## Description

The `AbaqusUserElement` user object is used to execute Abaqus UEL plugins that
users code with the purpose of building finite element kinematics (i.e.
interpolation functions), numerical treatments (e.g. locking correction),
Jacobian of the forces with respect to the displacements and the forces
themselves. These plug-ins can be coded in Fortran (`.f` and `.f90` file
extensions) or C/C++  (`.c` and `.C` file extensions) and can be located in the
`plugins` directory of the app or another appropriate directory such as
`examples`.

Note that one can combine a UEL routine with UMAT routines such that the UEL
coder is responsible for calling a UMAT routine compiled within the same
plugin.

!alert note
When state variables are needed, the user is responsible for prescribing them
in the input files via `num_state_vars =` in the `AbaqusUserElement` user
object.

Various forms of verification of this interface have been carried out. The most
general is that defined by a beam subjected to external loading. The internal
forces are a function of strain-dependent state variables and two external
fields that vary spatially. Results between a UEL of triangular elements and
the corresponding model set up in MOOSE with a call to an equivalent UMAT routine
match to a relative tolerance of $10^{-6}$.

The input file that calls the UEL does so from the `AbaqusUserElement` block:

!listing /test/tests/uel/small_test_uel_states_fields_gradient.i block=UserObjects

The equivalent input file that performs all the setup in MOOSE, except the computation
of internal forces, uses the `AbaqusUMATStress` plugin.

!listing /test/tests/uel/small_test_umat_states_fields_gradient.i block=Materials/umat

## Interface

The UEL plugin entry function signature is defined in the `AbaqusUserElement.h` header file

!listing modules/tensor_mechanics/include/userobjects/AbaqusUserElement.h start=typedef end=validParams

### Output parameters

The UEL routine sets `RHS`, `AMATRX`, `SVARS`, `ENERGY`, and `PNEWDT`

- `RHS` is the residual contribution for the DOFs associated with the current element
- `AMATRIX` are the Jacobian contributions for the DOFs associated with the current element
- `SVARS` are stateful properties (persistent across timesteps) that are managed by the user object similar to stateful material properties in MOOSE
- `ENERGY` array of 6 energy quantities (currently not used by MOOSE)

  1. Kinetic energy.
  2. Elastic strain energy.
  3. Creep dissipation.
  4. Plastic dissipation.
  5. Viscous dissipation.
  6. Artificial strain energy stemming from e.g. artificial stiffness to control singular modes
  7. Electroststic energy
  8. Incremental work done by loads applied through the UEL routine

- `PNEWDT` is a new recommended simulation time step (currently not used by MOOSE)

### Input parameters

Please consult the Abaqus user manual for more documentation on the UEL plugin parameters.

!syntax parameters /UserObjects/AbaqusUserElement

!syntax inputs /UserObjects/AbaqusUserElement

!syntax children /UserObjects/AbaqusUserElement
