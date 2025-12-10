# NEML2FEInterpolation

!if! function=hasCapability('neml2')

This userobject provides an interface to NEML2 for finite element interpolation (at quadrature points) of variables and their gradients. It loops through elements to cache shape functions, shape function gradients, and DOF maps, then provides them as NEML2 tensors for use in NEML2 models.

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

Once the shape function caching is done, this object does not actively update the function space information unless the `invalidateFEMContext()` method is called. The `contextUpToDate()` method can be used to check if the current FE context cache is up-to-date. Similarly, the variable interpolations are not actively updated until the `invalidateInterpolations()` method is called.

The FE context cache is automatically invalidated when the mesh changes, so this object works correctly with mesh adaptivity.

### Getter methods

Several getter methods are provided to access cached data. All tensors are stored in device memory and are only available after `finalize()` is called.

| Method            | Batch shape            | Base shape | Description                                        |
| ----------------- | ---------------------- | ---------- | -------------------------------------------------- |
| `getValue`        | $(n_e, n_q)$           |            | Variable value at quadrature points                |
| `getGradient`     | $(n_e, n_q)$           | $(3)$      | Variable gradient at quadrature points             |
| `getPhi`          | $(n_e, n_{dofe}, n_q)$ |            | Shape function values                              |
| `getPhiGradient`  | $(n_e, n_{dofe}, n_q)$ | $(3)$      | Shape function gradients                           |
| `getDofMap`       | $(n_e, n_{dofe})$      |            | Local DOF indices                                  |
| `getGlobalDofMap` |                        |            | Global DOF indices (as `std::vector<dof_id_type>`) |
| `local_ndof`      |                        |            | Number of local DOFs including ghost DOFs          |

where $n_e$ is the number of elements, $n_q$ is the number of quadrature points per element, and $n_{dofe}$ is the number of degrees of freedom per element.

### Restrictions

- Only variables of type `MooseVariableFE<Real>` are supported
- Variable scaling factors other than unity are not supported

## Syntax

!syntax parameters /UserObjects/NEML2FEInterpolation

!if-end!

!else

!include neml2/neml2_warning.md
