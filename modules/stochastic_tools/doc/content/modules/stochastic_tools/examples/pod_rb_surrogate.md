# POD Reduced Basis Surrogate

This example is meant to demonstrate how a POD reduced basis surrogate model is trained
and used on a parametric problem.

## Problem statement

The full-order model is a one energy group, fixed-source diffusion-reaction problem, adopted from [!cite](prince2019parametric).
The geometry for this problem
is presented in [pwr_geom]. The problem has four different material regions, from which
three (1, 2 and 3) act as fixed sources.

!media 2d_multiregion_geometry.png style=display:block;margin-left:auto;margin-right:auto;width:40%;
       id=pwr_geom caption=The geometry of the diffusion-reaction problem used in this example ([!cite](prince2019parametric)).

The fixed-source diffusion-reaction problem with space dependent coefficients can be expressed as:

!equation id=fom_problem
-\nabla \cdot\left[D(\textbf{r})\nabla\psi(\textbf{r})\right]+\Sigma_a(\textbf{r})\psi(\textbf{r}) = q(\textbf{r}) \,, \quad \textbf{r}\in\Omega

where $D(\textbf{r})$ is the diffusion coefficient, $\Sigma_a(\textbf{r})$ is the
reaction coefficient, $q(\textbf{r})$ is the fixed source term and
field variable $\psi(\textbf{r})$ is the solution of interest. Furthermore, $\Omega$
denotes the internal domain, without the boundaries, which can be
partitioned into four sub-domains corresponding to the four material regions ($\Omega_1$, $\Omega_2$, $\Omega_3$ and $\Omega_4$).
This equation needs to be supplemented with boundary conditions.
For the symmetry lines (dashed lines in [pwr_geom], denoted by $\partial\Omega_{sym}$) a homogeneous Neumann condition is used:

!equation
-D(\textbf{r})\nabla\psi(\textbf{r})\cdot \textbf{n}(\textbf{r}) = 0 \,, \quad \textbf{r}\in\partial\Omega_{sym}

while the rest of the boundaries (in the reflector, denoted by $\partial\Omega_{refl}$) are treated with homogeneous
Dirichlet conditions:

!equation
\psi(\textbf{r}) = 0 \,, \quad \textbf{r}\in\partial\Omega_{refl}

This problem is parametric in a sense that the solution depends on the values of the
coefficients and the source: $\psi = \psi\left(\textbf{r},D(\textbf{r}),\Sigma_a(\textbf{r}),q(\textbf{r})\right)$.
In this example, material region-wise constant coefficients and source terms are considered
yielding eight uncertain parameters altogether (assuming that Region 4 does not have a source).
The material properties in each region have [Uniform](Uniform.md) distributions ($\mathcal{U}(a,b)$)
specified in [param_distributions] with $a$ and $b$ being the lower and upper bounds.

!table id=param_distributions caption=The distributions of the uncertain parameters used in this problem ([!cite](prince2019parametric)).
| Parameter | Symbol | Distribution |
| :- | - | - | - |
| Diffusion coefficient in Region 1 $\left(cm\right)$ | $D_1$ | $\sim\mathcal{U}(0.2, 0.8)$ |
| Diffusion coefficient in Region 2 $\left(cm\right)$ | $D_2$ | $\sim\mathcal{U}(0.2, 0.8)$ |
| Diffusion coefficient in Region 3 $\left(cm\right)$ | $D_3$ | $\sim\mathcal{U}(0.2, 0.8)$ |
| Diffusion coefficient in Region 4 $\left(cm\right)$ | $D_4$ | $\sim\mathcal{U}(0.15, 0.6)$ |
| Reaction coefficient in Region 1 $\left(cm^{-1}\right)$ | $\Sigma_{a,1}$ | $\sim\mathcal{U}(0.0425, 0.17)$ |
| Reaction coefficient in Region 2 $\left(cm^{-1}\right)$ | $\Sigma_{a,2}$ | $\sim\mathcal{U}(0.065, 0.26)$ |
| Reaction coefficient in Region 3 $\left(cm^{-1}\right)$ | $\Sigma_{a,3}$ | $\sim\mathcal{U}(0.04, 0.16)$ |
| Reaction coefficient in Region 4 $\left(cm^{-1}\right)$ | $\Sigma_{a,4}$ | $\sim\mathcal{U}(0.005, 0.02)$ |
| Fixed-source in Region 1 $\left(\frac{n}{cm^3 \cdot s}\right)$ | $q_1$ | $\sim\mathcal{U}(5, 20)$ |
| Fixed-source in Region 2 $\left(\frac{n}{cm^3 \cdot s}\right)$ | $q_2$ | $\sim\mathcal{U}(5, 20)$ |
| Fixed-source in Region 3 $\left(\frac{n}{cm^3 \cdot s}\right)$ | $q_3$ | $\sim\mathcal{U}(5, 20)$ |

It is important to mention that POD-RB surrogates are only efficient when the original
problem has an affine decomposition. For more information about affine decomposition see
[PODReducedBasisTrainer.md]. Luckily, the problem at hand has an affine decomposition in
the following form:

!equation
-\sum\limits_{i=1}^{4}\nabla \cdot\left[D_i(\textbf{r})\nabla\psi(\textbf{r})\right]+\sum\limits_{i=1}^{4}\Sigma_{a,i}(\textbf{r})\psi(\textbf{r}) = \sum\limits_{i=1}^{3}q_i(\textbf{r}) \,, \quad \textbf{r}\in\Omega

where $D_i(\textbf{r})$, $\Sigma_{a,i}(\textbf{r})$ and $q_i(\textbf{r})$ take the values of $D_i$, $\Sigma_{a,i}$ and $q_i$
when $\textbf{r}\in\Omega_i$ and 0 otherwise.

## Solving the problem without uncertain parameters

The first step towards creating a POD-RB surrogate model is the generation of a [full-order problem](surrogates/pod_rb/2d_multireg/sub.i)
which can solve [fom_problem] with fixed parameters.
There are +three important factors+ that need to be considered while preparing the input file for this problem:

1. The user must specify vector tags in the `Problem` block for each component in the affine decomposition of the system.
   In this example, as shown in [vector_tags], 8 vector tags are specified for the eight uncertain parameters. These
   do not introduce extra work in the full-order model, however they help to identify the affine components
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
[full-order problem](surrogates/pod_rb/2d_multireg/sub.i)
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
This is done using a [LatinHypercubeSampler.md] defined in the  `Samplers` block.
It is visible that 100 samples are prepared, meaning that 100 snapshots will be collected
for the generation of the surrogates.

!listing surrogates/pod_rb/2d_multireg/trainer.i block=Samplers  

To be able to create the reduced operators for the surrogate model, a custom `MultiApp`,
[PODFullSolveMultiApp.md], has been created. This object is responsible for executing
sub-problems using different combinations of parameter values provided by the sampler.
The secondary function of this object is to create the action of the full-order operators
on the basis functions of the reduced subspace. Therefore, this object has to be executed
twice in the same simulation. It is visible that unlike a regular [SamplerFullSolveMultiApp.md],
this custom object has to know certain parameters of the trainer as well.

!listing surrogates/pod_rb/2d_multireg/trainer.i block=MultiApps

In terms of the `Transfers` block, besides sending the
actual parameter samples to the sub-applications, in this intrusive procedure,
the snapshots need to be collected from the sub-applications, the basis functions
need to be sent back to different sub-applications and the action of the operators on
the basis functions need to be collected as well. This requires four transfer objects.
The two custom types ([PODSamplerSolutionTransfer.md] and [PODResidualTransfer.md]) are specifically
used to support [PODReducedBasisTrainer.md] at this moment.

!listing surrogates/pod_rb/2d_multireg/trainer.i block=Transfers

Next, the [PODReducedBasisTrainer.md] is set up in the `Trainers` block.
The trainer stores the snapshots and uses them to create basis functions
for the reduced subspaces. Furthermore, it is also responsible for creating
the reduced-order operators, therefore it needs to be executed twice in the training process.
The trainer object needs to know what variable needs to be reduced and
the names of the vector tags from the sub-application to be able to identify the
affine constituent operators. Furthermore, using the [!param](/Trainers/PODReducedBasisTrainer/tag_types) input argument,
the user has to specify if the reduced affine constituent operator acts on the variable or not.
The ordering must be the same as the names of the vector tags. The meaning of the
energy retention limits is discussed in [PODReducedBasisTrainer.md].

!listing surrogates/pod_rb/2d_multireg/trainer.i block=Trainers

As a last step in the training process, the basis functions, reduced operators and
every necessary information for the surrogate are saved into an `.rd` file.
This file can be then used to construct a surrogate model
without the need to repeat the training process.

!listing surrogates/pod_rb/2d_multireg/trainer.i block=Outputs

## Evaluation of surrogate models

To evaluate surrogate models, a new [main input file](surrogates/pod_rb/2d_multireg/surr.i) has to be created.
In this example, the same distributions are defined for the parameters as used in the training phase.
Therefore, the content of the `Distributions` block is identical to the
one in the [trainer input file](surrogates/pod_rb/2d_multireg/trainer.i).
As a next step, new samples are generated using these distributions. Again, a [LatinHypercubeSampler.md]
is employed for this task, however this time the number of samples is increased to 1000 since the
surrogates are orders of magnitudes faster than the full-order model.

!listing surrogates/pod_rb/2d_multireg/surr.i block=Samplers  

A [PODReducedBasisSurrogate.md] is created in the `Surrogates` block.
It is constructed using the information available within the corresponding `.rd` file
and allows the user to change of the rank of the sub-spaces used for different variables
through [!param](/Surrogates/PODReducedBasisSurrogate/change_rank) and
[!param](/Surrogates/PODReducedBasisSurrogate/new_ranks) parameters.

!listing surrogates/pod_rb/2d_multireg/surr.i block=Surrogates

These surrogate models can be evaluated at the points defined in the testing sample batch
by a `PODSurrogateTester` object in the `VectorPostprocessors` block.
In this case, the Quantity of Interest (QoI) is the nodal $L^2$ norm of the solution for $\psi$.

!listing surrogates/pod_rb/2d_multireg/surr.i block=VectorPostprocessors

## Running the input files

Since the sub-applications use test objects, one has to allow the executioner to use them
by specifying the following argument on the command line:

```
cd moose/modules/stocastic_tools/examples/surrogates/pod_rb/2d_multireg
../../../../stochastic_tools-opt -i trainer.i --allow-test-objects
```

The same is true for the surrogate input file as well, therefore one needs to
start the executions as follows:

```
cd moose/modules/stocastic_tools/examples/surrogates/pod_rb/2d_multireg
../../../../stochastic_tools-opt -i surr.i --allow-test-objects
```

## Results and Analysis

In the following subsection a short analysis is provided for the results obtained
using the example input files. As already mentioned, the problem has 8 uncertain parameters and
altogether 100 parameter samples are generated using [LatinHypercubeSampler.md]
to obtain snapshots for the training. Three examples of the snapshots are presented in
[snap_1], [snap_2] and [snap_3]. It is visible that depending on the actual parameter combination,
the profile of the solution can change considerably.

!row!

!media 2d_multiregion_sol0.png style=width:33%;float:left
       id=snap_1 caption=Snap. example #1.

!media 2d_multiregion_sol1.png style=width:33%;float:left
       id=snap_2 caption=Snap. example #2.

!media 2d_multiregion_sol2.png style=width:33%;float:left
       id=snap_3 caption=Snap. example #3.

!row-end!

After all of the snapshots are obtained, the basis functions of the reduced subspaces are extracted.
In this scenario, an energy retention limit of 0.999 999 999 is used in the trainer
which will keep 55 basis functions for the reduced subspace. The decay of the
eigenvalues of the snapshot correlation matrix is shown in [ev_decay].
The reduced operators are then computed using these 55 basis functions.

!plot scatter id=ev_decay caption=Scree plot of the eigenvalues of the correlation matrix.
  filename=examples/surrogates/pod_rb/2d_multireg/gold/eigenvalues_psi.csv
  data=[{'x':'no', 'y':'evs', 'name':'Eigenvalues'}]
  layout={'xaxis':{'title':'Basis function'},
          'yaxis':{'type':'log','title':'Corresponding eigenvalue'}}
  style=display:block;margin-left:auto;margin-right:auto;width:40%

As a next step, two surrogate models are prepared using the
[!param](/Surrogates/PODReducedBasisSurrogate/change_rank) and
[!param](/Surrogates/PODReducedBasisSurrogate/new_ranks)
parameters of [PODReducedBasisSurrogate.md] to change the size of the reduced system.
The first surrogate model has 1 basis function, while the other has 8. Both
models are then run on a 1000 sample test set and the nodal $L^2$ norms of the approximate solutions
are saved. Additionally, a full-order model was executed on the same test set and the results
are saved to serve as reference values.
[hist_1] presents the results with the surrogate model built with 1 basis function only. It is
visible that the distribution of the QoI (nodal $L^2$ norm) on the test set is considerably different
than the reference distribution.

!media 2d_multireg_1.svg style=display:block;margin-left:auto;margin-right:auto;width:70%;
       id=hist_1 caption=The histogram of the QoI for the full-order reference model and the surrogate built with 1 basis function.

[hist_2] shows the distribution of the QoI obtained by a surrogate with 8 basis functions.
It is visible that the difference between the reference values and those from the surrogate is
negligible.

!media 2d_multireg_8.svg style=display:block;margin-left:auto;margin-right:auto;width:70%;
       id=hist_2 caption=The histogram of the QoI for the full-order reference model and the surrogate built with 8 basis function.

To see the convergence of the results from the surrogate to those of the full-order model,
the surrogate model is run multiple times with different ranks and
the following error indicators are computed for each sample in the test set:

!equation
e_i = \frac{||\psi_{Ref}-\psi_{Surr}||_{l^2}}{||\psi_{Ref}||_{l^2}}.\quad i=1,...,1000

Then, the maximum and average relative errors are recorder as function of the number of basis functions used.
[convergence] shows the results. It is visible that by increasing the number of basis functions,
both error indicators decrease rapidly.

!plot scatter
  id=convergence caption=The convergence of averaged quantities of interest.
  filename=examples/surrogates/pod_rb/2d_multireg/gold/2d_multireg_results.csv
  data=[{'x':'no_modes', 'y':'mean', 'name':'Average e'},
        {'x':'no_modes', 'y':'max', 'name':'Maximum e'}]
  layout={'xaxis':{'type':'log', 'title':'Number of basis functions used'},
          'yaxis':{'type':'log','title':'Relative error'}}

Lastly, the computation time full-order model on the test set is compared to the
combined cost of training and evaluating a POD-RB surrogate model in [com_time].
The test has been carried out on one processor only, not using the parallel capabilities of the
MultiApp system. The results show that it is beneficial to create a POD-RB surrogate
if more than 148 evaluations are needed. This assumes that the full-order evaluation time
can be equally distributed among the 1000 test samples (0.779 s/sample). By dividing the
training time with this number we get a critical sample number above which the
generation of a surrogate model is a better alternative.

!table id=com_time caption=The computation time of the full-order solutions on the test set compared to the cost of training a surrogate and evaluating it on the same test set.
| Process | Execution time (s) |
| :- | - | - | - |
| Evaluation of the full-order model on the 1000 sample test set | 779.5 |
| Training a POD-RB surrogate using 100 samples | 116.2 |
| Evaluation of the POD-RB surrogate on the 1000 sample test set (4 basis functions) | 0.592 |
| Evaluation of the POD-RB surrogate on the 1000 sample test set (8 basis functions) | 0.937 |
| Evaluation of the POD-RB surrogate on the 1000 sample test set (16 basis functions) | 1.576 |
