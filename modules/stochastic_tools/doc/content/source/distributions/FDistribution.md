# F-Distribution

!syntax description /Distributions/FDistribution

## Overview

This object defines a [F-distribution](https://en.wikipedia.org/wiki/F-distribution) with two degree of freedom parameters: [!param](/Distributions/FDistribution/df1) ($d_1$) and [!param](/Distributions/FDistribution/df2) ($d_2$). The probability density function and cumulative distribution function are defined in [eq:f_pdf] and [eq:f_cdf], respectively.

!equation id=eq:f_pdf
f(x; d_1, d_2) = \frac{1}{\Beta\left(\frac{d_1}{2},\frac{d_2}{2}\right)} \left(\frac{d_1}{d_2}\right)^{\frac{d_1}{2}} x^{\frac{d_1}{2}-1} \left(1+\frac{d_1}{d_2}x\right)^{-\frac{d_1-d_2}{2}},

!equation id=eq:f_cdf
F(x; d_1, d_2) = I_{z(x)}\left(\frac{d_1}{2},\frac{d_2}{2}\right), \quad z(x) = \frac{d_1x}{d_1x+d_2},

where $d_1,d_2 \in \mathbb{Z} > 0$ and $x > 0$. $\Beta(a,b)$ is the beta function and $I_z(a,b)$ is the regularized incomplete beta function, see [Beta.md] for more details.

!syntax parameters /Distributions/FDistribution

!syntax inputs /Distributions/FDistribution

!syntax children /Distributions/FDistribution
