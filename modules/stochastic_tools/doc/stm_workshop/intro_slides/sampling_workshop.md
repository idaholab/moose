# Sampling Methods

!---

# [Samplers](Samplers/index.md)

- Samplers define a "sampling matrix" used for stochastic analysis

!equation
\mathbf{X} \equiv
\begin{bmatrix}
x_{1,1} & x_{1,2} & \dots & x_{1,M} \\
x_{2,1} & x_{2,2} & \dots & x_{2,M} \\
\vdots & \vdots & \ddots & \vdots \\
x_{N,1} & x_{N,2} & \dots & x_{N,M}
\end{bmatrix}

- $N$ is the number of rows and $M$ is the number of columns

  - The number of rows is termed "number of samples"
  - The number of columns indicates the dimensionality of input space or "number of parameters"

- A "sample" is a slice of the matrix at a given row, i.e.:

  !equation
  \overrightarrow{x}_i \equiv
  \begin{bmatrix}
  x_{i,1} & x_{i,2} & \dots & x_{i,M}
  \end{bmatrix}

  - Where each entry is the value of a parameter at the given row

!---

# Random Sampling

!row!
!col! width=50%

- Random sampling involves building a sampling matrix filled with pseudo-random numbers.
- This random number is based on a probability "Distribution" defined for each column.
- The process is to compute a pseudo-random number ($p$) and evaluate the quantile of the distribution.

!col-end!

!col! width=40%
```
[Samplers]
  [mc]
    type = MonteCarlo
    distributions = 'uniform normal'
    num_rows = 10
  []
  [lhs]
    type = LatinHypercube
    distributions = 'uniform normal'
    num_rows = 10
  []
[]
```
!col-end!
!row-end!

!---

# [Distributions](Distributions/index.md)

Distribution objects are stand-alone classes that can be invoked by other objects as needed, very similar to the `Functions` system.

!row!

!col! width=50%

!style fontsize=60%
!listing framework/include/distributions/Distribution.h
         start=Distribution(
         end=quantile(
         include-start=False
         include-end=True
         style=width:500px

!col-end!

!col! width=10%
!!
!col-end!

!col! width=50%

!style! fontsize=60%

```
[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 0
    upper_bound = 20
  []
  [normal]
    type = Normal
    mean = 1
    standard_deviation = 0.1
  []
[]
```

!style-end!

!col-end!

!row-end!

!---

# Product Samplers

!row!
!col! width=50%
- Cartesian product sampler takes the Kronecker of $M$ linearly spaced vectors.
- Each vector is defined by three numbers: (1) start value, (2) step size, and (3) number of steps.
!col-end!

!col!
!!
!col-end!

!col! width=40%
!style! fontsize=60%
```
[Samplers]
  [cartesian_product]
    type = CartesianProduct
    linear_space_items = '10  1.5 3
                          20  1   4
                          130 10  2'
  []
[]
```
!style-end!
!col-end!
!row-end!

!row!
!col! width=50%
- Quadrature sampler creates an $M$-dimensional quadrature based on PDF weigting functions.
- Mainly meant to be used with PolynomialChaos.
- Sparse quadrature options allow for efficient integration of monomial spaces.

!media tensor_grid.svg style=width:50%;margin-left:auto;margin-right:auto;

!media smolyak_grid.svg style=width:50%;margin-left:auto;margin-right:auto;
!col-end!

!col!
!!
!col-end!

!col! width=40%
!style! fontsize=60%
```
[Samplers]
  [quadrature]
    type = Quadrature
    distributions = 'uniform normal'
    order = 4
    sparse_grid = smolyak
  []
[]
```
!style-end!
!col-end!
!row-end!

!---

# Explicitly Defined Samplers

!row!
!col! width=50%
- [CSVSampler.md] and [InputMatrix.md] samplers offer a way to explicitly define the sampling matrix.
- Useful for using STM in a workflow where sampler values are generated externally.
!col-end!

!col! width=10%
!!
!col-end!

!col! width=40%
```
[Samplers]
  [csv]
    type = CSVSampler
    samples_file = 'samples.csv'
    column_indices = '0 1 3'
  []
  [matrix]
    type = InputMatrix
    matrix = '1   2   3   4   5   6;
              10  20  30  40  50  60;
              0.1 0.2 0.3 0.4 0.5 0.6;
              2   4   6   8   10  12;
              1   4   9   16  25  36'
  []
[]
```
!col-end!
!row-end!

!---

# Workshop Distributions and Sampler

!row!

!col! width=30%

- [Uniform](Uniform.md)
- [Normal](Normal.md)
- [Weibull](Weibull.md)
- [LatinHypercube](LatinHypercubeSampler.md)

!style fontsize=50%
!listing examples/workshop/step01.i

!col-end!

!col! width=35%

!plot histogram filename=stochastic_tools/workshop/results.csv
                vectors=D
                legend=False
                layout={'width': 300, 'height': 250,
                        'xaxis': {'title': 'Diffusivity'},
                        'margin': {'b': 40, 't': 0, 'l': 50, 'r':10}}

!plot histogram filename=stochastic_tools/workshop/results.csv
                vectors=T_0
                legend=False
                layout={'width': 300, 'height': 250,
                        'xaxis': {'title': 'Fixed Left Temperature'},
                        'margin': {'b': 40, 't': 0, 'l': 50, 'r':10}}

!col-end!

!col! width=35%

!plot histogram filename=stochastic_tools/workshop/results.csv
                vectors=q
                legend=False
                layout={'width': 300, 'height': 250,
                        'xaxis': {'title': 'Source'},
                        'margin': {'b': 40, 't': 0, 'l': 50, 'r':10}}

!plot histogram filename=stochastic_tools/workshop/results.csv
                vectors=q_0
                legend=False
                layout={'width': 300, 'height': 250,
                        'xaxis': {'title': 'Right Heat Flux'},
                        'margin': {'b': 40, 't': 0, 'l': 50, 'r':10}}

!col-end!

!row-end!
