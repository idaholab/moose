# Elastic Driving Force for Grain Growth

!media phase_field/moose_pf_hex_grgr_stress.png  style=width:200px;padding-left:20px;float:right;
    caption=Elastic energy driven grain growth in a 2D hexagaonal copper polycrystal with one shrinking grain. The result was calculated using an example input file from the combined module. The input file is `hex_grain_growth_2D_eldrforce.i`





Grain boundaries (GBs) migrate to reduce the free energy of the system. One source of energy that can be reduced is the grain boundary energy, and the standard grain growth model accounts for this driving force (commonly called the curvature driving force). In addition, GBs can migrate to reduce the elastic energy stored in the system. To account for this driving force in the grain growth model, we couple to a mechanics solution.

## Model Summary

In our model, $N_{gr}$ grains are represented by $N_{op}$ order parameters, where $N_{gr} \geq N_{op}$. Each grain has a unique ID $gr$ and is represented by a specific order parameter $op$, though the order parameter used for each grain can change throughout the simulation. The assignment of the order parameters to each grain is controlled by the [GrainTracker](/GrainTracker.md) user object.

The crystal orientation of each grain is described by a set of three Euler angles $[\phi_1, \Phi, \phi_2]_{gr}$. The Euler angles define a rotation tensor $\mathbf{R}_{gr}$ used to rotate the elasticity tensor in the crystal frame of reference $\boldsymbol{\mathcal{C}}_0$ to the current frame. Thus, the elasticity tensor for each grain is fully defined by

\begin{equation}
\boldsymbol{\mathcal{C}}_{gr} = (\mathbf{R}_{gr} \boxtimes \mathbf{R}_{gr}) \boldsymbol{\mathcal{C}}_0 (\mathbf{R}_{gr} \boxtimes \mathbf{R}_{gr})^T
\end{equation}

where $\boxtimes$ is the fourth-order tensor product. In the phase field model, the grain boundaries are represented by a diffuse inteface in which multiple order parameters have non-zero values. Thus, the elasticity tensor at any point in space is a weighted average of the elasticity tensors from all grains with non-zero order parameters, i.e.

\begin{equation}
\boldsymbol{\mathcal{C}}(\mathbf{r}) = \frac{\sum_{gr} h(\eta_{gr}(\mathbf{r})) \boldsymbol{\mathcal{C}}_{gr} }{\sum_{gr} h(\eta_{gr}(\mathbf{r}))},
\end{equation}

!media phase_field/moose_pf_hex_grgr_stress_2.png  style=width:200px;padding-left:20px;float:right;
    caption=Elastic energy driven grain growth in a 2D hexagaonal copper polycrystal with one growing grain. The result was calculated using an example input file from the combined module. The input file is `hex_grain_growth_2D_eldrforce.i`

where the interpolation function $h$ is equal to 0 when $\eta_{gr} = 0$ and 1 when $\eta_{gr}=1$, i.e.

\begin{equation}
h(\eta_{gr}) = \frac{1}{2}(1 + \sin(\pi((\eta_{gr} - 0.5))
\end{equation}

The local stress is calculated from the local elasticity tensor according to

\begin{equation}
\boldsymbol{\sigma}(\mathbf{r}) = \boldsymbol{\mathcal{C}}(\mathbf{r}) \boldsymbol{\epsilon}(\mathbf{r})
\end{equation}

and the elastic energy density is calculated according to

\begin{equation}
\begin{aligned}
E_d =& \frac{1}{2} \boldsymbol{\sigma}(\mathbf{r}) \cdot \boldsymbol{\epsilon}(\mathbf{r}) \\
    =& \frac{1}{2} \boldsymbol{\mathcal{C}}(\mathbf{r}) \boldsymbol{\epsilon}(\mathbf{r}) \cdot \boldsymbol{\epsilon} (\mathbf{r})
\end{aligned}
\end{equation}

The elastic energy is added to the free energy of the system, as shown on the [Basic Phase Field Equations](Phase_Field_Equations.md) page. The Allen-Cahn equation defining the evolution of the order parameters becomes

\begin{equation}
\frac{\partial \eta_{gr}}{\partial t} = - L \left( \frac{\partial f_{loc}}{\partial \eta_j} +  \frac{\partial E_{d}}{\partial \eta_{gr}} - \kappa_j \nabla^2 \eta_j \right),
\end{equation}

where

\begin{equation}
\frac{\partial E_{d}}{\partial \eta_{gr}} = \frac{1}{2} \frac{\partial \boldsymbol{\mathcal{C}}}{\partial \eta_{gr}} \boldsymbol{\epsilon} \cdot \boldsymbol{\epsilon}.
\end{equation}

The partial derivative of the elasticity tensor with respect to the order parameters is defined by

\begin{equation}
\frac{\partial \boldsymbol{\mathcal{C}}}{\partial \eta_{gr}} = h'(\eta_{gr}) \frac{ \boldsymbol{\mathcal{C}}_{gr} }{\sum_{gr} h(\eta_{gr})} - h'(\eta_{gr}) \frac{\sum_{gr} h(\eta_{gr}) \boldsymbol{\mathcal{C}}_{gr} }{\left(\sum_{gr} h(\eta_{gr}) \right)^2}.
\end{equation}

!media phase_field/moose_pf_grgr_stress.png  style=width:200px;padding-left:20px;float:right;
    caption=Elastic energy driven grain growth in a 2D copper polycrystal. The result was calculated using an example input file from the combined module. The input file is `poly_grain_growth_2D_eldrforce.i`

## Model Implementation

The additional contribution of elastic energy on the Allen-Cahn equation describing grain growth is implemented in the phase field module with the kernel [ACGrGrElasticDrivingForce](/ACGrGrElasticDrivingForce.md). It only adds the elastic energy contribution, so it must be used with the [ACGrGrPoly](/ACGrGrPoly.md) kernel. The addition to the residual in weak form is

\begin{equation}
L \left( \frac{\partial E_d}{\partial \eta_{gr}}, \psi_m \right),
\end{equation}

and is implemented in the code according to

```cpp
Real
ACGrGrElasticDrivingForce::computeDFDOP(PFFunctionType type)
{
  // Access the heterogeneous strain calculated by the Solid Mechanics kernels
  RankTwoTensor strain(_elastic_strain[_qp]);

  // Compute the partial derivative of the stress wrt the order parameter
  RankTwoTensor D_stress = _D_elastic_tensor[_qp] * strain;

  switch (type)
  {
    case Residual:
      return 0.5 * D_stress.doubleContraction(strain); // Compute the deformation energy driving force

    case Jacobian:
      return 0.0;
  }

  mooseError("Invalid type passed in");
}
```

The stress and strain are computed using kernels from the tensor mechanics module, but a material in the phase field module was created to compute the polycrystal elasticity tensor, [ComputePolycrystalElasticityTensor](/ComputePolycrystalElasticityTensor.md). It also computes the derivatives of the elasticity tensor with respect to all of the order parameters. This material must be used with the [GrainTracker](/GrainTracker.md) user object.

As with the basic grain growth model, creating separate blocks in the input file for every order parameter would be burdensome. Thus, we have implemented an action that adds the elastic driving force kernel for each order parameter. The action is [PolycrystalElasticDrivingForceAction](/PolycrystalElasticDrivingForceAction.md) and it must be used with the [PolycrystalKernelAction](/PolycrystalKernelAction.md).

This capability is tested in

`moose/modules/combined/tests/ACGrGrElasticDrivingForce`

and is demonstrated on larger problems in

`moose/modules/combined/examples/phase_field-mechanics/hex_grain_growth_2D_eldrforce.i`

`moose/modules/combined/examples/phase_field-mechanics/poly_grain_growth_2D_eldrforce.i`
