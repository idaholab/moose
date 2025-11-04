# Coupleable

This class provides an API for coupling different kinds of variables values into MOOSE systems.
The following tables summarize the methods it provides.

| Methods for scalar field variables | Description |
| :--- | :--- |
`coupledValue`*‡§ | Value of a coupled variable at q-points
`coupledGradient`*‡§ | Gradient of a coupled variable at q-points
`coupledSecond`*‡§ | Second spatial derivatives of a coupled variable at q-points
`coupledDot`†§ | Time derivative of a coupled variable at q-points
`coupledDotDot`†§ | Second time derivative of a coupled variable at q-points
`coupledDotDu` | Derivative with regards to the variable of the time derivative of a coupled variable at q-points
`coupledDotDotDu` | Derivative with regards to the variable of the second time derivative of a coupled variable at q-points
`coupledGradientDot`§ | Time derivative of the gradient of a coupled variable at q-points
`coupledGradientDotDot` | Second time derivative of the gradient of a coupled variable at q-points

---

| Methods for nodal scalar field variables | Description |
| :--- | :--- |
`coupledNodalValue`*‡§ | Value of a coupled variable at nodes
`coupledNodalDot`† | Time derivative of a coupled variable at nodes
`coupledNodalDotDot`† | Second time derivative of a coupled variable at nodes

---

| Methods for array field variables | Description |
| :--- | :--- |
`coupledArrayValue`‡ | Value of a coupled array variable at q-points
`coupledArrayGradient`‡ | Gradient of a coupled array variable at q-points
`coupledArrayDot`† | Time derivative of a coupled array variable at q-points
`coupledArrayDotDot`† | Second time derivative of a coupled array variable at q-points
`coupledArrayDotDu` | Derivative with regards to the variable of the time derivative of a coupled array variable at q-points
`coupledArrayGradientDot` | Time derivative of the gradient of a coupled array variable at q-points

---

| Methods for vector field variables | Description |
| :--- | :--- |
`coupledVectorValue`‡§ | Value of a coupled vector variable at q-points
`coupledVectorGradient`‡§ | Gradient of a coupled vector variable at q-points
`coupledCurl`‡ | Curl of a coupled vector variable at q-points
`coupledDiv`‡ | Divergence of a coupled vector variable at q-points
`coupledVectorDot`†§ | Time derivative of a coupled vector variable at q-points
`coupledVectorDotDot`† | Second time derivative of a coupled vector variable at q-points
`coupledVectorDotDu` | Derivative with regards to the variable of the time derivative of a coupled vector variable at q-points
`coupledVectorDotDotDu` | Derivative with regards to the variable of the second time derivative of a coupled vector variable at q-points

---

| Methods for nodal vector field variables | Description |
| :--- | :--- |
`coupledNodalValue`*‡§ | Value of a coupled vector variable at nodes
`coupledNodalDot` | Time derivative of a coupled vector variable at nodes

---

*: These methods are also provided with values from the previous Newton iteration
by suffixing their name with `PreviousNL`, e.g. `coupledSecondPreviousNL`.

†: These methods are also provided with values from the previous time step of a
transient simulation by suffixing their name with `Old`, e.g. `coupledDotOld`.

‡: These methods are also provided with values from the previous or second previous
time steps of a transient simulation by suffixing their name with `Old` or `Older`,
respectively, e.g. `coupledDivOld` or `coupledDivOlder`.

§: These methods are also provided with automatic differentiation info by prefixing their
name with `ad` and preserving `camelCase`, e.g. `adCoupledValue` or `adCoupledVectorDot`.

Note that all of these prefixes and suffixes are mutually exclusive, i.e. none can be mixed and matched in any way.

Lastly, some methods are only available with automatic differentiation info, e.g.
`adCoupledLowerValue` returns the value of a coupled lower-dimensional variable and `adCoupledVectorSecond` returns the second spatial derivatives of a coupled vector variable.

## Optional Coupling

To determine if a variable was coupled, users can use `isCoupled` method.
The typical use case looks like this:

```
_value(isCoupled("v") ? coupledValue("v") : _zero)
```

However, this use case became obsolete and now it is recommended to use default values for optionally coupled variables, see the following example:

```
InputParameters
Class::validParams()
{
  InputParameters params = BaseClass::validParams();
  params.addCoupledVar("v", 2., "Coupled value");
  ...
  return params;
}

Class::Class(...) : BaseClass(...),
  _v(coupledValue('v'))
```

The advantage here is that users can provide arbitrary default values to their variables.

## Coupling of Vectors of Variables

Users can couple a vector of variables using the following syntax:

```
v = 'a b c'
```

This syntax provides 3 variables coupled as a variable `v` in a MOOSE object using the `Coupleable` interface.
The number of components coupled into can be obtained by `coupledComponents` method.
Then, individual components can be obtained by calling `coupledValue` (or any other method mentioned above) passing in the variable name (as usual) and the component index. See the following example:

Declarations:

```
class B : public A
{
  ...
protected:
  unsigned int _n_vars;
  std::vector<MooseVariable *> _vars;
};
```

Implementation:

```
InputParameters
B::validParams()
{
  InputParameters params = A::validParams();
  params.addRequiredCoupledVar("v", "Coupled value");
  ...
  return params;
}

B::B(...) : A(...),
  _n_vars(coupledComponents("v"))
{
  for (unsigned int i = 0; i < _n_vars; i++)
    _vars.push_back(dynamic_cast<MooseVariable *>(getVar("v", i)));
}
```

## Defaults for Coupling of Vectors of Variables

Vectors of variables can be added using `params.addCoupledVar` as described above. The parameter class allows providing
defaults for vector variables as follows:

```
InputParameters
B::validParams()
{
  InputParameters params = A::validParams();
  params.addCoupledVar("v", {1, 2, 3}, "Coupled value");
  ...
  return params;
}
```

Coupled variable parameters can be set to constant real numbers in the input file using the syntax

```
v = '1'
```

for a single coupled variable or using

```
v = '1 2 3'
```

for a vector of coupled variables. Currently mixing actual variable assignments and defaults like this:

```
v = '1 actual_var 3'
```

is not supported.

## Coupled Solution DOFs

It is possible to retrieve the solution DOFs of an element in an elemental loop. This is different than the
"value" type coupling which holds the interpolated values of the shape functions themselves. Obtaining the raw
DOFs all the user to perform their own integration or other evaluation without going through the interpolation
process. These functions can be found here:

!listing framework/include/interfaces/Coupleable.h start=coupled-dof-values-begin end=coupled-dof-values-end include-start=false

## Coupling of variables through their names

Typically variables are coupled in through calling `params.addCoupledVar` or `params.addRequiredCoupledVar` of the valid parameters of an object.
There are cases where the variable names are provided through parameters in other types with `params.addParam` or `params.addRequiredParam`.
For these cases, this interface provides two functions `coupledValueByName` and `coupledArrayValueByName` that take a variable name directly.
This suffix `ByName` is only available for the two functions currently because they are the only onces needed by MOOSE and MOOSE applications.
More can be added as desired in the future.


## Writing directly to coupled variables

Element- and nodal user objects as well AuxKernels may obtain a writable reference to a MOOSE field variable
through the `Coupleable::writableVariable` function. The returned variable reference provides a `setDofValue` (for FE and FV variables) and `setNodalvalue` (only for FE variables) methods that can be used to set the nodal or elemental DOF value(s) of the variable.

`Coupleable::writableVariable` enforces compatibility between the calling object type and the family of the
requested variable. I.e. nodal user objects and AuxKernels may only obtain references to nodal variables, and
element user objects and elemental AuxKernels may only obtain references to elemental variables.

The block restrictions of the variables are also checked not to exceed the block restrictions of the calling object.
MOOSE keeps track of all variables to which a reference was obtained through `Coupleable::writableVariable`. Each
variable in the system may at most be written to by a single object on any given subdomain.

The user object and aux kernel thread loops check if an executed object has any writable variable references, and
if so, will insert those variables into the aux solution vector. This obviates the need for using the
[`ProjectionAux`](ProjectionAux.md) kernel.

!alert warning
`Coupleable::writableVariable` can let users write to both FE / FV from AuxKernels and UserObjects but one must exercise caution about whether Nodal or Elemental type AuxKernels / UOs are used as the quadrature would depend on this choice and might lead to segfault if a FV variable values are set using `setDofValue` function for non-zero values of `_qp` .
