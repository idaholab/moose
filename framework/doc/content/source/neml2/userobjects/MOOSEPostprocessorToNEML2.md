# MOOSEPostprocessorToNEML2

!syntax description /UserObjects/MOOSEPostprocessorToNEML2

!alert note
Users are +NOT+ expected to directly use this object in an input file. Instead, it is always recommended to use the [NEML2 action](syntax/NEML2/index.md).

## Description

This family of objects collect a MOOSE postprocessor value given by [!param](/UserObjects/MOOSEPostprocessorToNEML2/from_moose) for use as a NEML2 input variable or model parameter [!param](/UserObjects/MOOSEPostprocessorToNEML2/to_neml2). The given postprocessor value is broadcast to all quadrature points.

The naming convention is

```
MOOSE[Old]PostprocessorToNEML2
```

For example, `MOOSEPostprocessorToNEML2` gathers the postprocessor value from the +current+ time step, and `MOOSEOldPostprocessorToNEML2` gathers the postprocessor value from the +previous+ time step.

!syntax parameters /UserObjects/MOOSEPostprocessorToNEML2
