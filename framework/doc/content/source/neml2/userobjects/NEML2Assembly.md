# NEML2Assembly

!if! function=hasCapability('neml2')

This userobject loops through elements to cache common assembly information including:

- Number of elements
- Number of quadrature points per element
- The product of $\det(J)$, quadrature weight, and coordinate transformation factor
- Quadrature point coordinates

## Implementation details

After the cache has been populated, this object does not update the assembly
information until `invalidate()` is called. The `upToDate()` method reports
whether the current cache is current.

The method `JxWxT()` returns a NEML2 tensor with batch shape $(n_e,n_q)$,
where $n_e$ is the number of elements and $n_q$ is the number of quadrature
points per element. The method `qPoints()` returns the corresponding
coordinates with batch shape $(n_e,n_q)$ and base shape $(3)$. Both methods
return constant references, and their tensor values are available only after
`finalize()` has been called.

### Limitations

- All elements processed by a single `NEML2Assembly` must have the same number
  of quadrature points; mixed quadrature rules or mixed element orders within
  one user object result in an error.
- For mixed meshes, define separate block-restricted `NEML2Assembly` objects,
  one per element type/order, and pair each with its own
  `NEML2FEInterpolation`.

## Syntax

!syntax parameters /UserObjects/NEML2Assembly

!if-end!

!else

!include neml2/neml2_warning.md
