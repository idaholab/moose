
# DerivativeParsedMaterial

!syntax description /Materials/DerivativeParsedMaterial

This material class does everything the `ParsedMaterial` does, plus automatic symbolic differentiation of the function expression. The function material property derivatives follow a naming scheme defined in `DerivativeMaterialPropertyNameInterface`. The maximum order of derivatives generated is set using the `derivative_order` parameter.

Only required derivatives will be evaluated (e.g. the split operator kernel does not require third order derivatives. Second-order derivatives are only required for the Jacobian, as discussed [here](../)).

Non linear and auxiliary variables declared in the `args` parameter, constants declared in `constant_names` and `constant_expressions` and material properties declared in `material_property_names` may be used in the parsed function expression. Note that the constants can be defined using parsed expressions as long as these expressions only use numbers and/or constants already defined to the left of the current constant, line in this example:

```yaml
    constant_names       = 'T    kB         E'
    constant_expressions = '300  8.6173e-5  T*kB'
```

where `E` can be defined in terms of `T` and `kB`, as those constants are to the left of `E`.

If a material property `M` is listed in `material_property_names` a special syntax (`M(c1,c2)` where `c1` and `c2` are variables) can be used to declare variable dependences  as well as selecting derivatives of material properties (for example, `d2M:=D[M(c1,c2),c2,c2]` would make the second derivative of `M` with respect to `c2` available as `d2M` in the parsed function expression). If variable dependencies are declared, the necessary derivatives of the coupled material properties will be automatically pulled in when constructing the derivatives of the parsed function.

In phase field, an application would be the definition of a mobility term

\begin{equation}
M = \frac D{\frac{\partial^2 F}{\partial c^2}}
\end{equation}

containing the second derivative of a free energy $F$ as

```yaml
  [./mob]
    type = DerivativeParsedMaterial
    args = c
    f_name = M
    material_property_names = 'd2F:=D[F(c),c,c]'
    constant_names = D
    constant_expressions = 1e-3
    function = D/d2F
  [../]
```

The mobility $M$ defined above would have accurately constructed automatic derivatives w.r.t. $c$, which contain third and higher derivatives of $F$ (make sure to set the `derivative_order` of F high enough!).

The `material_property_names` are parsed by the [`FunctionMaterialPropertyDescriptor` class](http://mooseframework.org/docs/doxygen/modules/classFunctionMaterialPropertyDescriptor.html), which understands the following syntax:

| Expression | Description |
| - | - |
| `F` | A material property called _F_ with no declared variable dependencies (i.e. vanishing derivatives)|
|`F(c,phi)` | A material property called _F_ with declared dependence on 'c' and 'phi' (uses `DerivativeFunctionMaterial` rules to look up the derivatives) using the round-bracket-notation|
|`d3x:=D[x(a,b),a,a,b]` | The third derivative $$\frac{\partial^3x}{\partial^2a\partial b}$$ of the a,b-dependent material property _x_, which will be referred to as `d3x` in the function expression|
|`dF:=D[F,c]` | Derivative of _F_ w.r.t. _c_. Although the c-dependence of _F_ is not explicitly declared using the round-bracket-notation it is implicitly assumed as a derivative w.r.t. _c_ is requested|

Add `outputs=exodus` to the material block to automatically write all derivatives and the free energy to the exodus output.

!syntax parameters /Materials/DerivativeParsedMaterial

!syntax inputs /Materials/DerivativeParsedMaterial

!syntax children /Materials/DerivativeParsedMaterial
