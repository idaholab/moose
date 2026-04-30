# InterfaceNormalCurvatures

!syntax description /Materials/InterfaceNormalCurvatures

## Overview

`InterfaceNormalCurvatures` is a `Material` object that computes two normal curvatures of a diffuse interface defined by an order parameter $\eta$.

The two normal curvatures characterize how the interface bends in two orthogonal directions tangent to the surface defined by the level set of $\eta$.

| Property | Symbol | Direction |
|---|---|---|
| `kappa1` | $\kappa_1$ | In-plane tangent $\hat{t}_1$ ($\hat{t}_1$ is chosen to lie in the $xy$-plane) |
| `kappa2` | $\kappa_2$ | Out-of-plane tangent $\hat{t}_2 = \hat{n} \times \hat{t}_1$ |

As a consistency check, their mean equals the mean curvature: $\frac{1}{2} \left(\kappa_1 + \kappa_2 \right) = \kappa_{mean} = \frac{1}{2} \left( \nabla \cdot \hat{n} \right)$.

---

## Theory

### Interface Normal

The unit normal to the diffuse interface is defined from the order parameter gradient:

\begin{equation}
  \hat{n} = \frac{\nabla\eta}{|\nabla\eta|}
  \label{eq:normal}
\end{equation}

This is well-defined in the interfacial region where $|\nabla\eta| \neq 0$. To prevent unreliable calculations when $|\nabla\eta| \approx 0$, curvatures are set to zero when $|\nabla\eta|$ is less than the user-settable input parameter `gradient_threshold`.

### Local Tangent Frame

A right-handed orthonormal frame $\{\hat{n},\, \hat{t}_1,\, \hat{t}_2\}$ is constructed at every quadrature point.

The first tangent vector $\hat{t}_1$ is chosen to lie in the $xy$-plane by taking the cross product with $\hat{z}$:

\begin{equation}
  \hat{t}_1 = \frac{\hat{z} \times \hat{n}}{|\hat{z} \times \hat{n}|}
  \label{eq:t1}
\end{equation}

When $\hat{n} \approx \pm\hat{z}$ the cross product degenerates; in this case $\hat{t}_1$ falls back to $\hat{x}$ so that the frame remains well-defined everywhere.

The second tangent vector (the binormal) is then:

\begin{equation}
  \hat{t}_2 = \hat{n} \times \hat{t}_1
  \label{eq:t2}
\end{equation}

which is automatically unit length and orthogonal to both $\hat{n}$ and $\hat{t}_1$.

### Shape Operator

The curvature of the interface is encoded in the shape operator (Weingarten map), defined as the negative surface gradient of the unit normal:

\begin{equation}
  \mathbf{S} = -\nabla\hat{n}
\end{equation}

Differentiating [eq:normal] with the quotient rule gives the full Jacobian of $\hat{n}$:

\begin{equation}
  \frac{\partial \hat{n}_i}{\partial x_j}
  = \frac{1}{|\nabla\eta|}\left( H_{ij} - \hat{n}_i \sum_k H_{kj}\,\hat{n}_k \right)
  \label{eq:jacobian}
\end{equation}

where $H_{ij} = \partial^2\eta / \partial x_i \partial x_j$ is the Hessian of $\eta$.

In compact tensor notation:

\begin{equation}
  \nabla\hat{n} = \frac{1}{|\nabla\eta|}\Bigl(\mathbf{H} - \hat{n} \otimes (\mathbf{H}\hat{n})\Bigr)
\end{equation}

### Normal Curvature

The normal curvature in a direction $\hat{v}$ tangent to the interface is:

\begin{equation}
  \kappa(\hat{v}) = \hat{v} \cdot \mathbf{S} \cdot \hat{v} = -\hat{v} \cdot (\nabla\hat{n}) \cdot \hat{v}
\end{equation}

Substituting [eq:jacobian] and using $\hat{v} \cdot \hat{n} = 0$ (since $\hat{v}$ is tangent), the cross-term vanishes identically and the expression simplifies to:

\begin{equation}
  \boxed{
    \kappa(\hat{v}) = -\frac{\hat{v} \cdot \mathbf{H} \cdot \hat{v}}{|\nabla\eta|}
  }
  \label{eq:kappa}
\end{equation}

This is the key formula implemented at each quadrature point. The two scalar outputs are:

\begin{equation}
  \kappa_1 = -\frac{\hat{t}_1 \cdot \mathbf{H} \cdot \hat{t}_1}{|\nabla\eta|}, \qquad
  \kappa_2 = -\frac{\hat{t}_2 \cdot \mathbf{H} \cdot \hat{t}_2}{|\nabla\eta|}
\end{equation}

### Mean Curvature (Diagnostic)

The mean curvature $\kappa_{mean} = \frac{1}{2} \left( \nabla \cdot \hat{n} \right)$ is also declared as a material property for verification. It equals the trace of the shape operator:

\begin{equation}
  \kappa_{mean} = \frac{1}{2} (\kappa_1 + \kappa_2) = -\frac{1}{2} \left( \frac{\mathrm{tr}(\mathbf{H}) - \hat{n} \cdot \mathbf{H} \cdot \hat{n}}{|\nabla\eta|} \right)
\end{equation}

---

## Material Properties Declared

| Name | Type | Description |
|---|---|---|
| `kappa1` | `Real` | Normal curvature $\kappa_1$ along the tangent $\hat{t}_1$ (lies in $xy$-plane) |
| `kappa2` | `Real` | Normal curvature $\kappa_2$ along the out-of-plane tangent $\hat{t}_2$ |
| `kappa_mean` | `Real` | Mean curvature $\kappa_{mean} =  \frac{1}{2} (\kappa_1 + \kappa_2) =  \frac{1}{2} \left( \nabla\cdot\hat{n} \right)$ |

All property names can be prefixed by setting `base_name`.

---

## Requirements

This object requires the second spatial derivatives of $\eta$ to be available at quadrature points. This is only possible when:

- $\eta$ uses second-order (or higher) Lagrange shape functions (family `LAGRANGE`, order `SECOND`), or
- $\eta$ uses a $C^1$-continuous basis (e.g. Hermite).

First-order elements will yield zero Hessian values and should not be used with this material.

---

## Example Input Syntax

!listing test/tests/misc/interface_normal_curvatures.i
         block=Materials/curvatures

A minimal `[Materials]` block:

```
[Materials]
  [curvature]
    type = InterfaceNormalCurvatures
    eta = eta
    regularization = 1e-8
    base_name = ''
  []
[]
```

The curvature properties can be visualised by coupling them to `MaterialRealAux` kernels:

```
[AuxVariables]
  [kappa1]  [../]
  [kappa2]  [../]
[]

[AuxKernels]
  [kappa1_aux]
    type = MaterialRealAux
    variable = kappa1
    property = kappa1
    execute_on = TIMESTEP_END
  []
  [kappa2_aux]
    type = MaterialRealAux
    variable = kappa2
    property = kappa2
    execute_on = TIMESTEP_END
  []
[]
```

---

## Parameters

!syntax parameters /Materials/InterfaceNormalCurvatures

## Input Files

!syntax inputs /Materials/InterfaceNormalCurvatures

## Children Objects

!syntax children /Materials/InterfaceNormalCurvatures

---

!bibtex bibliography
