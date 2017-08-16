# WeibullDistribution

!!!warning
    The WeibullDistribution object requires that libMesh be configured to utilize an external
    [Boost](www.boost.org) library. This may be done using the `--with-boost` configuration option
    when compiling libMesh.

## Description
As the name suggests, the WeibullDistribution object defines a Weibull distribution which
has a probability density function (PDF) defined as

\begin{equation}
f(x;\lambda,k) =
\begin{cases}
\frac{k}{\lambda}\left(\frac{x}{\lambda}\right)^{k-1}e^{-(x/\lambda)^{k}} & x\geq0 ,\\
0 & x<0,
\end{cases}
\end{equation}
where $k > 0$ and defines the shape parameter and $\lambda > 0$ and defines the scale parameter.

The cumulative distribution function (CDF) is defined as
\begin{equation}
F(x;k,\lambda) =
\begin{cases}
1- e^{-(x/\lambda)^k}\ & x\geq0, \\
0 & x<0.
\end{cases}
\end{equation}

The quantile function is defined as
\begin{equation}
Q(p;k,\lambda) = \lambda {(-\ln(1-p))}^{1/k} \quad 0 \leq p < 1.
\end{equation}

## Example Input Syntax

!listing /modules/stochastic_tools/test/tests/distributions/weibull.i block=Distributions

!syntax parameters /Distributions/WeibullDistribution

!syntax input /Distributions/WeibullDistribution

!syntax children /Distributions/WeibullDistribution
