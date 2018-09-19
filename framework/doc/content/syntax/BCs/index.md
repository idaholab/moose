<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# BCs System

## Boundary Condition

- A `BoundaryCondition` provides a residual (and optionally a Jacobian) on a boundary (or internal side) of a domain.
- The structure is very similar to Kernels

  - `computeQpResidual`/`Jacobian()`
  - Parameters
  - Coupling

- The only difference is that some BCs are NOT integrated over the boundary... and instead specify values on Boundaries (Dirichlet).
- BCs which are integrated over the boundary inherit from `IntegratedBC`.
- Non-integrated BCs inherit from `NodalBC`.

## (Some) Values Available to BCs

Integrated BCs:

- `_u, _grad_u` : Value and gradient of the variable this BC is operating on.
- `_phi, _grad_phi` : Value ($\phi$) and gradient ($\nabla \phi$) of the trial function.
- `_test,_grad_test` : Value ($\psi$) and gradient ($\nabla \psi$) of the test function.
- `_q_point` : XYZ coordinates
- `_i, _j` : test and trial shape function indices.
- `_qp` : Current quadrature point index.
- `_normals` : Normal vector.
- `_boundary_id` : The boundary ID.
- `_current_elem` : A pointer to the element.
- `_current_side` : The side number of `_current_elem`

Non-integrated BCs:

- `_u`
- `_qp`
- `_boundary_id`
- `_current_node` : A pointer to the current node that is being operated on.

## Coupling and BCs

The coupling of values and gradients into BCs is done the same way as in Kernels and materials :

- `coupledValue()`
- `coupledValueOld()`
- `coupledValueOlder()`
- `coupledGradient()`
- `coupledGradientOld()`
- `coupledGradientOlder()`
- `coupledDot()`

## Dirichlet BCs

- Set a condition on the `value` of a variable on a boundary.
- Usually... these are NOT integrated over the boundary.

  \begin{equation*}
  u = g_1 \quad \mathrm{on} \quad \partial \Omega_1
  \end{equation*}

  Becomes:
  \begin{equation*}
  u - g_1 = 0 \quad \mathrm{on} \quad \partial \Omega_1
  \end{equation*}

- In the following example:

  \begin{equation*}
  u = \alpha v \quad \mathrm{on} \quad \partial \Omega_2
  \end{equation*}

  And therefore:
  \begin{equation*}
  u - \alpha v = 0 \quad \mathrm{on} \quad \partial \Omega_2
  \end{equation*}

## Integrated BCs

- Integrated BCs (including Neumann BCs) are actually integrated over the external face of an element.
- Their residuals lokk similar to kernels. Thus

$\left\{
   \begin{array}{rl}
     (\nabla u, \nabla \psi_i) - (f, \psi_i) - \langle \nabla u\cdot \hat{\boldsymbol n}, \psi_i\rangle &= 0 \quad \forall i
    \\  
      \nabla u \cdot \hat{\boldsymbol n} &= g_1\quad \text{on} \quad\partial\Omega
   \end{array}
\right.$

Becomes:

$(\nabla u, \nabla \psi_i) - (f, \psi_i) - \langle g_1, \psi_i\rangle = 0 \quad \forall i$

- Also note that if $\nabla u \cdot \hat{n} = 0$, then the boundary integral is zero (sometimes known os the "natural boundary condition").

## Example 4

Look at [Example 4](ex04_bcs.md)

## Periodic BCs

- Periodic boundary conditions are useful for modeling quasi-infinite domains and systems with conserved quantities.
- MOOSE has full support for periodic BCs

  - 1D, 2D and 3D.
  - With mesh adaptivity.
  - Can be restricted to specific variables.
  - Supports arbitrary translation vectors for defining periodicity.

```puppet
[BCs]
  [./Periodic]
    [./all]
      variable = u 

      #Works for any regular orthogonal
      #mesh with defined boundaries
      auto_direction = 'x y'
    [../]
  [../]
[]
```

```puppet
[BCs]
  [./Periodic]
    [./x]
      primary = 1 
      secondary = 4 
      transform_func = 'tr_x tr_y'
      inv_transform_func = 'itr_x itr_y'
    [../]
  [../]
[]
```

- Normal usage: with an axis-aligned mesh, use `auto-direction` to supply the coordinate direction to wrap.
- Advanced usage: specify a translation or transformation function.

```puppet
[BCs]
  [./Periodic]
    [./x]
      variable = u 
      primary = 'left'
      secondary = 'right'
      translation = '10 0 0'
    [../]
  [../]
[]
```

## Periodic Example

Look at [Example 4](ex04_bcs.md) Periodic Example

## Further BCs documentation

!syntax list /BCs objects=True actions=False subsystems=False

!syntax list /BCs objects=False actions=False subsystems=True

!syntax list /BCs objects=False actions=True subsystems=False

