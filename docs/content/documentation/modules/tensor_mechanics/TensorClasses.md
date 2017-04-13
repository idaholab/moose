#Tensor Representation and Utility Classes

 The basis for Tensor Mechanics is the tensor representation classes.  The two base classes are RankTwoTensor and RankFourTensor, which as expected hold 3x3 values and 3x3x3x3 values, respectively.  A suite of operators and get/set methods are available.

##Specifying values from an input file


Both `RankTwoTensor` and `RankFourTensor` allow a user to specify how to the tensor from an input file.  `RankTwoTensor` takes a vector of six or nine inputs.  If six inputs are used, the appropriate symmetries are maintained ($ \sigma_{ij} = \sigma_{ji} $). `RankFourTensor` takes a vector of inputs of the appropriate length to fill in the tensor, with the appropriate symmetries maintained ($ C_{ijkl} = C_{klij}, C_{ijkl} = C_{ijlk}, C_{ijkl} = C_{jikl} $). Several fill methods are available to specify additional symmetries:

* `antisymmetric`
* `symmetric9`
* `symmetric21`
* `general_isotropic`
* `symmetric_isotropic`
* `antisymmetric_isotropic`
* `axisymmetric_rz`
* `general`

There is error checking on the input vector length and the enumerator type.

###Getting and setting values

Both RankTwoTensor and RankFourTensor allow a user to get and set values from the tensor using the bracket `()` notation.  

    RankTwoTensor a;
    a(i,j) = val;
sets the `i,j` component of the tensor to `val`. We use zero based indexing for the dimensions (0, 1, and 2).

    RankFourTensor b;
    b(i,j,k,l) = val;
sets the `i,j,k,l` component of the tensor to `val`. We use zero based indexing for the dimensions (0, 1, and 2).

Use the same notation to read tensor components.

    RankTwoTensor a;
    RankFourTensor b;
    Real c;
    c = a(0,0);
    c = b(0,0,0,0);


##Operators

A wide array of mathematical operators exist for the tensors in TensorMechanics.

### RankTwoTensor
The following operators are available for RankTwoTensor, with the values in parentheses indicating what the type of the other object to be subject to the operation.

    =
    += (RankTwoTensor)
    -= (RankTwoTensor)
    *= (Real, RankTwoTensor)
    /= (Real)
    + (RankTwoTensor)
    - (RankTwoTensor)
    * (Real, RankTwoTensor, TypeTensor<Real>)
    / (Real)

In addition, many methods are available for additional matrix operations:

* `zero()`
* `transpose()`
* `L2norm()`
* `row(int)` returns a TypeVector<Real>
* `rotate(RealTensorValue)`
* `rotate(RankTwoTensor)`
* `rotateXyPlane(Real)`
* `doubleContraction()`
* `deviatoric() `traceless part
* `trace()`
* `dtrace()` derivatives of `trace()` wrt tensor entries
* `secondInvariant()` second invariant of the symmetric part of `deviatoric()`
* `dsecondInvariant()`  derivatives of `secondInvariant()` wrt tensor entries
* `d2secondInvariant()`  second derivatives of `secondInvariant()` wrt tensor entries
* `thirdInvariant()` third invariant of the symmetric part of deviatoric, i.e. `((deviatoric() + deviatoric().transpose())/2).det()`
* `dthirdInvariant()`  derivatives of `thirdInvariant()` wrt tensor entries
* `d2thirdInvariant()`  second derivatives of `thirdInvariant()` wrt tensor entries
* `sin3Lode()`  sine of three times the Lode angle
* `dsin3Lode()`  derivatives of `sin3Lode()` wrt tensor entries
* `d2sin3Lode()`  second derivatives of `sin3Lode()` wrt tensor entries
* `det()` determinant
* `ddet()` derivatives of `det()` wrt tensor entries
* `inverse()`
* `symmetricEigenvalues()`  eigenvalues of symmetric part of tensor
* `dsymmetricEigenvalues()`  derivatives of symmetricEigenvalues wrt the tensor entries
* `d2symmetricEigenvalues()` second derivatives of symmetricEigenvalues wrt the tensor entries

These methods are thoroughly tested using CPPUNIT.


### RankFourTensor
The following operators are available for RankFourTensor, with the values in parentheses indicating what the type of the other object to be subject to the operation.

    =
    += (RankFourTensor)
    -= (RankFourTensor)
    *= (Real)
    /= (Real)
    + (RankFourTensor)
    - (RankFourTensor)
    * (RankTwoTensor, RealTensorValue, Real)
    / (Real)

In addition, many methods are available for additional matrix operations:
