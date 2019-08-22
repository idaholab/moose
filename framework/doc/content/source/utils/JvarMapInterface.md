# JvarMapInterface

The `JvarMapKernelInterface` and `JvarMapIntegratedBCInterface` interface
classes (veneers), for Kernels and integrated boundary conditions respectively,
provide a mapping from the the value of the `jvar` parameter passed into
`computeQpOffDiagJacobian` method to

- an index into the `_coupled_moose_vars` vector
- a `coupledValue(var_name, i)` index `i` for a given `var_name` using the `getParameterJvarMap` method

This class is useful in conjunction with
[`DerivativeMaterialInterface`](DerivativeMaterialInterface.md), where vectors
of material property derivatives with respect to coupled variables are pulled in.

## Mapping `jvar` to a `_coupled_moose_vars` index

In case an index into the vector of *all* coupled variables for the current object
(`_coupled_moose_vars` vector) is requested

```c++
  unsigned int cvar = mapJvarToCvar(jvar);
```

should be used. When called from `computeQpOffDiagJacobian` with the method's
`jvar` parameter passed in, `mapJvarToCvar` is guaranteed to return a valid index
into `_coupled_moose_vars`. The off diagonal Jacobian methos is only called for
variable numbers that are found in the `_coupled_moose_vars` vector, and those
are mapped in the global jvar map which the `JvarMapInterface` classes construct
automatically.

## Mapping `jvar` to a `coupledValue(parameter_name, index)` index

In case an index is requested that can be passed into `coupledValue` for a given
input parameter name (vector variable coupling), a parameter specific map has to
be obtained. This map can be build and fetched (by reference) in the initializer
list of the class constructor using

```c++
  _myvar_map(getParameterJvarMap("myvar"))
```

where the class should have a member

```c++
  const JvarMap & _myvar_map;
```

and an input parameter

```c++
  params.addCoupledValue("myvar", "Vector of coupled variables");
```


The index into a specific coupled variable vector for a given `jvar` can then be
obtained using

```c++
  int pvar = mapJvarToCvar(jvar, _myvar_map);
  if (pvar >= 0)
  {
    // jvar points to an entry in the myvar vector
  }
```

A negative return value indicates that the `jvar` value does not point to a
variable in the couple variable vector corresponding to the mapped parameter.
