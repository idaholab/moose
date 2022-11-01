# Beta Distribution

!syntax description /Distributions/Beta

## Overview

This object defines a [beta distribution](https://en.wikipedia.org/wiki/Beta_distribution) with two shape parameters: [!param](/Distributions/Beta/alpha) ($\alpha$) and [!param](/Distributions/Beta/beta) ($\beta$). The probability density function, cumulative distribution function, and quantile are defined in [eq:beta_pdf], [eq:beta_cdf], and [eq:beta_q], respectively.

!equation id=eq:beta_pdf
f(x;\alpha,\beta) = \frac{x^{\alpha-1}(1-x)^{\beta-1}}{\Beta(\alpha,\beta)},

!equation id=eq:beta_cdf
F(x;\alpha,\beta) = I_x(\alpha,\beta),

!equation id=eq:beta_q
F^{-1}(p; \alpha,\beta) = I_x^{-1}(\alpha, \beta, p),

where $\alpha,\beta > 0$ and $0\leq x,p \leq 1$. $\Beta(a,b)$ is the beta function defined in [eq:beta_fun], $I_x(a,b)$ is the regularized incomplete beta function defined in [eq:ibeta], and $I_x^{-1}(a, b, p)$ is the inverse of the incomplete beta function. Each of these quantities are computed using a custom iterative procedure [!citep](teukolsky1992numerical), which obtains an accuracy of about $10^{-14}$.

!equation id=eq:beta_fun
\Beta(a,b) = \frac{\Gamma(a)\Gamma(b)}{\Gamma(a+b)} = \int_0^1 x^{a-1}(1-x)^{b-1}dx

!equation id=eq:ibeta
I_x(a,b) = \frac{1}{\Beta(a,b)}\int_0^x t^{a-1}(1-t)^{b-1}dt

!syntax parameters /Distributions/Beta

!syntax inputs /Distributions/Beta

!syntax children /Distributions/Beta
