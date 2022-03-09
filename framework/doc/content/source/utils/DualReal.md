# DualReal

`DualReal` is the type underpinning all of MOOSE's automatic differentiation (AD)
capabilities.

[#overview]
[#timings]

## Overview id=overview

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
    - Indices storage allows sparse operations and hence flexibility
    - Avoid looping over the whole size of the std::array by
      maintaining a dynamic size data member

The default configuration in MOOSE uses `NumberArray` for derivative storage
because it is generally the fastest container (albeit
inflexible). `SemiDynamicSparseNumberArray` can be selected as the derivative
storage type by running `./configure --with-derivative-type=sparse` in MOOSE's
framework directory. The underlying derivative storage array size for both
`NumberArray` and `SemiDynamicSparseNumberArray` can be modified by running
`configure` with the option `--with-derivative-size=<n>` where `<n>` is the
desired size of the container. By default, MOOSE is configured `--with-derivative-size=53`.

## AD-Related Timings id=timings

[#stabilized_ins]

### Stabilized Incompressible Navier Stokes id=stabilized_ins

#### Test specs

- Navier-Stokes test directory input file: `ad_lid_driven_stabilized.i`
- 200x200 mesh
- default backing array size of 50
- Computer specs:
    - MacBook Pro running Mojave 10.14.5
    - 2.4 GHz Intel Core i9
    - 32 GB 2400 MHz DDR4

#### SNESComputeJacobian timing

- MOOSE non-sparse config: 9.61 seconds
- MOOSE sparse config: 9.22 seconds
