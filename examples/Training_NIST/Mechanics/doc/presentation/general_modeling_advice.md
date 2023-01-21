# Illustration of Key Mechanics Modeling with an Example

!---

## Example

- We will analyze the case of a three-point bending beam.

- We will incrementally build be input to consider:

  - Elasticity

  - Elastoplasticity

  - Elastoplasticity + creep

- Most of the example inputs may be run with MOOSE modules and do not require
  BISON.

!---

# Three-Point Bending Solution

!row!
!col! width=50%

- Problem Statement

  - $L = 4$

  - $d = 0.5$

  - $b = 1.0$ (depth into the plane)

  - $E = 1\times10^6$ Pa

  - $W = F = 50\mathrm{N}$

!media beam_bending_prob.png

!col-end!

!col! width=50%
!col width=50%

- Solution

!equation
y_{max} = \frac{-W L^3}{3EI}

\begin{equation*}
 \begin{aligned}
   I & = \frac{1}{12}bd^3\\
     & = \frac{1}{12}(1.0m)(0.5m)^3\\
     & = 1.041667\times10^{-2}m^4
 \end{aligned}
\end{equation*}

\begin{equation*}
  \begin{aligned}
   y_{max} & = \frac{-(50\mathrm{N})
(4.0m)^3}{3(1\times10^6\mathrm{Pa})(1.041667\times10^{-2}m^4)}\\
         & = 0.1024\mathrm{m}
  \end{aligned}
\end{equation*}

!col-end!

!row-end!

!---

## [Tensor Mechanics Master Action](https://mooseframework.inl.gov/syntax/Modules/TensorMechanics/Master/index.html)

!row!
!col! width=45%

```
[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_xy
                       stress_yy strain_xx
                       strain_xy strain_yy'
  []
[]
```

$~$

$~$

- $u_x,u_y,u_z$

- $\nabla\cdot{\boldsymbol\sigma} + b = 0$

!col-end!

!col! width=10%
!col width=10%

$~$

!col-end!

!col! width=45%
!col width=45%

- The primary or dependent variables in the PDEs (temperature, displacement)
  are defined in the `Variables` block.

- A user-selected unique name is assigned for each variable.

- An action can create and/or modify any number of the blocks in an
  input file. The Tensor Mechanics master action creates several blocks, thus
  condensing the input file for a user.

!col-end!

!row-end!

!---

## Materials Block with Master Action

!row!
!col! width=90%

!listing bending_3pt_3d.i
         block=Materials


- Here, the elastic properties are specified by the `ComputeIsotropicElasticityTensor` class.

  -   $E = 1\times 10^6$
  -   $\nu = 0.3$

- Notice that the stress material property must be defined, while the strain is
  made by the tensor mechanics master action and does not appear here.



!col-end!

!col! width=5%
!col width=5%

$~$

!col-end!

!row-end!

!---

## Boundary Conditions

Need to prevent rigid body motion and load the beam.

!style! fontsize=90%

$~$

!listing bending_3pt_3d.i
         block=BCs
         style=width:95%

!style-end!

!---

## Postprocessors

Shown here are a few postprocessors to help see how a simulation is running.

!style! fontsize=90%

$~$

!listing bending_3pt_3d.i
         block=Postprocessors
         style=width:95%

!style-end!

!---

## Mesh

!media beam_3pt_meshes.png

!---

## Results (Accuracy)

- Elastic (HEX8 elements)

| Model  | # Elem | # DOF | Max y-disp | Ratio to Exact | % Improve |
| :----- | ------ | ----- | ---------- | -------------- | --------- |
| 1x1x1  | 1      | 24    | -4.099E-3  | 0.040          | ---       |
| 5x1x1  | 5      | 72    | -6.981E-2  | 0.682          | 94.1      |
| 10x3x3 | 90     | 528   | -8.336E-2  | 0.814          | 16.3      |
| 20x5x5 | 500    | 2268  | -9.630E-2  | 0.940          | 13.4      |
| 40x5x5 | 1000   | 4428  | -1.008E-1  | 0.984          | 4.5       |
| 80x7x7 | 3920   | 15552 | -1.016E-1  | 0.992          | 0.8       |

- Elastic (HEX27 elements)

| Model  | # Elem | # DOF | Max y-disp | Ratio to Exact | % Improve |
| :----- | ------ | ----- | ---------- | -------------- | --------- |
| 1x1x1  | 1      | 81    | -7.763E-2  | 0.758          | ---       |
| 5x1x1  | 5      | 297   | -9.995E-2  | 0.976          | 22.3      |
| 10x3x3 | 90     | 3087  | -1.011E-1  | 0.987          | 1.1       |
| 20x5x5 | 500    | 14883 | -1.015E-1  | 0.991          | 0.4       |

!---

## Add Plasticity

$~$

##### What do you think?

##### More or less bend?

$~$

!listing bending_3pt_3d_elas_plas.i
         block=Materials
         style=width:95%

!---

## Results (Accuracy)

- Elastic-Plastic (HEX8 elements)

| Model  | # Elem | # DOF | Max y-disp | % Improve |
| :----- | ------ | ----- | ---------- | --------- |
| 1x1x1  | 1      | 24    | -4.131E-3  | ---       |
| 5x1x1  | 5      | 72    | -7.035E-2  | 94.1      |
| 10x3x3 | 90     | 528   | -9.039E-2  | 22.2      |
| 20x5x5 | 500    | 2268  | -1.212E-1  | 25.4      |
| 40x5x5 | 1000   | 4428  | -1.334E-1  | 9.1       |
| 80x7x7 | 3920   | 15552 | -1.377E-1  | 3.1       |

- Elastic-Plastic (HEX27 elements)

| Model  | # Elem | # DOF | Max y-disp | % Improve |
| :----- | ------ | ----- | ---------- | --------- |
| 1x1x1  | 1      | 81    | -7.725E-2  | ---       |
| 5x1x1  | 5      | 297   | -1.396E-1  | 44.7      |
| 10x3x3 | 90     | 3087  | -1.349E-1  | 3.5       |
| 20x5x5 | 500    | 14883 | -1.381E-1  | 2.3       |

!---

## Add Creep

##### What do you think?

##### More or less bend?

!style! fontsize=70%

!listing bending_3pt_3d_elas_plas_creep.i
         block=Materials
         style=width:95%

!style-end!

!---

## Results (Accuracy)

- Elastic-Plastic with Creep (HEX8 elements)

| Model  | # Elem | # DOF | Max y-disp | % Improve |
| :----- | ------ | ----- | ---------- | --------- |
| 1x1x1  | 1      | 24    | -4.143E-3  | ---       |
| 5x1x1  | 5      | 72    | -7.390E-2  | 94.4      |
| 10x3x3 | 90     | 528   | -9.428E-2  | 21.6      |
| 20x5x5 | 500    | 2268  | -1.268E-1  | 25.6      |
| 40x5x5 | 1000   | 4428  | -1.398E-1  | 9.3       |
| 80x7x7 | 3920   | 15552 | -1.443E-1  | 3.1       |

- Elastic-Plastic with Creep (HEX27 elements)

| Model  | # Elem | # DOF | Max y-disp | % Improve |
| :----- | ------ | ----- | ---------- | --------- |
| 1x1x1  | 1      | 81    | -7.977E-2  | ---       |
| 5x1x1  | 5      | 297   | -1.464E-1  | 45.5      |
| 10x3x3 | 90     | 3087  | -1.412E-1  | 3.7       |
| 20x5x5 | 500    | 14883 | -1.448E-1  | 2.5       |

!---

## Additional Comments

- The analytical solution for the three-point bending problem assumes elastic material behavior.

- The purpose of adding plasticity and creep was to demonstrate the affect of these material behaviors on the response of the beam to the same loading.
