# A shortcut to adding AuxKernel associated with an AuxVariable

The canonical way of adding an AuxKernel is described [here](syntax/AuxKernels/index.md). An alternative way of adding an AuxKernel is shown below

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
