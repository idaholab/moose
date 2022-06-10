# Tensor Representation and Utility Classes

 The basis for Tensor Mechanics is the tensor representation classes.  The two base classes are
 RankTwoTensor and RankFourTensor, which as expected hold 3x3 values and 3x3x3x3 values,
 respectively.  A suite of operators and get/set methods are available.

## Specifying Values from an Input File

### Full tensor notation

Both `RankTwoTensor` and `RankFourTensor` allow a user to specify how to the tensor from an input
file.

- `RankTwoTensor` takes a vector of six or nine inputs.  If six inputs are used, the appropriate
  symmetries are maintained ($\sigma_{ij} = \sigma_{ji}$).

- `RankFourTensor` takes a vector of inputs of the appropriate length to fill in
  the tensor, with the appropriate symmetries maintained
  \begin{equation}
    C_{ijkl} = C_{klij}, C_{ijkl} = C_{ijlk}, C_{ijkl} = C_{jikl}
  \end{equation}
  Several fill methods are available to specify additional symmetries as described
  in [ComputeElasticityTensor](/ComputeElasticityTensor.md).

### Symmetric Mandel notation tensors

- `SymmetricRankTwoTensor` uses the 6-vector representation of a symmetric rank two tensor.

- `SymmetricRankFourTensor` uses the 6x6 matrix representation with Mandel notation coefficients. The Mandel notation was chosen as it yields meaningful eigenvectors.

## Getting and Setting Specific Component Values

Both RankTwoTensor and RankFourTensor allow a user to get and set values from the tensor using the
bracket `()` notation.

```cpp
RankTwoTensor a;
a(i,j) = val;
```

sets the `i,j` component of the tensor to `val`. We use zero based indexing for the dimensions (0, 1,
and 2).

```cpp
RankFourTensor b;
b(i,j,k,l) = val;
```

sets the `i,j,k,l` component of the tensor to `val`. We use zero based indexing for the dimensions
(0, 1, and 2).

Use the same notation to read tensor components.

```cpp
RankTwoTensor a;
RankFourTensor b;
Real c;
c = a(0,0);
c = b(0,0,0,0);
```

## Tensor Operations

See the list of available operators and matrix operations for the RankTwoTensor,
RankThreeTensor, and RankFourTensor in the description of the MOOSE
[Utility Classes](utils/MooseUtils.md).
