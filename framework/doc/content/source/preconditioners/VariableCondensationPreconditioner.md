# VCP

!syntax description /Preconditioning/VCP

## Overview

The Variable Condensation Preconditioner (VCP) is designed to condense out variables from the linear system of equations and apply the preconditioner/solver on the reduced simplified system of equations.
The development of VCP is motivated by the need to enable a broader range of robust and scalable preconditioners for problems that have a saddle point type of Jacobian. The saddle point type of Jacobian may come from different applications. One typical example is the enforcement of constraints using Lagrange multipliers. Its special numerical character prevents the usage of many scalable iterative solvers. To resolve this issue, a static condensation step is carried out in VCP to remove the Degree of Freedoms (DoFs) that are associated with the Lagrange multipliers.  
This may result in an easy-to-solve system (sometimes it is positive definite ) which can be handled by a broader range of solvers/preconditioners with improved efficiency. With VCP, we can efficiently apply iterative solvers, e.g., BoomerAMG, to mortar-based mechanical contact problems.

## System Simplification

To illustrate the saddle point structure of the Jacobian matrix and how it can be simplified by static condensation of the Lagrange multiplier, we take the Jacobian matrix from a simple diffusion problem with equal value constraint as an example.
The system of equations at a typical time step can be written as a block matrix as follows,
\begin{equation}
	\begin{bmatrix}
  K_{1, ii} &  K_{1, ic} & & & \\
  K_{1, ci} &  K_{1, cc} & & & M\\
  & & K_{2, ii} &  K_{2, ic} & \\
  & & K_{2, ci} &  K_{2, cc} & D\\
  & M^{\intercal}&  &  D^{\intercal} & 0\\
	\end{bmatrix}_{}
	\begin{bmatrix}
\Delta u_{1,i} \\ \Delta u_{1,c} \\ \Delta u_{2,i} \\ \Delta u_{2,c} \\ \Delta \lambda
	\end{bmatrix}
=
	\begin{bmatrix}
\Delta r_{1,i} \\ \Delta r_{1,c} \\ \Delta r_{2,i} \\ \Delta r_{2,c} \\ 0
	\end{bmatrix}
\label{eqn:original_global_matrix}
\end{equation}
Here, we omit the identifier of time for simplicity. The first subscript $(\cdot)_1$ and $(\cdot)_2$ denote the primary and secondary subdomains, respectively. The second subscript $(\cdot)_{\cdot, c}$ denotes the part of the subdomain that is in contact and $(\cdot)_{\cdot, i}$ denotes the rest of the subdomain. The blocks $K_{\cdot, \cdot}$ denote the respective stiffness matrices. The block $D$ represents the coupling between the Lagrange multipliers and the primal variable in the secondary subdomain. The block $M$ denotes the coupling between the Lagrange multipliers and the primal variable in the primary subdomain.

The system of equations from mortar-based mechanical contact has a similar saddle point character, but is more complex than [eqn:original_global_matrix], considering the primary and secondary surfaces being partially in contact. For readers who are interested in the Jacobian matrix from mortar-based mechanical contact, please refer to [!citep](popp2010dual).

### Condensation of the Lagrange Multiplier

The discrete Lagrange multipliers (i.e., $\Delta \lambda$) can be eliminated by condensation as follows,
\begin{equation}
\Delta \lambda = D^{-1}(\Delta r_{2,c}-K_{2,ci} \Delta u_{2,i}-K_{2,cc} \Delta u_{2,c}).
\label{eqn:condensation_of_LMs}
\end{equation}

By substituting [eqn:condensation_of_LMs] into [eqn:original_global_matrix], we obtain a simplified linear system of equations that  contains only the primal variable DOFs,

\begin{equation}
	\begin{bmatrix}
  K_{1, ii} &  K_{1, ic} & & \\
  K_{1, ci} &  K_{1, cc} & -MD^{-1}K_{2,ii} &  -MD^{-1}K_{2,cc} \\
  &  & K_{2, ii} &  K_{2, ic} \\
  & M^{\intercal}&  &  D^{\intercal}\\
	\end{bmatrix}
	\begin{bmatrix}
\Delta u_{1,i} \\ \Delta u_{1,c} \\ \Delta u_{2,i} \\ \Delta u_{2,c}
	\end{bmatrix}
=
	\begin{bmatrix}
\Delta r_{1,i} \\ \Delta r_{1,c} - MD^{-1}\Delta r_{2,c} \\ \Delta r_{2,i} \\ 0
	\end{bmatrix}
\label{eqn:condensed_global_matrix}
\end{equation}
This condensed system (i.e., [eqn:condensed_global_matrix]) is positive definite and therefore state-of-art iterative solution techniques, such as multigrid methods, are applicable. As a post-processing step, the Lagrange multipliers can be recovered from the displacement following [eqn:condensation_of_LMs].

### Dual Basis Function

The computation of $D^{-1}$ in [eqn:condensation_of_LMs] and [eqn:condensed_global_matrix] can be reduced by using +dual basis+ for the Lagrange multipliers. The +dual basis+ is a relative definition to the +standard basis+, which satisfies a biorthogonal condition as follows
\begin{equation}
\label{eqn:local_biorthogonal_condition}
	\int_{\Gamma_e} \psi_j (\boldsymbol{x}) \phi_k(\boldsymbol{x})~\text{d}s = \delta_{jk} \int_{\Gamma_e} \phi_k(\boldsymbol{x})~\text{d}s,
\end{equation}
where $\phi_k(\boldsymbol{x})$ is the standard shape function, $\psi_j (\boldsymbol{x})$ is the dual shape function, and $\delta_{jk}$ is the Kronecker delta function. This biorthogonal condition can be assumed to hold in every lower-dimensional element $\Gamma_e$. Owing to the biorthogonality property of the +dual basis+ functions, the integral matrix $D$ become strict diagonal. Thus computation of $D^{-1}$ and condensation steps in [eqn:condensation_of_LMs] and [eqn:condensed_global_matrix] become trivial.

!alert note title=Use of dual basis is optional but recommended
In order to reduce the computational cost brought by the condensation steps (see [eqn:condensation_of_LMs] and [eqn:condensed_global_matrix]), we recommend the usage of +dual basis+ function for the Lagrange multipliers. Meanwhile, we suggest setting the option `is_lm_coupling_diagonal = true` in the preconditioning block to let VCP take the diagonal structure of the couple matrix ($D$) into account for improved efficiency. If D is not diagonal, but the number of multiplier DoFs is small, VCP will be still beneficial. It may be expensive to compute non-diagonal D when the number of multiplier DOFs is large.

One can enable the usage of +dual basis+ by enabling `use_dual = true` in the `Variables` block:

```
[Variables]
 [./lm]
   order = FIRST
   family = LAGRANGE
   use_dual = true
 [../]
[]
```

Or, for mortar-based contact problems, the dual basis function is used +by default+ for the Lagrange multipliers (while using the contact action). An example input block for contact action is as follows:

!listing modules/contact/test/tests/mortar_tm/2d/frictionless_first/small.i block=Contact

## VCP Workflow

A schematic is included in [VCP_scheme] to demonstrate the solution process of VCP. Compared to the standard precondition and solve procedure, VCP features two additional customized computation steps (i.e., one to condense out the variable and the other to obtain the full solution vector) (see [VCP_scheme]). During static condensation of the variable (e.g., $\Delta \lambda$), the DoFs are obtained for both the variable itself ($\Delta \lambda$) and its coupled variable ($\Delta u$). Based on this information, the coupling matrices (i.e., $D$ and $M$) will be extracted from the original Jacobian matrix. Then a reduced system of equations will be obtained with the necessary submatrix operations following [eqn:condensed_global_matrix]. After solving the reduced system of equations, we obtain the primal variable solution vector, which is then utilized to update the variable $\Delta \lambda$ (see [eqn:condensation_of_LMs]) and assemble the full solution vector.

!media preconditioning/VCP_Scheme.png
    id=VCP_scheme
    caption=The variable condensation preconditioner (VCP) workflow. Blue represents the original preconditioning steps. Orange shows the additional steps customized for VCP.
    style=display:block;margin-left:auto;margin-right:auto;width:70%

### Special Designs in VCP

Several special designs in VCP foster improved performance:

- +Adaptive variable condensation+  The  idea  is  to  obtain  the  actual  DoFs  that  have  zero  diagonal entries by checking the Jacobian matrix at runtime.  This refines the DoF list such that variable condensation will only be carried out for those DoFs that actually result in a saddle point structure. For contact problems, this will make sure that variable condensation will only happen when the material bodies come into contact. One can optionally turn on/off this capability by setting the `adaptive_condensation = true` or `false`.

- +Standard basis+ As mentioned above, we recommend the usage of dual basis to reduce computational overhead due to the condensation step. However, it can happen if the dual basis does not work for certain problems. In this case, the user can set `use_dual = false` to use the standard basis functions. Then $D$ has off-diagonal entries. During the computation of ${D}^{-1}$, the user can choose to either use an LU solver (by setting `is_lm_coupling_diagonal = false`) or obtain an approximated inverse using the diagonal entries (by setting `is_lm_coupling_diagonal = true`).

- +Generalized condensation+ Although we are focused on using VCP for mortar-based contact problems with a saddle point structure, VCP is developed to be applicable to general problems in which improved conditioning is achievable via a reduced number of DoFs.

!alert note title=Generalized variable condensation is not fully tested
VCP acts on the discretized system of equations and is designed to be as general as possible. However, the current implementation is mainly tested for mortar-based mechanical contact problems. We are actively improving this implementation and are looking for applications beyond contact. Therefore, please feel free to [create an issue](https://github.com/idaholab/moose/issues) or [start a discussion](https://github.com/idaholab/moose/discussions) if you may come across any problem with VCP.

## Performance

As an illustration, we solve the 2D Hertzian problem by using VCP with several sub-preconditioner types, including BoomerAMG (AMG), successive over-relaxation (SOR), additive Schwarz method (ASM), and block Jacobi (BJAC). For all the cases, we set `adaptive_condensation = true` and `is_lm_coupling_diagonal = true`. Note that here, ASM, and BJAC converge for the mortar-based mechanical contact problem, both as a standalone preconditioner and as a sub-preconditioner. AMG and SOR stagnate as a standalone preconditioner, while converge well as a sub-preconditioner under VCP.  [hertzian_vcp_performance] shows the performance of VCP in terms of total wall time and total number of linear iterations. It can be seen that VCP can pretty efficiently converge using AMG and SOR, whereas the standard preconditioner stagnates or diverges. For the other sub-preconditioner types, VCP is still more efficient than the standard preconditioner, despite the additional computational cost from the variable condensation step. This is because a better-conditioned system is obtained after static condensation of the Lagrange multipliers. For readers who are interested in getting more details, please refer to [!citep](yushu2021m3). Note the performance of VCP for problems with increased complexities, e.g., larger, multi-physics problems, is to be examined in the future.

!media preconditioning/Hertzian_vcp_performance.png
    id=hertzian_vcp_performance
    caption=Performance of VCP for the Hertzian problem.  Different sub-preconditioner types are utilized for comparison. Results are shown for four processors. Note the “inf.” identifies the case when the solver stagnates thus the wall time and total number of iterations go to infinity.
    style=display:block;margin-left:auto;margin-right:auto;width:85%

## Example Input File Syntax

!listing modules/contact/test/tests/dual_mortar/dm_mechanical_contact_precon.i block=Preconditioning

!syntax parameters /Preconditioning/VCP

!syntax inputs /Preconditioning/VCP

!syntax children /Preconditioning/VCP
