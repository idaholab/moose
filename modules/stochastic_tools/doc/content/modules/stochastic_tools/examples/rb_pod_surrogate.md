# POD Reduced Basis Surrogate

This example is meant to demonstrate how a POD reduced basis surrogate model is trained
and used on a parametric problem.

## Problem statement

The full-order model is a one energy group, fixed-source neutron diffusion problem, adopted from [!cite](prince2019parametric).
It showcases the core of a Pressurized Water Reactor (PWR) in 2D. The used geometry
is presented in [pwr_geom]. The problem has four different material regions, from which
three (1, 2 and 3) act as neutron sources.

!media 2d_multiregion_geometry.png style=display:block;margin-left:auto;margin-right:auto;width:40%;
       id=pwr_geom caption=The geometry of the PWR core used in this example ([!cite](prince2019parametric)).

The fixed-source one-group neutron diffusion with space dependent cross-sections can be expressed as:

!equation id=fom_problem
-\nabla \cdot\left[D(\textbf{r})\nabla\psi(\textbf{r})\right]+\Sigma_a(\textbf{r})\psi(\textbf{r}) = q(\textbf{r}) \,, \quad \textbf{r}\in\Omega

where $D(\textbf{r})$ is the diffusion coefficient, $\Sigma_a(\textbf{r})$ is the
absorption cross-section, $q(\textbf{r})$ is the fixed source term and
field variable \psi(\textbf{r}) is the scalar neutron flux. Furthermore, $\Omega$
denotes the internal domain, without the boundaries. This domain can be
partitioned into four sub-domains corresponding to the four material regions ($\Omega_1$, $\Omega_2$, $\Omega_3$ and $\Omega_4$).
This equation needs to be supplemented with boundary conditions.
For the symmetry lines (dashed in [pwr_geom], denoted by $\partial\Omega_{sym}$) a homogeneous Neumann condition is used:

!equation
-D(\textbf{r})\nabla\psi(\textbf{r}) = 0 \,, \quad \textbf{r}\in\partial\Omega_{sym}

where while the rest of the boundaries (in the reflectors, denoted by $\partial\Omega_{refl}$) are treated with homogeneous
Dirichlet conditions:

!equation
\psi(\textbf{r}) = 0 \,, \quad \textbf{r}\in\partial\Omega_{refl}

This problem is parametric in a sense that the solution for the scalar flux depends on the values of the
cross-sections and the source: $\psi = \psi\left(\textbf{r},D(\textbf{r}),\Sigma_a(\textbf{r}),q(\textbf{r})\right)$.
In this example, material region-wise constant cross-sections and source terms are considered
yielding eight uncertain parameters altogether (assuming that Region 4 does not have a source).
The material properties in each region have [Uniform](Uniform.md) distributions ($\mathcal{U}(a,b)$)
specified in [param_distributions] with $a$ and $b$ being the lower and upper bounds of the
distribution.

!table id=param_distributions caption=The distributions of the uncertain parameters used in this problem ([!cite](prince2019parametric)).
| Parameter | Symbol | Distribution |
| :- | - | - | - |
| Diffusion coefficient in Region 1 $\left(cm\right)$ | $D_1$ | $\sim\mathcal{U}(0.2, 0.8)$ |
| Diffusion coefficient in Region 2 $\left(cm\right)$ | $D_2$ | $\sim\mathcal{U}(0.2, 0.8)$ |
| Diffusion coefficient in Region 3 $\left(cm\right)$ | $D_3$ | $\sim\mathcal{U}(0.2, 0.8)$ |
| Diffusion coefficient in Region 4 $\left(cm\right)$ | $D_4$ | $\sim\mathcal{U}(0.15, 0.6)$ |
| Absorption cross-section in Region 1 $\left(cm^{-1}\right)$ | $\Sigma_{a,1}$ | $\sim\mathcal{U}(0.0425, 0.17)$ |
| Absorption cross-section in Region 2 $\left(cm^{-1}\right)$ | $\Sigma_{a,2}$ | $\sim\mathcal{U}(0.065, 0.26)$ |
| Absorption cross-section in Region 3 $\left(cm^{-1}\right)$ | $\Sigma_{a,3}$ | $\sim\mathcal{U}(0.04, 0.16)$ |
| Absorption cross-section in Region 4 $\left(cm^{-1}\right)$ | $\Sigma_{a,4}$ | $\sim\mathcal{U}(0.005, 0.02)$ |
| Fixed-source in Region 1 $\left(\frac{n}{cm^3 \cdot s}\right)$ | $q_1$ | $\sim\mathcal{U}(5, 20)$ |
| Fixed-source in Region 2 $\left(\frac{n}{cm^3 \cdot s}\right)$ | $q_2$ | $\sim\mathcal{U}(5, 20)$ |
| Fixed-source in Region 3 $\left(\frac{n}{cm^3 \cdot s}\right)$ | $q_3$ | $\sim\mathcal{U}(5, 20)$ |

It is important to mention that POD-RB surrogates are only efficient when the original
problem has an affine decomposition. For more information about affine decomposition see
[PODReducedBasisTrainer.md]. Luckily, there is an affine decomposition for [fom_problem] in
the following form:

!equation id=fom_problem
-\sum\limits_{i=1}^{4}\nabla \cdot\left[D_i(\textbf{r})\nabla\psi(\textbf{r})\right]+\sum\limits_{i=1}^{4}\Sigma_{a,i}(\textbf{r})\psi(\textbf{r}) = \sum\limits_{i=1}^{3}q_i(\textbf{r}) \,, \quad \textbf{r}\in\Omega

where $D_i(\textbf{r})$, $\Sigma_{a,i}(\textbf{r})$ and $q_i(\textbf{r})$ take the values of $D_i$, $\Sigma_{a,i}$ and $q_i$
when $\textbf{r}\in\Omega_i$ and 0 otherwise.

## Solving the problem without uncertain parameters

The first step towards creating a POD-RB surrogate model is the generation of a [full-order problem](surrogates/pod_rb/2d_multireg/sub.i)
which can solve [fom_problem] with fixed parameters.
There are +three important factors+ that need to be considered while preparing the input file for this problem:

1. The user must specify vector tags in the `Problem` block for each component in the affine decomposition of the system.
   In this, as shown in [vector_tags], 8 vector tags are specified for the eight uncertain parameters. These
   do not do anything in the full-order model, however they help to identify the affine components
   throughout the training phase.

   !listing surrogates/pod_rb/2d_multireg/sub.i id=vector_tags block=Problem
          caption=Complete input file for the heat equation problem in this study.

2. The input file has to reflect the affine decomposition of the problem. This means that the `Kernels`,
   `BCs` and `Materials` have to be created in a way that they correspond to the components in the
   affine decomposition. This is shown in [affine_kernels]. Note that a separate kernel has been
   created for every single term in the decomposition. The vector tags in the `Problem` block
   are then applied to these kernels to ensure that the affine components are correctly identified
   throughout the training phase.

   !listing surrogates/pod_rb/2d_multireg/sub.i id=affine_kernels block=Kernels Materials
          caption=Complete input file for the heat equation problem in this study.

3. The values for each uncertain parameter have to be set to 1 by default. This is necessary because
   the [PODReducedBasisTrainer.md] uses the same input file to create the affine constituent operators.
   This ensures that the mentioned operators are not influenced by the parameter-dependent multipliers.
   Of course, these values are not fixed and are changed by the main application throughout the simulations to
   values aligned with those in [param_distributions]. However, the default values in the input file
   should be set to one. This is shown in [affine_kernels] as well.

## Training surrogate models

For the details of the training procedure of a POD-RB surrogate, see [PODReducedBasisTrainer.md].
The first step is the collection of data, which involves the repeated execution of the
with different parameter combinations and the saving of the full solution vectors.
These solution vectors are often referred to as snapshots and this naming is preferred in
this example as well.
This step is managed by the [main input file](surrogates/pod_rb/2d_multireg/trainer.i) which creates parameter samples,
transfers them to the sub-application and collects the results from the
completed computations.

The snapshot collection phase starts with the definition of the distributions
in the `Distributions` block. The uniform distributions for all the 8 uncertain parameters
are specified as:

!listing surrogates/pod_rb/2d_multireg/trainer.i block=Distributions

As a next step, the underlying distributions are sampled to create parameter combinations.
This done using a LatinHypercubeSampler.md] defined in the  `Samplers` block.
It is visible that 100 samples are prepared, meaning that 100 snapshots will be collected
for the generation of the surrogates.

!listing surrogates/pod_rb/2d_multireg/trainer.i block=Samplers  

To be able to create the reduced operators for the surrogate model, a custom `MultiApp`,
[PODFullSolveMultiApp.md] has been created. This object is responsible for executing
sub-problems using different combinations of parameter values provided by the sampler.
The secondary function of this object is to create the action of the full-order operators
on the basis functions of the reduced subspace. Therefore, this object has to be executed
twice in the same simulation. It is visible that unlike a regular [SamplerFullSolveMultiApp.md],
this custom object has to know certain parameters of the trainer as well.

!listing surrogates/pod_rb/2d_multireg/trainer.i block=MultiApps

The `Transfers` block became slightly more convoluted as well. Besides sending the
actual parameter samples to the sub-applications, in this intrusive procedure,
the snapshots need to be collected from the sub-applications, the basis functions
need to be sent back to different sub-applications and the action of the operators on
the basis functions need to be collected as well. This requires four transfer objects,
out of which two ([SamplerSolutionTransfer.md] and [ResidualTransfer.md]) are specifically
used to support [PODReducedBasisTrainer.md] at this moment.

!listing surrogates/pod_rb/2d_multireg/trainer.i block=Transfers

Next, the [PODReducedBasisTrainer.md] is set up in the `Trainers` block.
The trainer stores the snapshots and uses them to create basis functions
for the reduced subspaces. Furthermore, it is also responsible for creating
the reduced-order operators. These two tasks can be only carried out one after
the other, therefore the trainer object needs to be executed twice.
The trainer object needs to know what variable needs to be reduced and
the names of the vector tags from the sub-application to be able to identify the
affine constituent operators. Furthermore, using the `independent` input argument
one has to specify if the affine constituent operator acts on the variable or not.
The ordering must be the same as the names of the vector tags. The meaning of the
limit for energy retention is discussed in [PODReducedBasisTrainer.md].

!listing surrogates/pod_rb/2d_multireg/trainer.i block=Trainers

As a last step in the training process, the basis functions, reduced operators and
every necessay information for the surrogate are saved into and `.rd` file.
This file can be then used to construct a surrogate model
without the need to repeat the training process.

!listing surrogates/pod_rb/2d_multireg/trainer.i block=Outputs

## Evaluation of surrogate models

To evaluate surrogate models, a new [main input file](surrogates/pod_rb/2d_multireg/surr.i) has to be created for.
The same distributions have to be defined for the parameters to be able to test the
generated surrogate models. Therefore, the content of the `Distributions` block is identical to the
one in the [trainer input file](surrogates/pod_rb/2d_multireg/trainer.i).
As a next step, new samples are generated using these distributions. Again, a [LatinHypercubeSampler.md]
is employed for this task, however this time the number of samples is 1000 since the
surrogates are orders of magnitudes faster than the full-order model.

!listing surrogates/pod_rb/2d_multireg/surr.i block=Samplers  

As a next step, a [PODReducedBasisSurrogate.md] is created in the `Surrogates` block.
It is constructed using the information available within the corresponding `.rd` file.
The surrogate allows the change of the rank of the sub-spaces used for different parameters
through `change_rank` and `new_ranks` parameters.

!listing surrogates/pod_rb/2d_multireg/surr.i block=Surrogates

These surrogate models can be evaluated at the points defined in the testing sample batch.
This is done using a `PODSurrogateTester` object in the `VectorPostprocessors` block.
In this case the Quantity of Interest (QoI) is the nodal $l^2$ norm of the solution for $\psi$. 

!listing surrogates/pod_rb/2d_multireg/surr.i block=VectorPostprocessors

## Results and Analysis



!plot scatter
  id=convergence caption=The convergence of averaged quantities of interest.
  filename=examples/surrogates/pod_rb/2d_multireg/gold/2d_multireg_results.csv
  data=[{'x':'no_modes', 'y':'mean', 'name':'Average relative error'},
        {'x':'no_modes', 'y':'max', 'name':'Maximum relative error'}]
  layout={'xaxis':{'type':'log', 'title':'Number of basis functions'},
          'yaxis':{'type':'log','title':'Relative Error'}}
