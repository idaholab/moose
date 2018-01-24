# Compute Smeared Cracking Stress
!syntax description /Materials/ComputeSmearedCrackingStress

## Description
This class implements a fixed smeared cracking model, which represents cracking as a softening stress-strain law at the material points as opposed to introducing topographic changes to the mesh, as would be the case with a discrete cracking model.

In this model, principal stresses are compared to a critical stress.  If one of the principal stresses exceeds the critical stress, the material point is considered cracked in that direction, and the model transitions to an orthotropic model, in which the stress in the cracked direction is decreased according to a softening law. Material behavior in the cracking direction is affected in two ways: reduction of the stiffness in that direction, and adjusting the stress to follow the softening curve.

This class derives from [ComputeMultipleInelasticStrain](ComputeMultipleInelasticStress.md), and prior to cracking, allows multiple inelastic models to be active. Once cracking occurs, the inelastic strains at that material point are preserved, but those models are no longer called for the duration of the simulation, and inelastic strains from those other models are no longer permitted to evolve.

The orientation of the principal coordinate system is determined from the eigenvectors of the elastic strain tensor.  However, once a crack direction is determined, that direction remains fixed and further cracks are considered in directions perpendicular to the original crack direction.  Note that for axisymmetric problems, one crack direction is known _a priori_.  The theta or out-of-plane direction is not coupled to the $r$ and $z$ directions (i.e., no $r\theta$ or $z\theta$ shear strain/stress exists) and is therefore a known or principal direction.

If we store a scalar value, $c_i$, for each of the three possible crack directions at a material point, these in combination with the principal directions (eigenvectors or rotation tensor) provide a convenient way to eliminate stress in cracked directions.  A value of 1 for $c_i$ indicates that the material point has not cracked in that direction.  A value very close to zero (not zero for numerical reasons) indicates that cracking has occurred.

We define a cracking tensor in the cracked orientation as $\mathbf{c}$:
\begin{equation}
\mathbf{c}=
\begin{bmatrix}
c_1 & & \\
& c_2 & \\
& & c_3
\end{bmatrix}.
\end{equation}
The rotation tensor $\mathbf{R}$ is defined in terms of the eigenvectors $e_i$:
\begin{equation}
\mathbf{R}=
\begin{bmatrix}
e_1 & e_2 & & e_3
\end{bmatrix}.
\end{equation}
This leads to a transformation operator $\mathbf{T}$:
\begin{equation}
\mathbf{T}=\mathbf{R}\mathbf{c}\mathbf{R}^T.
\end{equation}

$\mathbf{T}$ is useful for transforming uncracked tensors in the global frame to cracked tensors in the same frame.  For example, the cracked stress $\mathbf{\sigma}_{cg}$ in terms of the stress $\mathbf{\sigma}_g$ is (subscript $c$ indicates cracked, $l$ local frame, and $g$ global frame):
\begin{equation}
\begin{aligned}
\mathbf{\sigma}_{cg} &= \mathbf{T}\mathbf{\sigma}_g\mathbf{T}^T \\
&= \mathbf{RcR}^T\mathbf{\sigma}_g\mathbf{RcR}^T \\
&= \mathbf{Rc}\mathbf{\sigma}_l\mathbf{cR}^T \\
&= \mathbf{R}\mathbf{\sigma}_{cl}\mathbf{R}^T.
\end{aligned}
\end{equation}

When many material points have multiple cracks, the solution becomes difficult to obtain numerically.  For this reason, controls are available to limit the number and direction of cracks that are allowed. Also, there are options to control the amount of shear retention and amount of stress correction during softening, both of which can significantly affect convergence.

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/smeared_cracking/cracking.i block=Materials/elastic_stress

!syntax parameters /Materials/ComputeSmearedCrackingStress

!syntax inputs /Materials/ComputeSmearedCrackingStress

!syntax children /Materials/ComputeSmearedCrackingStress
