# AbaqusUserElement

!syntax description /UserObjects/AbaqusUserElement

## Description

The `AbaqusUserElement` user object is used to execute Abaqus UEL plugins
that users code with the purpose of building finite element kinematics (i.e. interpolation functions),
numerical treatments (e.g. locking correction), Jacobian of the forces with respect
to the displacements and the forces themselves. These plug-ins can be coded in
Fortran (`.f` and `.f90` file extensions) or C/C++  (`.c` and `.C` file
extensions) and can be located in the `plugins` directory of the app or another appropriate directory such as `examples`.

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


!syntax parameters /UserObjects/AbaqusUserElement

!syntax inputs /UserObjects/AbaqusUserElement

!syntax children /UserObjects/AbaqusUserElement
