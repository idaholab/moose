# Morris

!syntax description /Samplers/MorrisSampler

## Overview

The Morris screening method is a one-at-a-time (OAT) pertabative sampling method for performing global sensitivity analaysis [!citep](morris1991factorial, saltelli2008global). The implementation is equivalent to the trajectory design described in the [GSA Module](https://gsa-module.readthedocs.io/en/stable/implementation/morris_screening_method.html) and implemented in [SALib](https://github.com/SALib/SALib). The stochastic tools module implementation is fully parallelized and is meant to be used with the [MorrisReporter.md] to calculate the sensitivity quantities.

## Trajectory Design

Although multiple sampling strategies exist for Morris screening, this object utilizes the trajectory design. This strategy performs set perturbations in random directions for individual parameters one-at-a-time. A trajectory performs the perturbation for each parameter randomly starting from a random starting point. The number of parameters ($D$) is determined by the number of entries in [!param](/Samplers/MorrisSampler/distributions). The number of trajectories is specified by [!param](/Samplers/MorrisSampler/trajectories). The discretization of the starting point and the perturbation $\Delta$ is determined by [!param](/Samplers/MorrisSampler/levels):

!equation
\Delta = \frac{1}{2}\,\frac{\texttt{levels}}{\texttt{levels} - 1} .

The sampling matrix for a single trajectory ($b^{*}$) is given by the following expression:

!equation
b^{*} = x^{*} + \frac{\Delta}{2}\left(\left(2bp^{*}-j\right)d^{*}+ j\right) ,

where,

- $b \equiv$ ($D+1$)-by-$D$ strictly lower triangular matrix of 1s,
- $x^{*} \equiv$ ($D+1$)-by-$D$ matrix where each row is the same random starting point $[0, 1]^D$,
- $d^{*} \equiv$ $D$-by-$D$ diagonal matrix with either a $1$ or $-1$ for each row, this determines the direction of the perturbations,
- $p^{*} \equiv$ $D$-by-$D$ maxtrix where each row has a different column equal to 1 with no repeated columns, this determines which parameter is perturbed, and
- $j \equiv$ ($D+1$)-by-$D$ dense matrix of 1s.

Each row of the resulting $b^{*}$ represents OAT perturbed sample between 0 and 1; each entry of the rows is put through the corresponding [!param](/Samplers/MorrisSampler/distributions)'s quantile to produce a $D+1$ samples. A seperate $b^{*}$ is produced for the number specified by [!param](/Samplers/MorrisSampler/trajectories).

## Example Input Syntax

The following input generates a 3-dimensional sampling matrix with 4 trajecteries, which creates $(D+1)\times N = 16$ samples.

!listing samplers/morris/morris.i block=Samplers

!listing samplers/morris/gold/morris_out_data_0000.csv

!plot scatter caption=Visualization of trajectory design Morris sampling (2D projection)
    data=[{'x':[0.4,0.4,0.4,1], 'y':[0,0.6,0.6,0.6], 'name':'Trajectory 1'},
          {'x':[0.4,1,1,1], 'y':[0.2,0.2,0.2,0.8], 'name':'Trajectory 2'},
          {'x':[0.6,0.6,0,0], 'y':[0.2,0.2,0.2,0.8], 'name':'Trajectory 3'},
          {'x':[0.2,0.8,0.8,0.8], 'y':[1,1,0.4,0.4], 'name':'Trajectory 4'}]

!syntax parameters /Samplers/MorrisSampler

!syntax inputs /Samplers/MorrisSampler

!syntax children /Samplers/MorrisSampler

!bibtex bibliography
