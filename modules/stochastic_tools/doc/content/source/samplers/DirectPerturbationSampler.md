# DirectPerturbationSampler

!syntax description /Samplers/DirectPerturbationSampler

## Overview

The direct perturbation method is the simplest approach for
computing local sensitivity coefficients using the finite
difference approximation of the derivatives at a certain
point in the parameter domain. The coordinates of the reference
location can be supplied through the [!param](/Samplers/DirectPerturbationSampler/nominal_parameter_values) parameter.
The parameter vector holding the parameters can be denoted by:
$\vec{\mu}=\left[\mu_1,\mu_2,\mu_3,...,\mu_{N_p}\right]^T$, where
$N_p$ is the number of parameters. The direct perturbation-based
sensitivity coefficients can then be computed using the
following two methods:

- +Central difference approximation:+
  We assume that the sensitivity coefficient for parameter
   $\mu_i$ can be approximated as:

  \begin{equation}
  S_{\mu_i} =C\frac{\Phi(\vec{\mu}_{i+})-\Phi(\vec{\mu}_{i-})}{\mu_{i+}-\mu_{i-}},
  \end{equation}

  where $\vec{\mu}_{i+/-}$ are the vectors with the $i$-th entries
  perturbed in the positive and negative direction (so they
  are replaced by $\mu_{i+}$ and $\mu_{i-}$), respectively.
  Parameter [!param](/Samplers/DirectPerturbationSampler/relative_perturbation_intervals)
  can be used to specify
  the perturbation interval around the nominal values. For the
  central difference approach a 0.1 value would mean that the
  parameter is perturbed by 5% in both directions making the
  total perturbation centered around the nominal value. This
  approach is more accurate, but needs $2N_p+1$ model evaluations.
  We add an additional model evaluation for the reference point.

- +Forward difference approximation:+
  We assume that the sensitivity coefficient for parameter
   $\mu_i$ can be approximated as:

  \begin{equation}
  S_{\mu_i} =C \frac{\Phi(\vec{\mu}_{i+})-\Phi(\vec{\mu})}{\mu_{i+}-\mu},
  \end{equation}

  where $\vec{mu}_{i+}$ is the vector with the $i$-th entry
  perturbed in the positive direction (i.e. it is
  are replaced by $\mu_{i+}$).
  Parameter [!param](/Samplers/DirectPerturbationSampler/relative_perturbation_intervals) can be used to specify
  the perturbation interval from the nominal values. For the
  forward difference approach a 0.1 value would mean that the
  parameter is perturbed by 10% in one direction. This
  approach is less accurate, but only needs $N_p+1$ model
  evaluations.

The multiplication factor $C$ in the expressions above depends on if the
user requested relative sensitivities in the [DirectPerturbationReporter.md].
If an absolute sensitivity was requested $C=1$, for relative sensitivities
$C=\frac{\mu_{ref}}{\Phi_{ref}}$, where $\mu_{ref}$ and $\Phi_{ref}$
are the parameter value and quantity of interest at the reference point.

This sampler is responsible for creating parameter vectors with
perturbed entries. The computation of the sensitivity
coefficients is done using [DirectPerturbationReporter.md].
The differencing approach can be selected using the
[!param](/Samplers/DirectPerturbationSampler/perturbation_method)
parameter.
The implementation is fully parallelized meaning that the model
evaluations and the sensitivity calculation all happen in
parallel.


## Example Input Syntax

the following input generates perturbed samples for a
3-dimensional parameter space around a nominal value
using the central difference approach.

!listing samplers/directperturbation/directperturbation.i block=Samplers

This results in the following parameters:

!listing samplers/directperturbation/gold/directperturbation_out_data_0001.csv

With forward differencing the sample output is the following:

!listing samplers/directperturbation/gold/dp_fd_data_0001.csv

!syntax parameters /Samplers/DirectPerturbationSampler

!syntax inputs /Samplers/DirectPerturbationSampler

!syntax children /Samplers/DirectPerturbationSampler
