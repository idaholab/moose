# DualReal

`DualReal` is a MOOSE typedef defined from the
[`MetaPhysicL`](https://github.com/roystgnr/metaphysicl) template class
`DualNumber`. `DualNumber` takes two template arguments `T` and `D`; `T`
represents the "value" type of the `DualNumber`, e.g. the type of $f(\vec{x})$,
while `D` represents the derivative type of the `DualNumber`, e.g. the type of
$\nabla f = \frac{\partial f}{\partial\vec{x}}$. `MetaPhysicL` offers several
options for `D` types, including:

1. `NumberArray`
    - C-array as underlying derivative vector storage for data
    - Fast because of static storage
    - Inflexible; no storage of indices so no sparse support. Essentially pick a
      maximum possible storage size and hope for the best. Doesn't work out for
      many 3D problems or problems with a lot of variables
2. `DynamicSparseNumberArray`
    - `std::vector` as underlying storage for derivative data and indices
    - Maximum flexibility
    - Slow because of continual dynamic allocation of memory
3. `SemiDynamicSparseNumberArray`
    - `std::array` as underlying storage for derivative data and indices
    - Fast because of static storage
    - Indice storage allows sparse operations and hence flexibility
    - Avoid looping over the whole size of the std::array by
      maintaining a dynamic size data member

Because of its flexibility and speed, `SemiDynamicSparseNumberArray` is the
structure chosen as the `D` type for MOOSE's `DualNumber`/`DualReal`.
The only real draw-back to this derivative type is that an assertion will be triggered
if the sparse size ever exceeds the static storage size
of the std::array. One could get around this by increasing the static size
of the array, but this could incur some (small?) memory overhead because of
the dynamic material property and variable value containers (underlying
`std::vector` and `MooseArray` respectively) that hold MOOSE's `DualReal`
objects.
