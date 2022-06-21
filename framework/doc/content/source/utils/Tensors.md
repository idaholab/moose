# Tensors

MOOSE provides the following types of tensors.

- [`RankTwoTensor`/`ADRankTwoTensor`](https://mooseframework.inl.gov/docs/doxygen/moose/classRankTwoTensorTempl.html)
- [`RankThreeTensor`/`ADRankThreeTensor`](https://mooseframework.inl.gov/docs/doxygen/moose/classRankThreeTensorTempl.html)
- [`RankFourTensor`/`ADRankFourTensor`](https://mooseframework.inl.gov/docs/doxygen/moose/classRankFourTensorTempl.html)
- [`SymmetricRankTwoTensor`/`ADSymmetricRankTwoTensor`](https://mooseframework.inl.gov/docs/doxygen/moose/classSymmetricRankTwoTensorTempl.html)
- [`SymmetricRankFourTensor`/`ADSymmetricRankFourTensor`](https://mooseframework.inl.gov/docs/doxygen/moose/classSymmetricRankFourTensorTempl.html)

These tensor classes provide a rich set of methods for commonly used algebraic operations. For a complete list of supported operations, please refer to the corresponding Doxygen documentation linked above.

## RankTwoTensor

A `RankTwoTensor` is a second order tensor containing 9 components. No symmetry is assumed.

## RankThreeTensor

A `RankThreeTensor` is a third order tensor containing 27 components. No symmetry is assumed.

## RankFourTensor

A `RankFourTensor` is a fourth order tensor containing 81 components. No symmetry is assumed.

## SymmetricRankTwoTensor

A `SymmetricRankTwoTensor` is a symmetric second order tensor containing 6 components. [Mandel notation](https://en.wikipedia.org/wiki/Voigt_notation#Mandel_notation) is used internally to store and access the components.

## SymmetricRankFourTensor

A `SymmetricRankFourTensor` is a fourth order tensor with minor symmetry containing 36 components. [Mandel notation](https://en.wikipedia.org/wiki/Voigt_notation#Mandel_notation) is used internally to store and access the components.
