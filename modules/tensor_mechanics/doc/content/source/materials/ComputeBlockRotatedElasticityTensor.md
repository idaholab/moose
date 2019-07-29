# Compute Block Rotated Elasticity Tensor

!syntax description /Materials/ComputeBlockRotatedElasticityTensor

## Description

The material `ComputeBlockRotatedElasticityTensor` is used for running Polycrystalline mechanics simulations
with pre-meshed grain structures where each grain is assigned with unique block ID, we need a
elasticity tensor class that provides rotated elasticity tensor taking EulerAngles from an
EulerAngleProvider based on the current block ID. `ComputeBlockRotatedElasticityTensor`
takes two constants provided by the user and a user object in the form of EulerAngleProvider and executed prior to the
first time step.

Following the design of compute rotated elasticity tensor which is used for GrainTracker based simulations,
but using `_current_elem->subdomain_id()` instead of grain ID for looking up the euler angles.

The fill method `symmetric9` is appropriate for materials with three orthotropic planes of symmetry
[citep!malvern1969introduction], and is often used for simulations of anistropic materials such as
cubic crystals.  The enginering elasticity tensor notation, [eq:rank4tensor_aux_indices],
for an orthotropic material is given in [eq:symmetric9_fill_method]
\begin{equation}
\label{eq:symmetric9_fill_method}
C_{ijkl}^{orthotropic} = \begin{bmatrix}
              C_{11} & C_{12} & C_{13} &      0 &      0 &      0 \\
              C_{12} & C_{22} & C_{23} &      0 &      0 &      0 \\
              C_{13} & C_{23} & C_{33} &      0 &      0 &      0 \\
                   0 &      0 &      0 & C_{44} &      0 &      0 \\
                   0 &      0 &      0 &      0 & C_{55} &      0 \\
                   0 &      0 &      0 &      0 &      0 & C_{66}
              \end{bmatrix}
\end{equation}

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/block_rotated_elasticity_tensor/block_rotated_elasticity_tensor_test.i block=Materials/elasticity_tensor

!syntax parameters /Materials/ComputeBlockRotatedElasticityTensor

!syntax inputs /Materials/ComputeBlockRotatedElasticityTensor

!syntax children /Materials/ComputeBlockRotatedElasticityTensor

!bibtex bibliography
