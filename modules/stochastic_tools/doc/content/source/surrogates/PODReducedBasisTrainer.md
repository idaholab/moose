# PODReducedBasisTrainer

!syntax description /Trainers/PODReducedBasisTrainer

## Overview

In this trainer, an intrusive Proper Orthogonal Decomposition (POD) based Reduced Basis (RB) method
([!cite](pinnau2008model)) is implemented which, unlike non-intrusive surrogates such as Polynomial Regression or
Polynomial Chaos Expansion, is capable of considering the physics of the full-order problem
at surrogate level. Therefore, it is often referred to as a physics-based but still data-driven approach.
The intrusiveness, however, decreases the range of problems which this method can be used for.
In the current version, this surrogate model can deal with
+Parameterized scalar-valued linear steady-state PDEs with affine parameter dependence+ only.
This class is responsible for two steps in the generation of the surrogate model:

1. [Generation of reduced subspaces](#generation-of-reduced-subspaces)
2. [Generation of reduced operators](#generation-of-reduced-operators)

It must be mentioned that in POD-RB literature, the training phase is often referred to
as offline phase and in the upcoming sections the two expressions are used interchangeably.

## A parameterized linear steady-state PDE

Before the details of the above-mentioned steps are discussed, a short overview is
given about the problems considered. A +scalar-valued linear steady-state PDE+ can be expressed in
operator notation as:

!equation id=fom_operators
\mathcal{A}u = \mathcal{b},

where $u$ is the solution, $\mathcal{A}$ is a linear operator and
$\mathcal{b}$ is a source term. The linear operator and the source terms may
depend on uncertain parameters which are denoted by $\mu_i,~i=0,...,N_\mu$ and
organized into a parameter vector $\boldsymbol{\mu} = [\mu_1,...,\mu_{N_\mu}]^T$.
Therefore, [fom_operators] can be expressed as:

!equation id=fom_parameterized
\mathcal{A}(\boldsymbol{\mu})u = \mathcal{b}(\boldsymbol{\mu}).

This also means that the solution itself is the function of these parameters
$u=u(\boldsymbol{\mu})$. To make an efficient surrogate, operator
$\mathcal{A}(\boldsymbol{\mu})$ and source $\mathcal{b}(\boldsymbol{\mu})$ should
have an +affine parameter dependence+:

!equation
\mathcal{A}(\boldsymbol{\mu})
= \sum \limits_{i=1}^{N_A} f^A_i(\boldsymbol{\mu})\mathcal{A}_i,
\text{\quad and \quad} \mathcal{b}(\boldsymbol{\mu})
= \sum \limits_{i=1}^{N_b} f^b_i(\boldsymbol{\mu})\mathcal{b}_i,

or in other words, the operators have to be decomposable as the sums
of products of parameter-dependent scalar functions and parameter-independent
constituent operators. By plugging the decompositions back to [fom_parameterized],
the problem takes the following form:

!equation id=fom_affine_decomp
\left(\sum \limits_{i=1}^{N_A} f^A_i(\boldsymbol{\mu})\mathcal{A}_i\right)u =
\sum \limits_{i=1}^{N_b} f^b_i(\boldsymbol{\mu})\mathcal{b}_i.

Therefore, before starting the construction of a POD-RB surrogate model,
the user must identify the these decompositions first. At input level, this can
be done by utilizing the `Tagging System`. For each constituent operator a separate
vector tag has to be created and the tags need to be supplied to the trainer object
through the [!param](/Trainers/PODReducedBasisTrainer/tag_names) input parameter. Furthermore, an indicator shall be added
to each tag through the [!param](/Trainers/PODReducedBasisTrainer/tag_types) input to show if the tag corresponds to a
source term ($\mathcal{b}_i$) or an operator ($\mathcal{A}_i$).
As a last step, this system is discretized in space using the finite element method to obtain:

!equation id=fom_affine_decomp_discrete
\left(\sum \limits_{i=1}^{N_A} f^A_i(\boldsymbol{\mu})\textbf{A}_i\right)\textbf{u} =
\sum \limits_{i=1}^{N_b} f^b_i(\boldsymbol{\mu})\textbf{b}_i,

where $\textbf{A}_i$  and $\textbf{b}_i$ are finite element matrices and vectors,
while $\textbf{u}$ denotes the vector containing the values of the degrees of freedom.
This model is referred to as Full-Order Model (FOM) in subsequent sections.
Furthermore, let $\textbf{u}(\boldsymbol{\mu}^* )$ denote a solution vector which is
obtained by solving [fom_affine_decomp_discrete] with $\boldsymbol{\mu}=\boldsymbol{\mu}^* ~$.

## Generation of reduced subspaces

Even though it is not explicitly stated in the previous sub-section, $\textbf{u}$ may
contain solutions for multiple variables, hence it can be expressed as
$\textbf{u}=[\textbf{u}_1;...;~\textbf{u}_{N_v}]$, where $N_v$ is the total number of variables.
It is assumed that each variable has $N_i,~i=1,...,N_v$ spatial degrees of freedom,
thus the size of the full-order system is $\sum\limits_{i=1}^{N_v}N_i$.

As a first step in this process, [fom_affine_decomp_discrete] is solved using $N_s$
different parameter samples and the solution vectors for each variable defined on the  
[!param](/Trainers/PODReducedBasisTrainer/var_names) input parameter are saved into
snapshot matrices

!equation id=s-matrices
\textbf{S}_i = [\textbf{u}_i(\boldsymbol{\mu}_1),...,\textbf{u}_i(\boldsymbol{\mu}_{N_s})].

In this implementation, the solutions are obtained from a [PODFullSolveMultiApp.md] using a
[PODSamplerSolutionTransfer.md] and the snapshot matrices are stored within the trainer object.
The next step in this process is to use these snapshots to create reduced sub-spaces for
each variable. This can be done by performing POD on the snapshot matrices,
which consists of the following four steps for each variable:

1. +Creation of correlation matrices:+ The correlation matrices ($\textbf{C}_i$) can be computed
   using the snapshot matrices as $\textbf{C}_i=\textbf{S}_i^T \textbf{W}_i \textbf{S}_i$,
   where $\textbf{W}_i$ is a weighting matrix. At this moment only $\textbf{W}_i = \textbf{I}$ is
   supported, where $\textbf{I}$ is the identity matrix.

2. +Eigenvalue decomposition of the correlation matrices:+ The eigenvalue decompositions of the
   correlation matrices is obtained as: $\textbf{C}_i = \textbf{V}_i\boldsymbol{\Lambda}_i\textbf{V}^T_i$,
   where matrix $\textbf{V}_i$ and matrix $\boldsymbol{\Lambda}_i$
   contain the eigenvectors and eigenvalues of $\textbf{C}_i$, respectively.

3. +Determining the dimension of the reduced subspace:+ Based on the magnitude of the
   eigenvalues ($\lambda_{i,k},~k=1,...,N_s$) in $\boldsymbol{\Lambda}_i$, one can compute how many basis functions
   are needed to reconstruct the snapshots with a given accuracy. The rank of the subspace
   ($r_i$) can be determined as:

   !equation id=ev_determination
   r_i = \argmin\limits_{1\leq r_i\leq N_s}
   \left(\frac{\sum_{k=1}^{r_i}\lambda_{i,k}}{\sum_{k=1}^{N_s}\lambda_{i,k}} > 1 - \tau_i\right),

   where $\tau$ is a given parameter describing the allowed error in the reconstruction of the snapshots.
   This can be supplied to the trainer using the [!param](/Trainers/PODReducedBasisTrainer/error_res) input parameter.
   Of course, this rank can be determined manually as well. For more information about this option, see
   [PODReducedBasisSurrogate.md].

4. +The reconstruction of the basis vectors for each variable:+ For this, the eigenvalues and
   eigenvectors of the correlation matrices are used together with the snapshots as:

   !equation id=base_definition
   \Phi_{i,k}=\frac{1}{\sqrt{\lambda_{i,k}}}\sum\limits_{j=1}^{N_s}\textbf{V}_{i,k,j}S_{i,j},

   where $\Phi_{i,k}$ is the $k$-th ($k=1,...,r_i$) basis function of the reduced subspace for
   variable $\textbf{u}_i$. Moreover, $\textbf{V}_{i,k,j}$ denottes the $j$-th element of
   the $k$-th eigenvector of correlation matrix $\textbf{C}_i$. It is important to remember that
   $\Phi_{i,k}$ has a global support in space, and shall not be mistaken for the local
   basis functions ($\phi$) of the finite element approximation. The global basis vectors can be
   also referred to as POD modes and the two expressions are used interchangeably from here on.

It must be noted that these basis functions are also stored in the trainer object.
Finally, the solutions of different variables in the full-order model can be approximated as the
parameter-dependent linear combination of these basis functions:

!equation id=sol_approximation
\textbf{u}_i(\boldsymbol{\mu})\approx\sum\limits_{k=1}^{r_i}\Phi_{i,k}c_{i,k}(\boldsymbol{\mu}),
\text{\quad or \quad} \textbf{u}_i(\boldsymbol{\mu})\approx\boldsymbol{\Phi}_{i}\textbf{c}_{i}(\boldsymbol{\mu})

where $\boldsymbol{\Phi}_{i}=[\Phi_{i,1},...,\Phi_{i,r_i}]$ is a matrix with the POD modes as
columns and $\textbf{c}_{i}(\boldsymbol{\mu}) = [c_{i,1}(\boldsymbol{\mu}),...,c_{i,r_i}(\boldsymbol{\mu})]^T$
are the expansion coefficients. In essence, these coefficients
describe the coordinates of the approximate solution in the reduced subspace.
To approximate the full solution vector $\textbf{u}(\boldsymbol{\mu})$ using
its components ($\textbf{u}_i(\boldsymbol{\mu})$-s), a segregated
approach is used as follows:

!equation id=full_sol_approximation
\textbf{u}(\boldsymbol{\mu})
=
\left[
\begin{array}{c}
\textbf{u}_1\\
\vdots \\
\textbf{u}_{N_v}
\end{array}\right]
\approx
\left[
\begin{array}{c}
\boldsymbol{\Phi}_{1}\textbf{c}_{1}(\boldsymbol{\mu})\\
\vdots \\
\boldsymbol{\Phi}_{N_s}\textbf{c}_{N_s}(\boldsymbol{\mu})
\end{array}\right]
=
\left[
\begin{array}{cccc}
\boldsymbol{\Phi}_{1} & 0 & \cdots & 0\\
0 &  \boldsymbol{\Phi}_{2} & \cdots & 0\\
\vdots &  \vdots & \ddots & \vdots\\
0 &  0 & \cdots & \boldsymbol{\Phi}_{N_s}
\end{array}\right]
\left[
\begin{array}{c}
\textbf{c}_{1}(\boldsymbol{\mu})\\
\textbf{c}_{2}(\boldsymbol{\mu})\\
\vdots\\
\textbf{c}_{N_s}(\boldsymbol{\mu})
\end{array}\right]
=
\boldsymbol{\Phi}\textbf{c}(\boldsymbol{\mu}).

It is important to mention that in this approximation the unknowns are the elements
of $\textbf{c}(\boldsymbol{\mu})$ vector.
In most of the cases, the size of this vector ($\sum\limits_{i=1}^{N_v}r_i$) is considerably
smaller than the size of the original solution vector ($\sum\limits_{i=1}^{N_v}N_i$).

## Generation of reduced operators

To generate reduced opertors, [full_sol_approximation] is plugged into [fom_affine_decomp_discrete] first:

!equation id=fom_affine_decomp_discrete_approx
\left(\sum \limits_{i=1}^{N_A} f^A_i(\boldsymbol{\mu})\textbf{A}_i\right)\boldsymbol{\Phi}\textbf{c}(\boldsymbol{\mu}) =
\sum \limits_{i=1}^{N_b} f^b_i(\boldsymbol{\mu})\textbf{b}_i.

Since the size of \textbf{c} is smaller than the size of \textbf{u}, this equation is
underdetermined. To solve this problem, a Galerkin projection is used on the system:

!equation id=rom_affine_decomp_intro
\boldsymbol{\Phi}^T\left(\sum \limits_{i=1}^{N_A} f^A_i(\boldsymbol{\mu})\textbf{A}_i\right)\boldsymbol{\Phi}\textbf{c}(\boldsymbol{\mu}) =
\boldsymbol{\Phi}^T\sum \limits_{i=1}^{N_b} f^b_i(\boldsymbol{\mu})\textbf{b}_i.

By pulling the basis matrices into the summation, the following form is obtained.

!equation id=rom_affine_decomp_intermed
\left(\sum \limits_{i=1}^{N_A} f^A_i(\boldsymbol{\mu})\boldsymbol{\Phi}^T\textbf{A}_i\boldsymbol{\Phi}\right)\textbf{c}(\boldsymbol{\mu}) =
\sum \limits_{i=1}^{N_b} f^b_i(\boldsymbol{\mu})\boldsymbol{\Phi}^T\textbf{b}_i,

where the reduced operators $\textbf{A}_i^r=\boldsymbol{\Phi}^T\textbf{A}_i\boldsymbol{\Phi}$ and
source terms $\textbf{b}_i^r=\boldsymbol{\Phi}^T\textbf{b}_i$ can be precomputed once
the basis functions are available. Therefore, the reduced equation system that is solved
to obtain $\textbf{c}(\boldsymbol{\mu})$ is

!equation id=rom_affine_decomp_final
\left(\sum \limits_{i=1}^{N_A} f^A_i(\boldsymbol{\mu})\textbf{A}_i^r\right)\textbf{c}(\boldsymbol{\mu}) =
\sum \limits_{i=1}^{N_b} f^b_i(\boldsymbol{\mu})\textbf{b}_i^r.

It is important to note that this equation system is of size $\sum\limits_{i=1}^{N_v}r_i \times \sum\limits_{i=1}^{N_v}r_i$,
therefore it can be solved much faster than the original full-order system which is
of size $\sum\limits_{i=1}^{N_v}N_i \times \sum\limits_{i=1}^{N_v}N_i$.
The trainer object only assembles the reduced operators and source terms, which are then
transferred to [PODReducedBasisSurrogate.md] that is responsible for assembling
and solving [rom_affine_decomp_final] and reconstructing the approximate solutions
using [full_sol_approximation] if needed.

The computation of the reduced operators consists of two step in the current implementation:

1. +Computing the effect of the full-order operator on the global basis functions:+ this step
   includes the creation of $\textbf{A}_i\boldsymbol{\Phi}$. In practice, this is done by
   plugging in the basis function into a [PODFullSolveMultiApp.md] object which evaluates the residual for a given
   vector tag (defined using the [!param](/Trainers/PODReducedBasisTrainer/tag_names) input argument). The tagged residual is then
   transferred back to the trainer using a [PODResidualTransfer.md] object. In case when
   the residual from a kernel contains contributions to both the system matrix and the
   source term (e.g. Dirichlet BC or time derivative), certain input-level tricks
   can be used to separate these.

2. +Projection of the residual vectors:+ this step consists of computing the
   $\boldsymbol{\Phi}^T(\textbf{A}_i\boldsymbol{\Phi})$ inner products.

As a final note, it must emphasized that even though obtaining snapshots and creating reduced operators
is a computationally expensive procedure, it has to be carried out only once. After
this initial investment every new evaluation for a new parameter sample involves the summation
and scaling of small dense matrices, which is of low computational cost.

## Note on parallelism

As of now, the training phase is implemented in a semi-parallel manner. This means that
the snapshot generation, correlation matrix generation, base generation and
the computation of the reduced operators are all executed in parallel. However,
the eigenvalues and eigenvectors of the correlation matrices are obtained in serial.
Therefore, this phase may experience considerable slowdown when the number of
snapshots is large (above ~2000).   

## Example Input File Syntax

To get the snapshots, four essential blocks have to be added the main input file.
First, a sampler has to be defined in `Samplers` to generate realizations for $\boldsymbol{\mu}$.
These samples are then fed into a [PODFullSolveMultiApp.md] (defined
in the `MultApps` block) which is capable of running simulations for each parameter.
The solution vectors (snapshots) for each run are added to the trainer by a
[PODSamplerSolutionTransfer.md] defined in the `Transfers` block. It is important to mention
that the multiapp and transfer objects need to know about the trainer. This can be
ensured using the [!param](/Transfers/PODSamplerSolutionTransfer/trainer_name) input argument.
The number of collected snapshots is
defined in the sampler object using the [!param](/Samplers/LatinHypercube/num_rows) parameter.

!listing pod_rb/internal/trainer.i block=Samplers

!listing pod_rb/internal/trainer.i block=MultiApps

The global basis functions are then plugged back into the same [PODFullSolveMultiApp.md]
by a new [PODSamplerSolutionTransfer.md] and the residuals are evaluated. The residuals
are transferred back to the trainer using a [PODResidualTransfer.md].

!listing pod_rb/internal/trainer.i block=Transfers

Finally, the [PODReducedBasisTrainer.md] is defined in the `Trainers` block. It requires
the names of the variables ([!param](/Trainers/PODReducedBasisTrainer/var_names)) one wishes to create reduced basis for and the names of the
tags ([!param](/Trainers/PODReducedBasisTrainer/tag_names)) associated with the affine components of the full-order operator.
Additionally, a vector specifying which tag corresponds to a linear operator and which to
a source term needs to be added as well ([!param](/Trainers/PODReducedBasisTrainer/tag_types)). The available tag types are:

!table
| Tag type | Description |
| :- | - |
| op | Operator that acts on the solution vector (i.e. a matrix). |
| src | Operator that does not act on the solution vector (i.e. a source vector). |
| op_dir | Dirichlet operator that acts on the solution vector (i.e. Dirichlet penalty matrix). |
| src_dir | Dirichlet operator that does not act on the solution vector (i.e. Dirichlet pensalty source vector). |

To specify the allowed error in the snapshot reconstruction
for reach variable ($\tau_i$), one needs to specify [!param](/Trainers/PODReducedBasisTrainer/error_res).
In the following example the [!param](/Trainers/PODReducedBasisTrainer/execute_on) parameter is set to '`timestep_begin final`' which means
that at the beginning of the training it will create the basis functions while at the
end it will create the reduced operators.

!listing pod_rb/internal/trainer.i block=Trainers

!syntax parameters /Trainers/PODReducedBasisTrainer

!syntax inputs /Trainers/PODReducedBasisTrainer

!syntax children /Trainers/PODReducedBasisTrainer
