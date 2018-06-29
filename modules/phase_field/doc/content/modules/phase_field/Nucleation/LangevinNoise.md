# Langevin Noise

The MOOSE phase field module supports adding consistent random noise terms $\zeta$ to
any PDE.

\begin{equation}
\frac{\partial c}{\partial t} = \dots + \zeta(t,\vec r)
\end{equation}

The challenge in finite element solves, which recompute the resildual and Jacobian
terms multiple times per timestep, is to have the same noise field throughout a
given timestep. Ideally the noise field should also be independent of the parallelization
of the simulation.

The legacy [LangevinNoise](kernels/LangevinNoise.md) kernel supplies a stable noise field for
a given timestep by initializing the random number stream with a seed that is a function
of the timestep number at the beginning of every residual calculation. The Jacobian is
independent of the noise field.

!include /Nucleation/conservednoise_include.md
