When developing MOOSE Kernels bugs in the implementation of the On- and Off-Diagonal Jacobian may lead to bad (or no) convergence of problems utilizing the new kernel.

One way of determining if a faulty Jacobian implementation is the problem is to run the simulation with a finite difference preconditioner. Insert 

```puppet
[Preconditioning]
  [./SMP]
    type = FDP
    full = true
  [../]
[]
```

into your input file. If the problem converges well with the (slow) finite difference preconditioner the next step is determining the exact Jacobian terms that are faulty.

MOOSE comes with a Jacobian Debugger script to assist the developers with this task. First remove the FDP preconditioner setting from the input file again. Then simply run the Jacobian Debugger on your input file as follows

```bash
$MOOSE_DIR/python/jacobiandebug/analyzejacobian.py input_file.i
```

This runs the moose application (same autodetection peacock uses) and computes the Jacobian using the user supplied ```compute[..]Jacobian()``` methods and through finite differencing the residuals. If both Jacobian matrices agree the analyzer outputs.

```
No errors detected. :-)
```

Otherwise it outputs a human readable diagnosis of the input file, which could look like this:

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

Note how the analyzer puts relative discrepancies between the hand coded and finite differenced into natural language.

The Jacobian analyzer is still under development and a major feature that is being worked on is the output of the faulty kernel class names. This is not straightforward as in an input file multiple kernels can be contributing to the residual/Jacobian of any given variable.

A couple of additional notes:

- `analyzejacobian.py` will not work if you have `solve_type = PJFNK` in an active `Preconditioning` block
- `analyzejacobian.py` turns off boundary conditions, e.g. it currently tests kernels only and not integrated boundary conditions

## Advanced

To view the available commandline options for the Jacobian Debugger run the script with the ```--help``` option

```bash
$MOOSE_DIR/python/jacobiandebug/analyzejacobian.py --help
```

Notable options are

* ```-r```, ```--resize-mesh```    
    * Perform resizing of generated meshs to speed up the testing.
    * This option will attempt to reduce the number of elements in a ```type = GeneratedMesh``` mesh by setting the ```nx```, ```ny```, and ```nz``` values to either 1 or the value given by option ```-s```
* ```-s MESH_SIZE```, ````--mesh-size=MESH_SIZE```
    * Set the mesh dimensions to this number of elements along each dimension (defaults to 1, requires ```-r``` option).
* ```-e EXECUTABLE```, ```--executable=EXECUTABLE```
    * The executable you would like to build an input file for.  If not supplied an executable will be searched for.  The searched for executable will default to the optimized version of the executable (if available).
    * As in peacock executables are first search in the current directory, and then in the parent directories.
* ```-d```, ```--debug```
    * Output the command line used to run the application.
    * This shows exactly which commands are run to acquire the data needed to analyze the Jacobian matrix. Useful to debug the analyzer itself in case of failures.
