# MOOSEVariableToNEML2

!syntax description /UserObjects/MOOSEVariableToNEML2

!alert note
Users are +NOT+ expected to directly use this object in an input file. Instead, it is always recommended to use the [NEML2 action](syntax/NEML2/index.md).

## Description

This family of objects collect a MOOSE variable or auxiliary variable given by [!param](/UserObjects/MOOSEVariableToNEML2/from_moose) for use as a NEML2 input variable or model parameter [!param](/UserObjects/MOOSEVariableToNEML2/to_neml2). The given variable is interpolated at each quadrature point.

The naming convention is

```
MOOSE[Old]VariableToNEML2
```

For example, `MOOSEVariableToNEML2` gathers the (auxiliary) variable from the +current+ time step, and `MOOSEOldVariableToNEML2` gathers the (auxiliary) variable from the +previous+ time step.

!syntax parameters /UserObjects/MOOSEVariableToNEML2
