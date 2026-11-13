# NEML2BatchIndexGenerator

!if! function=hasCapability('neml2')

!syntax description /UserObjects/NEML2BatchIndexGenerator

!alert note
Users are +NOT+ expected to directly use this object in an input file. Instead, it is always recommended to use the [NEML2 action](syntax/NEML2/index.md).

## Description

This domain user object assigns each evaluated location a contiguous span of the NEML2 batch. A span
stores where the location starts in the batch (`begin`) and how many quadrature points it has
(`size`). Two maps are generated:

- element ID to span, for element interiors (only when [!param](/UserObjects/NEML2BatchIndexGenerator/volume) is `true`);
- `(element ID, side)` to span, for sides on the selected external boundaries and on both sides of the
  selected internal interfaces.

For example, consider 3 QUAD4 elements with IDs 0, 1, and 2, the default volume quadrature (4 points
per element), and a 2-point side quadrature. If element 1, side 1 and element 2, side 3 form a
selected interface, the maps are

```
element         span (begin, size)
0           ->  (0, 4)
1           ->  (4, 4)
2           ->  (8, 4)

element, side   span (begin, size)
(1, 1)      ->  (12, 2)
(2, 3)      ->  (14, 2)
```

The batch index of quadrature point $Q$ at a location with span $S$ is

\begin{equation}
  I = S_\text{begin} + Q, \quad 0 \le Q < S_\text{size}
\end{equation}

The order of the spans follows the mesh loop, so the actual numbers may differ from this example. The
gatherers visit the same locations in the same order, so the gathered data matches the spans.

!syntax parameters /UserObjects/NEML2BatchIndexGenerator

!if-end!

!else

!include neml2/neml2_warning.md
