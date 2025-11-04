# DirectPerturbationReporter

!syntax description /Reporters/DirectPerturbationReporter

## Overview

This reporter is designed to compute direct
perturbation-based sensitivity
coefficients using samples obtained by [DirectPerturbationSampler.md].
The available methods and settings are discussed in
[DirectPerturbationSampler.md] in detail.

## Example Input Syntax

The following example computes the local sensitivity coefficients of three linear functions that can be described as:

\begin{equation}
  f_1(\mu_1,\mu_2,\mu_2)=\mu_1+2\mu_2+3\mu_3
\end{equation}

\begin{equation}
  f_2(\mu_1,\mu_2,\mu_2)=4\mu_1+5\mu_2+6\mu_3
\end{equation}

\begin{equation}
  f_3(\mu_1,\mu_2,\mu_2)=7\mu_1+8\mu_2+9\mu_3
\end{equation}

!listing reporters/directperturbation/dp_main.i block=Samplers

!listing reporters/directperturbation/dp_main.i block=Reporters

The resulting output is a [json file](JSONOutput.md) with the sensitivity coefficients:

!listing reporters/directperturbation/gold/dp_main_out.json language=json

We see that the method recovers the expansion coefficients of
the $f_i$ functions exactly.

!syntax parameters /Reporters/DirectPerturbationReporter

!syntax inputs /Reporters/DirectPerturbationReporter

!syntax children /Reporters/DirectPerturbationReporter
