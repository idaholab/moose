# NEML2FEInterpolation

!if! function=hasCapability('neml2')

This user object provides an interface to NEML2 for finite element
interpolation of variables and their gradients. It loops through elements to
cache shape functions, shape function gradients, DOF maps, and optional nodal
data, then provides them as NEML2 tensors.

## Example usage

This object requires a [NEML2Assembly](NEML2Assembly.md) object to provide assembly information:

```
[UserObjects]
  [assembly]
    type = NEML2Assembly
  []
  [fe]
    type = NEML2FEInterpolation
    assembly = 'assembly'
  []
[]
```

## Implementation details

After the shape function cache has been populated, this object does not update
the function space information until `invalidateFEMContext()` is called. The
`contextUpToDate()` method reports whether the current finite element context
cache is current. Similarly, variable interpolations are not updated until
`invalidateInterpolations()` is called.

The finite element context cache is automatically invalidated when the mesh
changes, so this object works with mesh adaptivity.

### Getter methods

Several getter methods provide access to cached data. All tensors are stored in
device memory and are available only after `finalize()` is called.

| Method               | Batch shape            | Base shape | Description                                        |
| -------------------- | ---------------------- | ---------- | -------------------------------------------------- |
| `getValue`           | $(n_e, n_q)$           |            | Variable value at quadrature points                |
| `getGradient`        | $(n_e, n_q)$           | $(3)$      | Variable gradient at quadrature points             |
| `getPhi`             | $(n_e, n_{dofe}, n_q)$ |            | Shape function values                              |
| `getPhiGradient`     | $(n_e, n_{dofe}, n_q)$ | $(3)$      | Shape function gradients                           |
| `getDofMap`          | $(n_e, n_{dofe})$      |            | Local DOF indices                                  |
| `getNodalValue`      | $(n_e, n_{dofe})$      |            | Variable values at element nodes                   |
| `getNodeCoordinates` | $(n_e, n_n)$           | $(3)$      | Reference node coordinates                         |
| `getGlobalDofMap`    |                        |            | Global DOF indices (as `std::vector<dof_id_type>`) |
| `local_ndof`         |                        |            | Number of local DOFs including ghost DOFs          |

Here $n_e$ is the number of elements, $n_q$ is the number of quadrature points
per element, $n_{dofe}$ is the number of degrees of freedom per element, and
$n_n$ is the number of nodes per element.

### Restrictions

- Only variables of type `MooseVariableFE<Real>` are supported
- Variable scaling factors other than unity are not supported
- All elements handled by a single `NEML2FEInterpolation` must share the same
  number of quadrature points and the same number of DOFs per element for each
  `FEType`.
- When node coordinates are requested, all handled elements must have the same
  number of nodes.
- Split mixed element topologies or p-adaptivity into multiple block-restricted
  `NEML2FEInterpolation` and `NEML2Assembly` pairs, one per element type.
- Only the current solution is interpolated; old variable values/gradients are not provided to NEML2 through this path.
- The implementation currently assumes PETSc vectors for the solution transfer.

## Syntax

!syntax parameters /UserObjects/NEML2FEInterpolation

!if-end!

!else

!include neml2/neml2_warning.md
