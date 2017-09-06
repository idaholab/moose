<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# CoupledForce

## Description

`CoupledForce` implements a source term proportional to a coupled variable's
value. The strong form and weak forms respectively are $$-\sigma v$$ and
$$(\psi_i, -\sigma v)$$ In a species transport context, the value $\sigma$ can
be regarded as a reaction rate coefficient. The corresponding Jacobian is
$$(\psi_i, -\sigma \phi_j)$$

## Example Syntax

The kernel block below shows a variable $u$ that is diffusing and being produced
at a rate proportional to the concentration of a variable $v$ which is also
diffusing.

!listing test/tests/bcs/coupled_dirichlet_bc/coupled_dirichlet_bc.i
block=Kernels label=false

Note that in this example the input parameter `coef` that corresponds to
$\sigma$ is omitted. In this case its value defaults to $1$. The `CoupledForce`
block could be modified to use a different value for $\sigma$ as shown below:

```
  [./coupled_force_u]
    type = 'CoupledForce'
    variable = 'u'
    v = 'v'
    coef = 5
  [../]
```

!syntax parameters /Kernels/CoupledForce

!syntax inputs /Kernels/CoupledForce

!syntax children /Kernels/CoupledForce
