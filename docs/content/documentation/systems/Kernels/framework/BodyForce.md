<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# BodyForce

## Description

`BodyForce` implements a force term in momentum transport or structural
mechanics or a source term in species/mass transport. Its weak form looks like
$$(\psi_i, -f)$$ where $f$ is the magnitude of the force or source. The force is
constructed through a user supplied constant,
[function](systems/Functions/index.md), and/or
[postprocessor](systems/Postprocessors/index.md). The constant, supplied through
the parameter `value`, may also be controlled over the course of a transient
simulation with a [`Controls`](systems/Controls/index.md) block.

## Example Syntax

The case below demonstrates the use of `BodyForce` where the force term is
supplied through a postprocessor:

!listing test/tests/kernels/2d_diffusion/2d_diffusion_bodyforce_test.i
 block=Kernels label=false

 Since `value` and `function` were not supplied, they default to 1 and 1. The
 corresponding `Postprocessor` block is

 !listing test/tests/kernels/2d_diffusion/2d_diffusion_bodyforce_test.i
 block=Postprocessors label=false

 Note that in this test the postprocessor value is actually calculated from a
 function.

!syntax description /Kernels/BodyForce

!syntax parameters /Kernels/BodyForce

!syntax inputs /Kernels/BodyForce

!syntax children /Kernels/BodyForce
