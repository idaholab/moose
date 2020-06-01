# PolynomialRegressionTrainer

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

where $D$ denotes the number of dimensions in the parameter space.
Similarly to other `Trainer` classes, `PolynomialRegressionTrainer` accesses this matrix
from a `Sampler` object. Of course, the trainer has to know the values of the QoI at
these locations as well:

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
the $\textbf{y}=f(\textbf{x})$ is approximated using a polynomial expression of the following form:

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
To determine these tuples the trainer needs an additional input parameter, namely
the maximum degree of the polynomials. This limits the number of polynomial terms in
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

Using this regression matrix and and Ordinary Least Squares (OLS) approach described on
[Wikipedia](https://en.wikipedia.org/wiki/Polynomial_regression) in detail, the unknown
coefficients can be determined as follows:

!equation
\textbf{c}=\left(\textbf{R}^T\textbf{R}\right)^{-1}\textbf{R}^T\textbf{y}.

As a last note, it must be mentioned that this method is only applicable if $N_p \leq N$
and keeping $N_p << N$ is recommended.


## Example Input File Syntax

To get the necessary data, two essential blocks have to be included in the master input file.
The first, the sampler defined in `Samplers`, creates the coordinates in matrix $textbf{X}$, while
the objects in `VectorPostprocessors` create, fill and store the result vector $\textbf{y}$.

EXAMPLE HERE Sampler

EXAMPLE HERE VectorPostprocessor

The trainer then uses these arrays together with the defined `max_degree`
to compute the model coefficients `_coeffs` array:

EXAMPLE HERE Trainers

!syntax parameters /Trainers/PolynomialRegressionTrainer

!syntax inputs /Trainers/PolynomialRegressionTrainer

!syntax children /Trainers/PolynomialRegressionTrainer
