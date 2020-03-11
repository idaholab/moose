# LogisticDistribution

## Description

The logistic distribution is a continuous distribution defined with a location ($\alpha$) and
shape ($\beta$) parameters.

Probability Density Function:

!equation
z(x) = \exp{-\frac{(x - \alpha)}{\beta}}
f(x) = \frac{z(x)}{(\alpha * (1.0 + z)^2)}

Cumulative Density Function:

!equation
F(x) = \frac{1}{1+z(x)}

!syntax description /Distributions/LogisticDistribution

!syntax parameters /Distributions/LogisticDistribution

!syntax inputs /Distributions/LogisticDistribution

!syntax children /Distributions/LogisticDistribution

!bibtex bibliography
