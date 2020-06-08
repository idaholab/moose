# PolynomialRegressionTrainer

!syntax description /Trainers/PolynomialRegressionTrainer

## Overview

This class is responsible for determining the coefficients of a multi-dimensional polynomial which
approximates the behavior of the Quantity of Interest (QoI) in the parameter space.
For this, the object needs $N$ training points in the parameters space.
These training points can be characterized using their coordinates in the parameter space:

!equation
\textbf{X} =
\begin{bmatrix}
x_{1,1} & x_{1,2} & \dots  & x_{1,D} \\
x_{2,1} & x_{2,2} & \dots  & x_{2,D} \\
\vdots  & \vdots  & \ddots & \vdots  \\
x_{N,1} & x_{N,2} & \dots  & x_{N,D}
\end{bmatrix}
=
\begin{bmatrix}
\textbf{x}_{1} \\
\textbf{x}_{2} \\
\vdots  \\
\textbf{x}_{N}
\end{bmatrix}.

where $D$ denotes the dimension of the parameter space and $\textbf{x}$ is a
D-dimensional vector containing the coordinates in each dimension.
Similarly to other `Trainer` classes, `PolynomialRegressionTrainer` accesses this matrix
from a `Sampler` object. Of course, the trainer has to know the values of the QoI at
these coordinates as well:

!equation
\textbf{y} =
\begin{bmatrix}
y_{1} \\
y_{2} \\
\vdots\\
y_{N}
\end{bmatrix}
.

This data is accessed through a `VectorPostprocessor`. Now that all data is available,
$\textbf{y}=f(\textbf{x})$ unknown function is approximated using a polynomial expression of the following form:

!equation id=poly_exp
\textbf{y} \approx \hat{\textbf{y}} = \sum \limits_{k=1}^{N_p}P(\textbf{x}, \textbf{i}_{k})c_k=\textbf{P}(\textbf{x})\textbf{c},

where $N_p$ is the number of polynomial terms in the approximation, $\textbf{c}=[c_1,...,c_{N_p}]^T$ are the
unknown coefficients and $\textbf{P}(\textbf{x})=[P(\textbf{x}, \textbf{i}_{1}),...,P(\textbf{x}, \textbf{i}_{N_p})]$.
The used polynomials in this case can be defined as

!equation
P(\textbf{x}, \textbf{i}) = \textbf{x}^\textbf{i} = \prod \limits_{j=1}^D x_j^{i_j},

where $x_j$ denotes the $j$-th coordinate of parameter vector $\textbf{x}$,
while $\textbf{i}=(i_1,...,i_D)$ is a $D$-dimensional tuple containing the powers
for each coordinate. This tuple is the same as described in [PolynomialChaos.md].
To determine these tuples, the trainer needs an additional input parameter, namely
the maximum degree of the polynomial. This limits the number of polynomial terms in
[poly_exp]. If this number is fixed, the only unknown parameters are the elements of
$\textbf{c}$.

To determine these, a regression matrix can be defined as:

!equation
\textbf{R} =
\begin{bmatrix}
\textbf{P}(\textbf{x}_1) \\
\textbf{P}(\textbf{x}_2) \\
\vdots  \\
\textbf{P}(\textbf{x}_{N})
\end{bmatrix}.

## Ordinary Least Squares (OLS) regression

Using regression matrix $\textbf{R}$ and and Ordinary Least Squares (OLS) approach described on
[Wikipedia](https://en.wikipedia.org/wiki/Polynomial_regression) in detail, the unknown
coefficients can be determined as follows:

!equation
\textbf{c}=\left(\textbf{R}^T\textbf{R}\right)^{-1}\textbf{R}^T\textbf{y}.

Finally, it must be mentioned that this method is only applicable if $N_p \leq N$
and keeping $N_p << N$ is recommended.

## Ridge regression

Unfortunately, the OLS approach is known to have some issues like:
- It is prone to overfit the data,
- It yields inaccurate results if the input variables are correlated,
- It is sensitive to outliers.

To tackle the problem, an $L^2$ regularization (or
[Tikhonov regularization](https://en.wikipedia.org/wiki/Regularized_least_squares) is adopted
to make sure that the coefficients of the expansion do have uncontrollably high values.
This extended least squares regression is often referred to as Ridge Regression.
In this scenario the coefficients can be determined by solving:

!equation
\textbf{c}=\left(\textbf{R}^T\textbf{R}+\lambda I\right)^{-1}\textbf{R}^T\textbf{y},

where $\lambda$ is a penalty parameter which penalizes coefficients with large
magnitudes. As $\lambda \rightarrow 0$, Ridge regression converges to OLS.

## Example Input File Syntax

To get the necessary data, two essential blocks have to be included in the master input file.
The first, the sampler defined in `Samplers`, creates the coordinates in matrix $\textbf{X}$, while
the objects in `VectorPostprocessors` create, fill and store the result vector $\textbf{y}$.

!listing polynomial_regression/train.i block=Samplers

!listing polynomial_regression/train.i block=VectorPostprocessors

Similarly to [NearestPointTrainer.md], a `GFunction` vector postprocessor from [SobolStatistics.md] is
used to emulate a full-order model. This simply evaluates a function at sample points.

Using this data and the maximum degree setting (`max_degree` in the input file),
the trainer computes the model coefficients $\textbf{c}$. To control the type of regression,
the user has to set `regression_type` to either 'ols' or 'ridge' in the input file:

!listing polynomial_regression/train.i block=Trainers

!syntax parameters /Trainers/PolynomialRegressionTrainer

!syntax inputs /Trainers/PolynomialRegressionTrainer

!syntax children /Trainers/PolynomialRegressionTrainer
