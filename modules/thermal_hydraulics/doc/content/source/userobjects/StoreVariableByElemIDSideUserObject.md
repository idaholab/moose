# StoreVariableByElemIDSideUserObject

This user object stores a variable's values (for each face quadrature point)
by element ID along a boundary. These values can then be retrieved with

```
const auto & var_values = getVariableValues(elem_id);
```

This user object is used by [HSCoupler2D3D.md].

!syntax parameters /UserObjects/StoreVariableByElemIDSideUserObject

!syntax inputs /UserObjects/StoreVariableByElemIDSideUserObject

!syntax children /UserObjects/StoreVariableByElemIDSideUserObject
