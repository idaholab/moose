# HDG Navier-Stokes

## Interior Penalty

Interior penalty HDG Navier-Stokes is favored for advection-dominated flows due to its
pressure robustness. The finite element fields include interior and facet velocities
and interior and facet pressures. We have built a preconditioning strategy for IP-HDG
Navier-Stokes that is extremely robust in approximating the Schur complement with respect
to mesh size and Reynolds number. Its detailed description can be found in the
[arXiv paper](https://arxiv.org/abs/2512.02971). The preconditioning strategy leverages
an augmented Lagrange like addition to the weak form, which allows the accurate approximation
of the Schur complement. However, as also discussed in [NavierStokesProblem.md], the addition
of an augmented Lagrange term introduces a large symmetric singular perturbation in the momentum block,
transferring solver difficulty from the Schur complement to the momentum block. As discussed in
the arXiv paper, our current strategy is to use an inexact LU decomposition with butterfly
compression for the (trace) momentum block. This is effective until reaching sufficiently large
problem size and Reynolds number at which point the inexactness of the decomposition due to compression
loss is unable to resolve the poor conditioning resulting from the combination of the large singular
perturbation and advection dominance. We hypothesize that introduction of turbulence models, whether
via RANS or subgrid scale models in LES, would add sufficient viscosity to restore the effectiveness
of the inexact LU. However, this is an open question that would have to be addressed by further work.

## Hybridizable Local Discontinuous Galerkin

The local discontinuous Galerkin method introduces additional finite element fields corresponding
to the interior and facet velocity gradients. This method is a strong
choice for diffusion-dominated flows because the velocity gradient
converges with optimal order whereas for other methods, the velocity
gradient convergence is suboptimal. Additionally, with postprocessing
the velocity converges with an additional order, e.g. it is superconvergent. Additional
information may be found at the [core HDG kernel page](NavierStokesLHDGKernel.md).
