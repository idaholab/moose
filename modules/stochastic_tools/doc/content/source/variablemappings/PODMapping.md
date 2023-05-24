# PODMapping

!syntax description /VariableMappings/PODMapping

## Overview

A mapping which uses a Proper Orthogonal Decomposition (POD) to establish mapping between
(high-dimensional) solution vectors coming from numerical simulations and corresponding coefficients
in lower-dimensional spaces (latent spaces). For the use cases in MOOSE, this method is equivalent to
linear Principal Component Analysis (PCA) or Singular Value Decomposition (SVD).

The process for building the mapping is the following:

1. Collection of snapshots of the solution fields. This can be done by running the full-order models
   with different model parameters and saving the solution fields along the simulations. This
   process is established in [SolutionContainer.md], [SerializedSolutionTransfer.md] and
   [ParallelSolutionStorage.md].

2. Once the snapshots are at our disposal, we organize them into a snapshot matrix ($\boldsymbol{S}$) as follows:

   !equation id=data-matrix
   \boldsymbol{S}_{v} = \left[\boldsymbol{s}_{v,\mu_1,t_1}~\boldsymbol{s}_{v,\mu_1,t_2}~\boldsymbol{s}_{v,\mu_1,t_3}~ ... ~\boldsymbol{s}_{v,\mu_1,t_{N_{T_{\mu_1}}}}~ \boldsymbol{s}_{v,\mu_2,t_1}~ \boldsymbol{s}_{v,\mu_2,t_2}~ ...~ \boldsymbol{s}_{v,\mu_{N_\mu},t_{N-1}}~ \boldsymbol{s}_{v,\mu_{N_\mu},t_{N_{T_{\mu_{N_\mu}}}}} \right]~,

   where $N_\mu$ is the number of different model parameter samples, $N_{T_{\mu_i}}$ is the number of snapshots per transient simulation
   with parameter sample $\mu_i$, while $\boldsymbol{s}_{v,\mu_i,t_j}$ denotes the solution vector for variable $v$ with parameter sample $\mu_i$ at time step $t_j$. Note that the number of time steps might depend on the model parameters. The current implementation supports variable-based POD only.

3. As the next step, we extract the common features in the snapshots. In other words, we compute the POD or SVD of the
   snapshot matrix:

   !equation id=svd
   \boldsymbol{S}_v = \boldsymbol{U}_v \boldsymbol{\Sigma}_v \boldsymbol{V}_v^T~,

   where $\boldsymbol{U}$ and $\boldsymbol{V}$ are the unitary left and right singular vector matrices, respectively.
   Matrix $\boldsymbol{\Sigma}$ contains the singular values on its diagonal. Given that the size of the snapshot matrix
   is high, we use the parallel SVD solver available in SLEPc with a Lánczos method to solve the underlying
   eigenvalue problem and get only the requested number of singular values. Additional parameters for SLEPc to fine tune
   the solution algorithm can be supplied using input parameter [!param](/VariableMappings/PODMapping/extra_slepc_options).

   The number of needed singular triplets
   ($\boldsymbol{u}_{v,i}, \sigma_{v,i}, \boldsymbol{v}_{v,i}$) for each variable can be prescribed using
   the [!param](/VariableMappings/PODMapping/num_modes_to_compute) parameter. The algorithm will select the minimum of these numbers and the
   number of converged singular triplets. Let's denote these numbers by $r_v$.

   For this mapping object the transition from high-dimensional to low-dimensional and back is determined by the left
   singular vector computed in the decomposition. We assume that the solution vector ($\boldsymbol{s}_v$)
   of the system for variable $v$ can be approximated as

   !equation id=inverse-mapping
   \boldsymbol{s}_v \approx \boldsymbol{U}_v \boldsymbol{c}_v~,

   where vector $\boldsymbol{c}_v$ contains the reduced-order coefficients (or coordinates in the latent) space
   in the low-dimensional space. Considering that the left singular vector vector matrix is unitary, we can
   express the mapping from high to low dimensional spaces as:

   !equation id=mapping
   \boldsymbol{c}_v = \boldsymbol{U}^T_v \boldsymbol{s}_v~.

4. The last step in this process consists of filtering out singular triplets which do not contribute to the
   description of variation in the variable fields. For this, we utilize the singular values as follows:

   !equation id=truncation
   \begin{array}{lr}
        \text{keep basis vector $I$}, & \text{if\quad} \left(1 - \frac{\sum\limits_{i=1}^{I} \sigma^2_{v,i}}{\sum\limits_{i=1}^{r_v} \sigma_{v,i}^2}\right) < \tau_v\\ & \\
        \text{discard}, & \text{otherwise.}
   \end{array}

   The filtering parameter $\tau_v$ van be specified for every variable using input parameter [!param](/VariableMappings/PODMapping/energy_threshold).

Once a mapping is trained, one can save it into a binary file using [MappingOutput.md] and load it by specifying
the [!param](/VariableMappings/PODMapping/filename) parameter in the object.

!alert warning
This object is only compatible with PETSc versions above 3.14 with SLEPc support.

## Example Input File Syntax

Creating a mapping object:

!listing test/tests/variablemappings/pod_mapping/pod_mapping_main.i block=VariableMappings

Loading a mapping object:

!listing test/tests/userobjects/inverse_mapping/inverse_map.i block=VariableMappings

## Syntax

!syntax parameters /VariableMappings/PODMapping

!syntax inputs /VariableMappings/PODMapping

!syntax children /VariableMappings/PODMapping
