# [Material System](syntax/Materials/index.md)

A system for defining material properties to be used by multiple systems and allow for variable
coupling.

!---

The material system operates by creating a producer/consumer relationship among objects

- `Material` objects +produce+ properties.
- Other MOOSE objects (including materials) +consume+ these properties.

!---

## Material Property Evaluation

Quantities are recomputed at quadrature points, as needed.

Multiple `Material` objects may define the same "property" for different parts of the subdomain or
boundaries.

!---

## Stateful Material Properties

Values of material properties can be stored at the previous ("old") and two-before ("older") time steps.

It can be useful to have "old" values of `Material` properties available in a simulation, such as
in solid mechanics plasticity constitutive models.

Traditionally, this type of value is called a "state variable"; in MOOSE, they are called
"stateful material properties".

Stateful `Material` properties require more memory.

!---

## Default Material Properties

Material properties may have default values, which can be found in the documentation,
and are automatically prefilled when using the syntax auto-complete.

Only scalar (`Real`) valued properties may have defaults.

!---

## Material Property Output

Output of `Material` properties is enabled by setting the "outputs" parameter.

The following example creates two additional variables called "mat1" and "mat2" that will show up in
the output file.

!listing output_block.i block=Materials Outputs

!---

## Supported Property Types for Output

`Material` properties can be of arbitrary (C++) type, but not all types can be output.

| Type | AuxKernel | Variable Name(s) |
| :- | :- | :- |
| Real | `MaterialRealAux` | prop |
| RealVectorValue | `MaterialRealVectorValueAux` | prop_1, prop_2, and prop_3 |
| RealStdVector | `MaterialStdVectorAux` | prop_1, prop_2, and prop_3 |
| RealTensorValue | `MaterialRealTensorValueAux` | prop_11, prop_12, prop_13, prop_21, etc. |
| DenseMatrix | `MaterialRealDenseMatrixAux` | prop_11, prop_12, prop_13, prop_21, etc. |

AD-material property types can also be output.
