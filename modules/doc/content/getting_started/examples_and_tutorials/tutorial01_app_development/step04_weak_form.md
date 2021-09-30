!content pagination previous=tutorial01_app_development/step03_moose_object.md
                    next=tutorial01_app_development/step05_kernel_object.md
                    margin-bottom=0px

# Step 4: Generate a Weak Form

The first question to ask when presented with a [!ac](PDE) that governs a problem's physics is: "How do I solve this equation?" The MOOSE answer to this question is to use [Galerkin's Method](#galerkin), which involves expressing the *strong form* of a governing [!ac](PDE) in its *weak form*.

In this step, the relevance of the so-called "weak form" of a [!ac](PDE) as it pertains to creating an application and how to derive it from the equation's strong form will be discussed. The diffusion equation of the previous two steps shall be revisited. In addition, the reader shall begin to consider the next phase of the tutorial by deriving the weak form of Darcy's pressure equation, which was given on the [Problem Statement](tutorial01_app_development/problem_statement.md#equations) page.

!alert warning title=This is not an introduction to the Finite Element Method.
For a complete introduction to the [!ac](FEM), please refer to the myriad of literature on it.
The information presented here introduces the concepts as simply as possible for the purpose of
demonstrating the development of a MOOSE-base application. Please visit the [help/finite_element_concepts/index.md] page for more information.

## The Method of Weighted Residuals id=residuals

[The Method of Mean Weighted Residuals](https://en.wikipedia.org/wiki/Method_of_mean_weighted_residuals) assumes that the solutions to [!ac](PDEs) are well approximated by a finite sum of test functions, which are denoted by $\psi$. It involves rearranging the strong form of a [!ac](PDE) so that all terms are on one side of the equation, with zero on the other, and then multiplying by all $\psi_{i}$. For the Laplace equation, this method is an attempt to find $u$ such that

!equation id=laplace-residual
\psi (-\nabla \cdot \nabla u) = 0, \enspace \forall \, \psi

The left-hand side of [!eqref](laplace-residual) is called the *residual* and the coefficient $\psi$ is arbitrary, except for requiring that it satisfy homogeneous boundary conditions.

There is a general procedure for expressing a residual as one that is readily solved with MOOSE. First, integrate with respect to the problem domain $\Omega$:

!equation id=laplace-sum
-\int_\Omega \psi (\nabla \cdot \nabla u) = 0

Next, recall that the product rule of differentiation implies that

!equation id=laplace-product
\psi (\nabla \cdot \nabla u) = \nabla \cdot (\psi \nabla u) - \nabla \psi \cdot \nabla u

Substituting the right-hand side of [!eqref](laplace-product) results in integration by parts of [!eqref](laplace-sum):

!equation id=laplace-ibp
\int_\Omega \nabla \psi \cdot \nabla u - \int_\Omega \nabla \cdot (\psi \nabla u) = 0

Finally, notice that the second term in [!eqref](laplace-ibp) is a volume integral whose integrand is the divergence of the field $\psi \nabla u$. Therefore, it is possible to apply the [divergence theorem](https://en.wikipedia.org/wiki/Divergence_theorem) and integrate the magnitude of this field along the volume's surface normal $\hat{n}$ with respect to that surface's domain $\Gamma = \partial \Omega$, i.e.,

!equation id=laplace-weak
\int_\Omega \nabla \psi \cdot \nabla u - \oint_\Gamma \psi \nabla u \cdot \hat{n} = 0

+This is the weak form of the Laplace equation.+ From an application developer's perspective, it is convenient to express a weak form using *inner product notation*, and to identify the MOOSE class that each residual term shall inherit from:

!equation id=laplace-objects
\underbrace{(\nabla \psi, \nabla u)}_{\clap{Kernel}} - \underbrace{\langle \psi, \nabla u \cdot \hat{n} \rangle}_{\clap{Boundary \, Condition}} = 0

### Procedure for Generating Weak Forms id=procedure

The general procedure for expressing the weak form of a [!ac](PDE) is as follows:

1. Write down the strong form of the equation.
2. Rearrange terms so that all are on one side of the equals sign, with zero on the other.
3. Multiply the whole equation by a test function $\psi$.
4. Integrate the whole equation over the domain $\Omega$ and apply the integrand sum rule to separate the terms.
5. Integrate by parts (use the product of rule of differentiation) any divergence, or higher-order derivatives, of the primary variable multiplied by $\psi$. Repeat this step until the appropriate order of the differential functions is achieved.
6. Apply the divergence theorem to generate boundary integrals, if necessary and/or possible.
7. Express the final weak form using inner product notation, if desired.

## The Galerkin Finite Element Method id=galerkin

In general, the [!ac](FEM) assumes that solutions to [!ac](PDEs) take on simple analytical forms called [shape functions](source/problems/FEProblemBase.md#shape_functions), e.g., a polynomial. This assumption enables difficult analytical derivative expressions to be directly integrated in a piecewise-continuous fashion. +The Galerkin [!ac](FEM) is one that uses the same functions to approximate the solution of a [!ac](PDE) as those used for test functions $\boldsymbol{\psi}$.+ There are many approaches to achieve solutions using the [!ac](FEM), but the Galerkin approach is particularly useful for multiphysics applications, because it is a purely numerical one whose effectiveness is independent of the underlying physics.

## Demonstration id=demo

Consider the zero-gravity, divergence-free form of Darcy's Pressure law, as it was given on the [Problem Statement](tutorial01_app_development/problem_statement.md#equations) page, over the domain $\Omega$ enclosed by the pipe between the two pressure vessels:

!equation id=darcy-strong
-\nabla \cdot \dfrac{\mathbf{K}}{\mu} \nabla p = 0 \in \Omega

The [#procedure] shall now be applied to this equation. The result of each step will be provided here, but the reader should make their own attempt and then verify their conclusions.

1. Write down the strong form: [!eqref](darcy-strong)
2. Rearrange terms: *done*
3. Multiply the equation by $\psi$:

   !equation
   (-\nabla \cdot \dfrac{\mathbf{K}}{\mu} \nabla p) \psi = 0, \enspace \forall \, \psi

4. Integrate the equation over $\Omega$:

   !equation
   -\int_{\Omega} \psi (\nabla \cdot \dfrac{\mathbf{K}}{\mu} \nabla p) = 0

5. Integrate by parts:

   !equation
   \int_{\Omega} \nabla \psi \cdot \dfrac{\mathbf{K}}{\mu} \nabla p - \int_{\Omega} \nabla \cdot (\psi (\dfrac{\mathbf{K}}{\mu} \nabla p)) = 0

6. Apply the divergence theorem:

   !equation
   \int_{\Omega} \nabla \psi \cdot \dfrac{\mathbf{K}}{\mu} \nabla p - \oint_{\Gamma} \psi (\dfrac{\mathbf{K}}{\mu} \nabla p \cdot \hat{n}) = 0

7. Express the final weak form using inner product notation:

   !equation
   \underbrace{(\nabla \psi, \dfrac{\mathbf{K}}{\mu} \nabla p)}_{\clap{\clap{Kernel}}} - \underbrace{\langle \psi, \dfrac{\mathbf{K}}{\mu} \nabla p \cdot \hat{n} \rangle}_{\clap{Boundary \, Condition}} = 0

   It is important to become comfortable with this notation for weak form expressions as it shall henceforth be used exclusively.

!content pagination previous=tutorial01_app_development/step03_moose_object.md
                    next=tutorial01_app_development/step05_kernel_object.md
