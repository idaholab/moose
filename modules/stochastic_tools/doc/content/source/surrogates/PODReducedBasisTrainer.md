# PODReducedBasisTrainer

!syntax description /Trainers/PODReducedBasisTrainer

## Overview

In this trainer, an intrusive Proper Orthogonal Decomposition (POD) based Reduced Basis (RB) method
is implemented which, unlike non-intrusive surrogates such as Polynomial Regression Surrogate or
Polynomial Chaos Expansion Surrogate, is capable of considering the physics of the full-order problem
at surrogate level. Therefore, it is often referred to as a physics-based but still data-driven approach.
The intrusiveness, however, decreases the range of problems which this method can be used for.
In the current version, this surrogate model can deal with
+Parameterized scalar-valued linear steady-state PDEs with affine parameter dependence+ only.
This class is responsible for two steps in the generation of the surrogate model:

1. [Generation of reduced subspaces](#generation-of-reduced-subspaces)
2. [Generation of reduced operators](#generation-of-reduced-operators)

## A parameterized linear steady-state PDE

Before the details of the above-mentioned steps are discussed, the basic notation
has to be established. A +scalar-valued linear steady-state PDE+ can be expressed in
operator notation as:

!equation id=fom_operators
\mathcal{A}u = \mathcal{b},

where $u$ is the unknown solution vector, $\mathcal{A}$ is a linear operator and
$\mathcal{b}$ is a source term. The linear operator and the source terms can
depend on uncertain parameters which are denoted by $\mu_i,~i=0,...,N_\mu$ and
organized into a parameter vector $\boldsymbol{\mu} = [\mu_1,...,\mu_{N_\mu}]^T$.
Therefore, [fom_operators] can be expressed as:

!equation id=fom_parameterized
\mathcal{A}(\boldsymbol{\mu})u = \mathcal{b}(\boldsymbol{\mu}).

This also means that the solution itself is the function of these parameters
$u=u(\boldsymbol{\mu})$ as well. To make an efficient surrogate, operator
$\mathcal{A}(\boldsymbol{\mu})$ and source $\mathcal{b}(\boldsymbol{\mu})$ should
have an +affine parameter dependence+:

!equation
\mathcal{A}(\boldsymbol{\mu})
= \sum \limits_{i=1}^{N_A} f^A_i(\boldsymbol{\mu})\mathcal{A}_i,
\text{\quad and \quad} \mathcal{b}(\boldsymbol{\mu})
= \sum \limits_{i=1}^{N_b} f^b_i(\boldsymbol{\mu})\mathcal{b}_i.

It is visible that the affine decomposition is the sum of the products of parameter-dependent
scalar functions and parameter-independent constituent operators. Therefore, the problem
can be written as

!equation id=fom_affine_decomp
\left(\sum \limits_{i=1}^{N_A} f^A_i(\boldsymbol{\mu})\mathcal{A}_i\right)u =
\sum \limits_{i=1}^{N_b} f^b_i(\boldsymbol{\mu})\mathcal{b}_i.

This decomposition cannot be defined automatically, the user has to identify the constituent
scalar functions and operators before starting to construct a POD-RB surrogate model.
As a last step, this system is discretized in space using the finite element method to obtain:

!equation id=fom_affine_decomp_discrete
\left(\sum \limits_{i=1}^{N_A} f^A_i(\boldsymbol{\mu})\textbf{A}_i\right)\textbf{u} =
\sum \limits_{i=1}^{N_b} f^b_i(\boldsymbol{\mu})\textbf{b}_i,

where $\textbf{A}_i$  and $\textbf{b}_i$ are finite element matrices and vectors,
while $\textbf{u}$ denotes the vector containing the values of the degrees of freedom.
Furthermore, let $\textbf{u}(\boldsymbol{\mu}^* )$ denote a solution vector which is
obtained by solving [fom_affine_decomp_discrete] with $\boldsymbol{\mu}=\boldsymbol{\mu}^* ~$.

## Generation of reduced subspaces

Even though it is not explicitly stated in the previous sub-section, $\textbf{u}$ may
contain solutions for multiple variables, hence it can be expressed as
$\textbf{u}=[\textbf{u}_1;...;~\textbf{u}_{N_v}]$, where $N_v$ is the total number of variables.
It is assumed that each variable has $N$ spatial degrees of freedom, thus the full size of the
system is $N\times N$.

As a first step in this process, [fom_affine_decomp_discrete] is solved using $N_s$
different parameter samples and the solution vectors for each variable are saved into
snapshot matrices

!equation id=fom_parameterized
\textbf{S}_i = [\textbf{u}_i(\boldsymbol{\mu}_1),...,\textbf{u}_i(\boldsymbol{\mu}_{N_s})].

In this implementation, the solutions are obtained from a [PODFullSolveMultiApp.md] using a
[SamplerSolutionTransfer.md], however the snapshot matrices are stored within the trainer object.
The next step in this process is to use these snapshots to create reduced sub-spaces for
each variable. This can be done by performing POD on the snapshot matrices for each variable.
POD consists of the following four steps for each variable:

1. +Creation of correlation matrices:+ The correlation matrices ($\textbf{C}_i$) can be computed
   using the snapshot matrices as $\textbf{C}_i=\textbf{S}_i^T \textbf{W}_i \textbf{S}_i$,
   where $\textbf{W}_i$ is a weighting matrix. At this moment only $\textbf{W}_i = \textbf{I}$ is
   supported.
2. +Eigenvalue decomposition of the correlation matrices:+ The eigenvalue decompositions of the
   correlation matrices is obtained as: $\textbf{C}_i = \textbf{V}_i\boldsymbol{\Lambda}_i\textbf{V}^T_i$,
   where matrix $\textbf{V}_i$ contains the eigenvectors and matrix $\boldsymbol{\Lambda}_i$
   contains the eigenvalues of $\textbf{C}_i$.
3. +Determining the dimension of the reduced subspace:+ Based on the magnitude of the
   eigenvalues ($\lambda_{i,k},~k=1,...,N_s$) in $\boldsymbol{\Lambda}_i$, one can compute how many basis functions
   are needed to reconstruct the snapshots with a given accuracy. The rank of the subspace
   $r_i$ can be determined as:

   !equation id=ev_determination
   r_i = \argmin\limits_{1\leq r_i\leq N_s}
   \left(\frac{\sum_{k=1}^{r_i}\lambda_{i,k}}{\sum_{k=1}^{N_s}\lambda_{i,k}}\right).

   Of course this rank can be determined manually as well.
4. +The reconstruction of the basis vectors for each variable:+ For this, the eigenvalues and
   eigenvectors of the correlation matrices are used together with the snapshots as:

   !equation id=base_definition
   \Phi_{i,k}=\frac{1}{\sqrt{\lambda_{i,k}}}\sum\limits_{j=1}^{N_s}\textbf{V}_{i,k,j}S_{i,j},

   where \Phi_{i,k} is the $k$-th ($k=1,...,r_i$) basis function of the reduced subspace for
   variable $\textbf{u}_i$. Moreover, $\textbf{V}_{i,k,j}$ denottes the $j$-th element of
   the $k$-th eigenvector of the correlation matrix. It is important to remember that
   $\Phi_{i,k}$ has a global support, and shall not be mistaken for the local
   basis functions ($\phi$) of the finite element approximation.


## Generation of reduced operators

!syntax parameters /Trainers/PODReducedBasisTrainer

!syntax inputs /Trainers/PODReducedBasisTrainer

!syntax children /Trainers/PODReducedBasisTrainer
