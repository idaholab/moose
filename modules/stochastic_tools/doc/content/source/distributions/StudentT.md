# Student t-distribution

!syntax description /Distributions/StudentT

## Overview

This object defines a [student t-distribution](https://en.wikipedia.org/wiki/Student%27s_t-distribution) with one degree of freedom parameter [!param](/Distributions/StudentT/dof) ($\nu$). The probability density function and cumulative distribution function are defined in [eq:t_pdf] and [eq:t_cdf], respectively.

!equation id=eq:t_pdf
f(x; \nu) = \frac{1}{\sqrt{\nu}\Beta\left(\frac{1}{2},\frac{\nu}{2}\right)} \left(1+\frac{x^2}{\nu}\right)^{-\frac{\nu+1}{2}},

!equation id=eq:t_cdf
F(x; \nu) = 1 - \frac{1}{2}I_{z(x)}\left(\frac{\nu}{2},\frac{1}{2}\right), \quad z(x) = \frac{\nu}{x^2+\nu},

where $\nu \in \mathbb{Z} > 0$. $\Beta(a,b)$ is the beta function and $I_z(a,b)$ is the regularized incomplete beta function, see [Beta.md] for more details.

!syntax parameters /Distributions/StudentT

!syntax inputs /Distributions/StudentT

!syntax children /Distributions/StudentT
