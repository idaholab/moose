# AddAuxKernelAction

This action serves as a shortcut to add an AuxKernel nested within an AuxVariable block in the input file syntax.

The syntax is

```python
[AuxVariables]
  [foo]
    order = SECOND
    [AuxKernel]
      type = bar
    []
  []
[]
```

See [AuxKernel](source/auxkernels/AuxKernel.md) for more information about AuxKernels.
