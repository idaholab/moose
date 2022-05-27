MOOSE performs several sanity checks when parsing the input file and command line parameters and after all objects for the simulation have been constructed. There are several classes of errors that are covered.

# Parsing

The HIT (formerly GetPot) syntax is strict but fairly easy to use
and understand. The syntax consists of nesting blocks delineated by square brackets. Name/value pairs are then used to set parameters
throughout the system. The token rules are fairly loose. Most tokens are allowed (letters, numbers, underscores). Notably, spaces may
not occur inside any section names, nor parameter names. When the parser runs, several different kinds of errors are detected:

- Syntax errors: (missing brackets, lack of closing section, mismatched quotes - single or double, etc.)
- Semantic errors: incorrect types (numbers instead of strings, incorrect MOOSE types, etc)

# Missing and incorrect parameters

MOOSE will report errors when requirement parameters are missing. MOOSE will also report warning when unknown (mispelled) parameters
are supplied but are not used. As a developer you should use `paramError()` or `paramWarning()`, when creating error messages
related to input file parameters. These "param" type errors will be used by the parser to generate line number information for the
offending problematic parameters.

# Simulation Sanity Checks.

MOOSE performs several sanity checks just before the simulation runs. This includes several different kinds of checks

- Each block must have an active Kernel (Part of a PDE)
- Each "consumed" material property must be supplied (checked on each subdomain and boundary)
- Each specified block or boundary must exist in the mesh.
- Each "active" or "inactive" section in the input must exist
- Invalid object types are flagged as invalid
- All coupling parameters must refer to valid variables
- All Functions must refer to valid functions or must be parsed as a valid function expression.
- All Postprocessors must refer to valid Postprocessors in the input file.
- All other system "coupling" parameters are checked for validity
- All pluggable systems must refer to the right type of variable (scalar, vector, array, "nonlinear", "auxiliary", etc)
- Material properties must be defined on every block or boundary where they are consumed.
- Multiple material properties with the same name be defined on overlapping blocks/boundaries.
- Variable definition on boundary restricted objects. Nodal object integrity
  checking is controlled with
  [!param](/Problem/FEProblem/boundary_restricted_node_integrity_check). Elemental object
  integrity checking is controlled with
  [!param](/Problem/FEProblem/boundary_restricted_elem_integrity_check). Checked objects include
    - Nodal user objects
    - Nodal auxiliary kernels
    - Nodal boundary conditions
    - Side user objects
    - Elemental auxiliary kernels
    - Integrated boundary conditions

Some of these checks can be disabled. This often comes in handy for testing purposes. One of the most common
pair of parameters used in testing can be used to disable the kernel coverage check when a solve is not necessary.

```
# Disables the kernel coverage check and skips solving the system
[Problem]
  kernel_coverage_check = false
  solve = false
[]
```
