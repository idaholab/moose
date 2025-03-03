# Step 2: Simple Heat Conduction Kernel id=step02

!---

## Implementing equations in MOOSE

To implement the Heat Conduction equation, we will use a `Kernel`  object to compute the
residual and the Jacobian of the diffusion term:

!equation
-\nabla \cdot k \nabla T = 0,

where  $k$  is the thermal diffusivity.

A `Kernel`  is C++ class, which inherits from `MooseObject` that is used by MOOSE for coding
volume integrals of a [!ac](PDE).

!!end-intro

!---

## CoefDiffusion Kernel

To implement the coefficient a new Kernel object must be used: `CoefDiffusion`.

This object inherits from `Diffusion` and will use input parameters for specifying the
diffusivity.

!---

## Step 2: Input File

We introduce block restriction to differentiate between water and concrete.
Block restriction is a common feature of MOOSE objects, introduced by setting

```text
block = 'concrete'
```

!listing step02_coef_diffusion/step2.i

!---

## Step 2: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step02_coef_diffusion
moose-opt -i step2.i
```


!---

## Step 2: Result

!media shield_multiphysics/results/step02.png
