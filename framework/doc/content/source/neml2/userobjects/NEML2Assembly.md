# NEML2Assembly

!if! function=hasCapability('neml2')

This userobject loops through elements to cache common assembly information including:

- Number of elements
- Number of quadrature points per element
- The product of $\det(J)$, quadrature weight, and coordinate transformation factor

## Implementation details

Once the caching is done, this object does not actively update the assembly information unless the `invalidate()` method is called. The `upToDate()` method can be used to check if the current cache is up-to-date.

The method `JxWxT()` returns a NEML2 tensor with batch shape $(n_e, n_q)$, where $n_e$ is the number of elements, and $n_q$ is the number of quadrature points per element. A constant reference is returned by this method, and the tensor value is only "ready" after the `finalize()` is called for the first time.

## Syntax

!syntax parameters /UserObjects/NEML2Assembly

!if-end!

!else

!include neml2/neml2_warning.md
