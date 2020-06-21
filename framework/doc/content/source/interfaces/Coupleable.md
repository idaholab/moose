# Coupleable

This class provides API for coupling different kinds of variables values into MOOSE systems.
The following table summarizes the methods and kinds of values they provide:

| Method | Description |
| - | - |
coupledValue | Values of a coupled variable in q-points
coupledGradient | Gradients of a coupled variable in q-points
coupledSecond | Second derivatives of a coupled variable in q-points
adCoupledValue | Values of a coupled variable in q-points with automatic differentiation info
adCoupledGradient | Gradients of a coupled variable in q-points with automatic differentiation info
adCoupledSecond | Second derivatives of a coupled variable in q-points with automatic differentiation info
coupledNodalValue | Values of a coupled variable at nodes.
adCoupledNodalValue | Values of a coupled (vector) variable at nodes with automatic differentiation info
coupledVectorValue | Values of a coupled vector variable in q-points
adCoupledVectorValue | Values of a coupled vector variable in q-points with automatic differentiation info
coupledCurl | Curl of a coupled vector variable in q-points
coupledDot | Time derivative of a coupled variable
coupledDotDu | Derivative of a time derivative of a coupled variable
coupledNodalDot | Nodal value of the time derivative of a coupled variable
coupledVectorDot | Time derivative of a coupled vector variable
coupledVectorDotDu | Derivative of a time derivative of a coupled vector variable
adCoupledLowerValue | Value a coupled lower-dimensional variable with automatic differentiation info

For values, gradients and second derivatives, users can request old and older values in case they are running a transient simulation.
In case of old and older values, the methods are called `coupledValueOld` and `coupledValueOlder`, respectively.
Similarly,

## Optional Coupling

To determine if a variable was coupled, users can use `isCoupled` method.
The typical use case looks like this:

```
_value(isCoupled("v") ? coupledValue("v") : _zero)
```

However, this use case became obsolete and now it is recommended to use default values for optionally coupled variables, see the following example:

```
validParams<Class>()
{
  InputParameters params = validParams<BaseClass>();
  params.addCoupledVar("v", 2., "Coupled value");
  ...
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
validParams<B>()
{
  InputParameters params = validParams<A>();
  params.addRequiredCoupledVar("v", "Coupled value");
  ...
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
validParams<B>()
{
  InputParameters params = validParams<A>();
  params.addCoupledVar("v", {1, 2, 3}, "Coupled value");
  ...
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

!listing moose/framework/include/interfaces/Coupleable.h start=coupled-dof-values-begin end=coupled-dof-values-end include-start=false
