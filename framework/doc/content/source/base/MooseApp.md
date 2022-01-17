# MooseApp

The `MooseApp` object is the base class for [!ac](MOOSE) based applications and serves as the
entry point for running all aspects of a simulation.

## Specifying Application to Use

It can be useful, when running a large "combined" application like `blue_crab`,
to specify a specific application type to run an input file with. For instance
one may want to run `griffin` as the main application and `pronghorn` as a
sub-application. Just as the application type for the sub-application is
specified in the main input file using `app_type = PronghornApp` the application
type for running the main input file can be specified on the command line
via `<blue_crab_root_dir>/blue_crab-* -i foo.i --app GriffinApp`. This allows
any special actions and setup specific to the application to be used.

## Nonzero reallocation behavior

The MOOSE framework and libMesh work together to supply a sparsity pattern to
PETSc which informs how much memory is allocated for the system matrix. If the
sparsity pattern is too small, then PETSc will be forced to allocate new memory
during matrix assembly if user code tries to add/set a matrix entry that wasn't
preallocated by the sparsity pattern. This allocation at matrix assembly time
can significantly slow down a simulation. MOOSE and libMesh are known to
generate accurate sparsity patterns in most cases; however, for complex
multi-body problems like mechanical and thermal contact, the sparsity pattern
may be incorrect. Because of this, by default MOOSE does not error if PETSc is
forced to do new nonzero allocations during matrix assembly. However, if an
application developer expects their physics to have accurate sparsity patterns,
they may override the default MOOSE behavior and error on new nonzero
allocations. This can give the application developer peace-of-mind that their
applications will not produce quiet nonzero allocations at run-time.

Overriding the default nonzero allocation behavior can be accomplished by
overriding the virtual function `bool
MooseApp::errorOnJacobianNonzeroReallocation() const`. The `MooseApp` default is
`false`, although this will hopefully be changed in the not-too-distant
future. Note that the application level code setting can always be overridden at
the input-file level by specifying a value for `Problem/error_on_jacobian_nonzero_reallocation`.
