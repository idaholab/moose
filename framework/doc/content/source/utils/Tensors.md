# Tensors

MOOSE provides the following types of tensors.

- `RankTwoTensor`/`ADRankTwoTensor`
- `RankThreeTensor`/`ADRankThreeTensor`
- `RankFourTensor`/`ADRankFourTensor`
- `SymmetricRankTwoTensor`/`ADSymmetricRankTwoTensor`
- `SymmetricRankFourTensor`/`ADSymmetricRankFourTensor`

These tensor classes provide a rich set of methods for commonly used algebraic operations. For a complete list of supported operations, please refer to the corresponding Doxygen documentation.

## RankTwoTensor / ADRankThreeTensor

A `RankTwoTensor` is a second order tensor containing 9 components. No symmetry is assumed. An `ADRankTwoTensor` provides additional information for automatic differentiation. The complete list of available methods and operators can be found in the [RankTwoTensor Doxygen documentation](https://mooseframework.inl.gov/docs/doxygen/moose/classRankTwoTensorTempl.html).

## RankThreeTensor / ADRankThreeTensor

A `RankThreeTensor` is a third order tensor containing 27 components. No symmetry is assumed. An `ADRankThreeTensor` provides additional information for automatic differentiation. The complete list of available methods and operators can be found in the [RankThreeTensor Doxygen documentation](https://mooseframework.inl.gov/docs/doxygen/moose/classRankThreeTensorTempl.html).

## RankFourTensor / ADRankFourTensor

A `RankFourTensor` is a fourth order tensor containing 81 components. No symmetry is assumed. An `ADRankFourTensor` provides additional information for automatic differentiation. The complete list of available methods and operators can be found in the [RankFourTensor Doxygen documentation](https://mooseframework.inl.gov/docs/doxygen/moose/classRankFourTensorTempl.html).

## SymmetricRankTwoTensor / ADSymmetricRankTwoTensor

A `SymmetricRankTwoTensor` is a symmetric second order tensor containing 6 components. [Mandel notation](https://en.wikipedia.org/wiki/Voigt_notation#Mandel_notation) is used internally to store and access the components. An `ADSymmetricRankTwoTensor` provides additional information for automatic differentiation. The complete list of available methods and operators can be found in the [SymmetricRankTwoTensor Doxygen documentation](https://mooseframework.inl.gov/docs/doxygen/moose/classSymmetricRankTwoTensorTempl.html).

## SymmetricRankFourTensor / ADSymmetricRankFourTensor

A `SymmetricRankFourTensor` is a fourth order tensor with minor symmetry containing 36 components. [Mandel notation](https://en.wikipedia.org/wiki/Voigt_notation#Mandel_notation) is used internally to store and access the components. An `ADSymmetricRankFourTensor` provides additional information for automatic differentiation. The complete list of available methods and operators can be found in the [SymmetricRankFourTensor Doxygen documentation](https://mooseframework.inl.gov/docs/doxygen/moose/classSymmetricRankFourTensorTempl.html).
