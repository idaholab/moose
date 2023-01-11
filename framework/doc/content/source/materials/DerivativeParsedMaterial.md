# DerivativeParsedMaterial

!syntax description /Materials/DerivativeParsedMaterial

This material class does everything the `ParsedMaterial` does, plus automatic
symbolic differentiation of the function expression. The function material
property derivatives follow a naming scheme defined in
`DerivativeMaterialPropertyNameInterface`. The maximum order of derivatives
generated is set using the `derivative_order` parameter.

Only required derivatives will be evaluated (e.g. the split operator kernel does
not require third order derivatives. Second-order derivatives are only required
for the Jacobian, as discussed [here](../)).

Non linear and auxiliary variables declared in the
[!param](/Materials/DerivativeParsedMaterial/coupled_variables) parameter, constants declared in
[!param](/Materials/DerivativeParsedMaterial/constant_names) and
[!param](/Materials/DerivativeParsedMaterial/constant_expressions), material properties
declared in [!param](/Materials/DerivativeParsedMaterial/material_property_names), and
postprocessors ([!param](/Materials/DerivativeParsedMaterial/postprocessor_names)) may be
used in the parsed function expression. Note that the constants can be defined
using parsed expressions as long as these expressions only use numbers and/or
constants already defined to the left of the current constant, line in this
example:

```yaml
    constant_names       = 'T    kB         E'
    constant_expressions = '300  8.6173e-5  T*kB'
```

where `E` can be defined in terms of `T` and `kB`, as those constants are to the
left of `E`.

If a material property `M` is listed in
[!param](/Materials/DerivativeParsedMaterial/material_property_names) a special syntax
(`M(c1,c2)` where `c1` and `c2` are variables) can be used to declare variable
dependencies  as well as selecting derivatives of material properties (for
example, `d2M:=D[M(c1,c2),c2,c2]` would make the second derivative of `M` with
respect to `c2` available as `d2M` in the parsed function expression). If
variable dependencies are declared, the necessary derivatives of the coupled
material properties will be automatically pulled in when constructing the
derivatives of the parsed function.

In phase field, an application would be the definition of a mobility term

\begin{equation}
M = \frac D{\frac{\partial^2 F}{\partial c^2}}
\end{equation}

containing the second derivative of a function $F$, or a custom switching
function derivative in a Grand potential model

!listing modules/phase_field/test/tests/GrandPotentialPFM/GrandPotentialPFM.i block=Materials/coupled_eta_function

The *ft* defined above would have accurately constructed automatic derivatives
w.r.t. $\eta$ (`eta`), which contain second and higher derivatives of $h$ (make
sure to set the `derivative_order` of $h$ high enough!).

The [!param](/Materials/DerivativeParsedMaterial/material_property_names) are parsed by
the [`FunctionMaterialPropertyDescriptor` class](http://mooseframework.org/docs/doxygen/modules/classFunctionMaterialPropertyDescriptor.html),
which understands the following syntax:

| Expression | Description |
| - | - |
| `F` | A material property called *F* with no declared variable dependencies (i.e. vanishing derivatives) |
| `F(c,phi)` | A material property called *F* with declared dependence on 'c' and 'phi' (uses `DerivativeFunctionMaterial` rules to look up the derivatives) using the round-bracket-notation |
| `d3x:=D[x(a,b),a,a,b]` | The third derivative $\frac{\partial^3x}{\partial^2a\partial b}$ of the a,b-dependent material property *x*, which will be referred to as `d3x` in the function expression |
| `dF:=D[F,c]` | Derivative of *F* w.r.t. *c*. Although the c-dependence of *F* is not explicitly declared using the round-bracket-notation it is implicitly assumed as a derivative w.r.t. *c* is requested |

Add `outputs=exodus` to the material block to automatically write all
derivatives and the function to the exodus output.

!syntax parameters /Materials/DerivativeParsedMaterial

!syntax inputs /Materials/DerivativeParsedMaterial

!syntax children /Materials/DerivativeParsedMaterial
