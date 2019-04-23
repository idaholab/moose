# DiscreteNucleationTimeStep

!syntax description /Postprocessors/DiscreteNucleationTimeStep

Supply this postprocessor to an [IterationAdaptiveDT](/IterationAdaptiveDT.md)
via the `timestep_limiting_postprocessor` parameter.

The timestep limit computed by this postprocessor is computed according to two
different criteria.

## Time step limit at nucleus insertion

If a nucleus has just been added to the nucleus list by the
[DiscreteNucleationInserter](/DiscreteNucleationInserter.md) the timestep limit
is set to the value supplied using the `dt_max` parameter for one timestep. In
conjunction with IterationAdaptiveDT this causes the time step to be cut to
`dt_max` from which it will slowly have to grow back.

## Nucleation rate based timestep limit

Between nucleation event onsets the timestep is limited based on the user
supplied upper bound on the probability $p_{2nuc}$  (`p2nucleus`) to have
_more than two_ nueceation events to occur during a single timestep.
This probability is calculated as

\begin{equation}
\label{eq-p2nuc}
p_{2nuc} = 1-(1+\lambda_{2nuc})e^{-\lambda_{2nuc}},
\end{equation}

where $\lambda_{2nuc}$ is the total nucleation rate over the whole simulation
cell that results in the probability $p_{2nuc}$. To obtain $\lambda_{2nuc}$ for
a given $p_{2nuc}$ equation [eq-p2nuc] is numerically inverted. $\lambda_{2nuc}$
is then divided by the integrated nucleation rate per unit time to obtain the
largest possible time step that keeps the probability for two or more nuclei to
form below the user specified upper bound.

!media /nucleation_timestep.png style=width:80%; margin-left:20px;
       caption=Timestep (dt) in a nucleation simulation with a DiscreteNucleationTimeStep
       limited time step. The green curve (dtnuc) shows the time step limit.
       The envelope of that curve is determined by the upper bound on the two or more
       nucleus probability. The sharp downward spikes are the time step cut-backs
       during nucleation events.

This addresses two issues with poisson statistics sampling. At every sampling
point in the domain the rate is sufficiently low to stay in the rare event
regime (i.e where either zero or one event are happening). At higher rates the
time resolution is insufficient to capture all possible nucleation events.
Controlling the probability of multiple nuclei forming also reduces the chance
of overlapping nuclei to form.

The `DiscreteNucleationTimeStep` postprocessor is part of the
[Discrete Nucleation system](Nucleation/DiscreteNucleation.md).

!syntax parameters /Postprocessors/DiscreteNucleationTimeStep

!syntax inputs /Postprocessors/DiscreteNucleationTimeStep

!syntax children /Postprocessors/DiscreteNucleationTimeStep

!bibtex bibliography
