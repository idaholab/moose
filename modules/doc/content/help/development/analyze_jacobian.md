# Jacobian Debugger `analyze_jacobian.py`

When developing MOOSE Kernels bugs in the implementation of the On- and
Off-Diagonal Jacobian may lead to bad (or no) convergence of problems utilizing
the new kernel. MOOSE comes with a Jacobian Debugger script to assist the
developers with this task.

The Jacobian debugger uses internal PETSc functionality to create a finite
differenced Jacobian matrix from the residuals and compares it to the
implemented Jacobian (usually found in `computeQpJacobian` and
`computeQpOffDiagJacobian`).

!alert! warning
Note that MOOSE by default only uses the on-diagonal Jacobian entries and will
not call your `computeQpOffDiagJacobian` implementations at all. To test those
add the following block to your input:

```style=background:#666
[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]
```
!alert-end!

Then simply run the Jacobian Debugger on your input file as follows:

```bash
$MOOSE_DIR/python/jacobiandebug/analyzejacobian.py input_file.i
```

This runs the moose application (same autodetection peacock uses) and computes
the Jacobian using the user supplied `compute[..]Jacobian()` methods and
through finite differencing the residuals. If both Jacobian matrices agree the
analyzer outputs.

```
No errors detected. :-)
```

Otherwise it outputs a human readable diagnosis of the input file, which could
look like this:

```
Kernel for variable 's':
  (0,0) On-diagonal Jacobian is slightly off (by 0.500073 %)

Kernel for variable 't':
  (1,1) On-diagonal Jacobian is wrong (off by 100.0 %)
  (1,2) Off-diagonal Jacobian for variable 'u' is questionable (off by 4.5 %)

Kernel for variable 'u':
  (2,2) On-diagonal Jacobian needs to be implemented

Kernel for variable 'u2':
  (3,3) On-diagonal Jacobian should just return  zero
```

Note how the analyzer puts relative discrepancies between the hand coded and
finite differenced into natural language.

!alert note
During the Jacobian analysis +no solve+ is being performed. The Jacobian is
checked at exactly the given initial conditions. Set this IC carefully.

The Jacobian analyzer is still under development and a major feature that is
being worked on is the output of the faulty kernel class names. This is not
straightforward as in an input file multiple kernels can be contributing to the
residual/Jacobian of any given variable.

## Tips

- Use a `RandomIC` on each variable to make sure that the values and gradients
  of your variables are non-zero to avoid trivial Jacobians.
- Reduce the mesh in size to greatly speed up the analysis. This can be done with
  the `-r` and `-s` options.

## Advanced

To view the available commandline options for the Jacobian Debugger run the
script with the `--help` option

```bash
$MOOSE_DIR/python/jacobiandebug/analyzejacobian.py --help
```

Notable options are:

- `-r`, `--resize-mesh`

  - Perform resizing of generated meshs to speed up the testing.
  - This option will attempt to reduce the number of elements in a `type = GeneratedMesh`
    mesh by setting the `nx`, `ny`, and `nz` values to either 1 or
    the value given by option `-s`

- `-s MESH_SIZE`, ``--mesh-size=MESH_SIZE`

  - Set the mesh dimensions to this number of elements along each dimension
    (defaults to 1, requires `-r` option).

- `-e EXECUTABLE`, `--executable=EXECUTABLE`

  - The executable you would like to build an input file for.  If not supplied an
    executable will be searched for.  The searched for executable will default to
    the optimized version of the executable (if available).
  - As in peacock executables are first search in the current directory, and then
    in the parent directories.

- `-d`, `--debug`

  - Output the command line used to run the application.
  - This shows exactly which commands are run to acquire the data needed to analyze
    the Jacobian matrix. Useful to debug the analyzer itself in case of failures.

## Further notes

- `analyzejacobian.py` will not work if you have `solve_type = PJFNK` in an active
  `Preconditioning` block (having it in the `Executioner` block is fine).
- `analyzejacobian.py` turns off boundary all conditions, e.g. it currently tests
  +kernels only+ and not integrated boundary conditions.
