# Gamma Distribution

!syntax description /Distributions/Gamma

## Overview

This object defines a [gamma distribution](https://en.wikipedia.org/wiki/Gamma_distribution) with [!param](/Distributions/Gamma/shape) parameter ($k$ or $\alpha$) and [!param](/Distributions/Gamma/scale) parameter ($\theta$ or $1/\beta$). The probability density function, cumulative distribution function, and quantile are defined in [eq:gamma_pdf], [eq:gamma_cdf], and [eq:gamma_q], respectively.

!equation id=eq:gamma_pdf
f(x; \alpha, \beta) = \frac{\beta^\alpha x^{\alpha-1} e^{-\beta x}}{\Gamma(\alpha)},

!equation id=eq:gamma_cdf
F(x; \alpha, \beta) = \Gamma(\alpha, \beta x),

!equation id=eq:gamma_q
F^{-1}(p; \alpha, \beta) = \frac{\Gamma^{-1}(\alpha, p)}{\beta},

where $x,\alpha,\beta > 0$ and $0\leq p \leq 1$. $\Gamma(a)$ is the gamma function defined by [eq:gamma_fun], $\Gamma(a, x)$ is the lower incomplete gamma function defined by [eq:igamma], and $\Gamma^{-1}(a, p)$ is the inverse of the incomplete gamma function. $\Gamma$ and $\Gamma^{-1}$ are computed using a custom iterative procedure, which obtains an accuracy of about $10^{-14}$.

!equation id=eq:gamma_fun
\Gamma(a) = \int_0^\infty x^{a-1}e^{-x}dx

!equation id=eq:igamma
\Gamma(a, x) = \frac{1}{\Gamma(a)}\int_0^x t^{a-1}e^{-t}dt

!syntax parameters /Distributions/Gamma

!syntax inputs /Distributions/Gamma

!syntax children /Distributions/Gamma
