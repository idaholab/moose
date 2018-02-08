  The MOOSE environment can be modified by several environment variables:

* `METHOD`
    - Determines the type of binary file created when compiling a MOOSE-based application:
        * `opt`: An optimized binary (the default)
        * `dbg`: A binary with no optimization that contains full debugging symbols
        * `oprof`: An optimized binary with debugging symbols


* `METHODS`
    * A space separated list of the `METHOD`s used to build libMesh during the `update_and_rebuild_libmesh.sh` script.


* `MOOSE_JOBS`
    * The number of simultaneous compile jobs to run when building libMesh
    * When using our redistributable package this number is automatically detected


* `MOOSE_PROMPT`
    * If set to `true` before sourcing the MOOSE environment from our redistributable package then the BASH prompt for your console will be colored.


* `MOOSE_JUMP`
    * If set to `true` before sourcing the MOOSE environment then [autojump](https://github.com/joelthelion/autojump) will be enabled.


* `MOOSE_PPS_WIDTH`
    * Determines the max character width of the Console Postprocessor table. Set this variable to prevent wrapping.


* `MOOSE_MPI_COMMAND`
    * Set this variable to override the MPI prefix used by the TestHarness. (Default: `mpiexec -host localhost`).
