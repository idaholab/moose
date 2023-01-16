!template load file=sqa/module_srs.md.template category=stochastic_tools module=Stochastic Tools

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the MOOSE Stochastic Tools module includes, but is not limited
to:

- Providing a MOOSE-like interface for performing stochastic analysis on
  MOOSE-based models.
- Sampling model parameters, running applications, and gathering data of
  interest that is both efficient (memory and runtime) and scalable.
- Performing uncertainty quantification and sensitivity analysis with
  distributed data.
- Training meta-models to develop fast-evaluation surrogates of the
  high-fidelity multiphysics model.
- Providing a pluggable interface for these surrogates.

As such, the purpose of this module is not to provide physical model
capabilities, which is typically the responsibility of other MOOSE modules and
dependent applications, but to provide data creation and processing capabilities
in stochastic analysis.
!! system-purpose-finish
!template-end!

!template! item key=system-scope
!! system-scope-begin
The MOOSE Stochastic Tools module builds on the existing framework MultiApps
system and provides several additional systems to address its purpose. The
address the efficient evaluation of multiphysics models, the MultiApps systems
is extended to include a "batch" mode of execution. This mode provides a memory
efficient way of building MooseApp instances and reusing them for different
perturbations of input parameters. This is useful for stochastic simulations
with many, many samples to be run without holding them in memory or
re-initializing potentially costly data allocation. The additional systems that
the Stochastic Tools module provides include: Distributions, Samplers, Trainers,
and Surrogates. Distributions are functions defining the uncertainty of input
parameters and provides and interface for computing probability density,
cumulative probability, and quantiles. Samplers define the sampling scheme of
the stochastic analysis, whether it be random or deterministic. Trainers are
objects that build meta-models meant to be used as surrogates or reduced-order
models of the multiphysics model. The resulting reduced model is able to be
saved in a meta-data file and reloaded for future use. Surrogates take the data
created from a Trainer and provide functionality to evaluate the model. The
module also provides capabilities for computing quantities related to basic
uncertainty quantification and sensitivity analysis using the framework's
Reporters system.
!! system-scope-finish
!template-end!
