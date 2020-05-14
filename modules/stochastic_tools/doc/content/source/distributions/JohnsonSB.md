# JohnsonSB

!syntax description /Distributions/JohnsonSB

## Description

The Johnson Special Bounded (SB) distribution [!cite](johnson1994continuous)
is related to the normal distribution. Four parameters
are needed: $\gamma$, $\delta$, $\lambda$, and $\epsilon$. It is a continuous distribution defined on
bounded range $\epsilon \leq x \leq \epsilon + \lambda$, and the distribution can be symmetric or
asymmetric.

Probability Density Function:

!equation
f(x) = \tfrac{\delta}{\lambda\sqrt{2\pi} z(1-z)} exp(-\tfrac{1}{2}(\gamma + \delta ln(\tfrac{z}{1-z}))^2),\,\textrm{where}\, z \equiv \tfrac{x-\zeta}{\lambda}

Cumulative Density Function:

!equation
F(x) = \Phi(\gamma + \delta ln \tfrac{z}{1-z}),\,\textrm{where}\, z = \tfrac{x-\epsilon}{\lambda}

!syntax parameters /Distributions/JohnsonSB

!syntax inputs /Distributions/JohnsonSB

!syntax children /Distributions/JohnsonSB

!bibtex bibliography
