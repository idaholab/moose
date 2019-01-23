# LogisticDistribution

## Description

The logistic distribution is a continuous distribution defined with a location ($\alpha$) and
shape ($\beta$) parameters.

Probability Density Function:

\begin{equation}
z(x) = \exp{-\frac{(x - \alpha)}{\beta}}
f(x) = \frac{z(x)}{(\alpha * (1.0 + z)^2)}
\end{equation}

Cumulative Density Function:

\begin{equation}
F(x) = \frac{1}{1+z(x)}
\end{equation}


!syntax description /Distributions/LogisticDistribution

!syntax parameters /Distributions/LogisticDistribution

!syntax inputs /Distributions/LogisticDistribution

!syntax children /Distributions/LogisticDistribution

!bibtex bibliography
