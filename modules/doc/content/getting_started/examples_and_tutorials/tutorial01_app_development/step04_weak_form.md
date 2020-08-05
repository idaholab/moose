# Step 4: Generating a Weak Form

The first question to ask when presented with a [!ac](PDE) that they wish to use for modeling purposes is: "How do I solve this equation?" The MOOSE answer to this question is to use [Galerkin's Method](#galerkin), which involves expressing the *strong form* of a governing [!ac](PDE) in a *weak form*. In [Step 3](tutorial01_app_development/step03_moose_object.md#physics), it was mentioned that the weak form of the Laplace Equation, subject to the natural boundary condition, $\nabla u \cdot \hat{n} = 0$, was the following:

!equation id=laplace-weak
\int_{\Omega} \nabla \psi \cdot \nabla u \, d\Omega = 0, \,\,\, \forall \, \psi

In this step, the relevance of the so-called "weak form" of a [!ac](PDE) as it pertains to creating an application and how to derive it from the equation's strong form will be discussed. The diffusion equation of the previous two steps shall be revisited, but with a few more terms added for demonstration purposes. In addition, the reader shall begin to consider the next phase of the tutorial by deriving the weak form of Darcy's pressure equation given on the [Problem Statement](tutorial01_app_development/problem_statement.md#equations) page.

The [!ac](FEM) is a method for solving differential equations. To understand the [!ac](FEM) in a broader sense, it is recommended that the reader visit the [help/faq/what_is_fem.md] page.

!alert warning
This step is not intended to provide a complete lesson in [!ac](FEM), it is designed to provide an overview of the method for implementing
new physics in a MOOSE-base application. For a complete introduction to [!ac](FEM) principals please refer to the myriad of books on the subject.

## Statement of Physics id=physics

Consider the advection-diffusion of a fluid within some domain, $\Omega$, subject to a forcing function, $f$. The strong form of the governing [!ac](PDE) is

!equation id=advection-strong
-\nabla \cdot k \nabla u + \bar{\beta} \cdot \nabla u = f \in \Omega

The exact meaning of the terms, $k$ and $\bar{\beta}$, in [advection-strong] are arbitrary---just assume that their values are known and that they may be continuously differentiable on $\Omega$ or constant throughout.

Also, consider the zero-gravity, divergence-free form of Darcy's Pressure law, as it was given on the [Problem Statement](tutorial01_app_development/problem_statement.md#equations) page, over the domain, $\Omega$, enclosed by the pipe between the two pressure vessels:

!equation id=darcy-strong
-\nabla \cdot \dfrac{\mathbf{K}}{\mu} \nabla p = 0 \in \Omega

A solution to [darcy-strong] is not yet of interest for this step of the tutorial. The goal here is to derive an expression for its weak form to use later on.

The solutions of these equations are assumed to be well approximated by a finite sum of test functions, $\psi_{i}$. The method involves rearranging the strong form of a [!ac](PDE) so that all terms are on one side of the equation, with zero on the other, and then multiplying by all $\psi$. For [advection-strong],
the [!ac](MWR) is an attempt to find $u$, such that

!equation id=advection-weak
(-\nabla \cdot k \nabla u + \bar{\beta} \cdot \nabla u - f) \psi_{i} = 0, \,\,\, \forall \, \psi_{i}

is satisfied. Technically, [advection-weak] is the weak form of [advection-strong], but there is more work to be done before it can be developed as a [!ac](MOOSE) object.

In general, the [!ac](FEM) assumes that solutions to [!ac](PDEs) take on simple forms, such as a polynomial. This assumption enables difficult, analytical derivative expressions to be directly integrated in a piecewise-continuous fashion. There are many approaches to achieve solutions using the [!ac](FEM), but the Galerkin approach is particularly useful for multiphysics applications, because it is a purely numerical one whose efficacy is independent of the underlying physics.

+The Galerkin [!ac](FEM) uses the same functions to approximate the primary variable of a [!ac](PDE) for all of the test functions, $\boldsymbol{\psi_{i}}$.+ Later in this tutorial, their will be a discussion on polynomial fitting and *shape functions*, and how these are used to interpolate a continuously differentiable variable. For now, simply acknowledge that shape functions form the basis for the approximate solutions obtained from the [!ac](FE) analysis, and that the test functions are those same functions.

## Weak Forms id=weak

There is a basic procedure for expressing a weak form of a [!ac](PDE), like [advection-weak],
as one that is readily solved with MOOSE. After multiplying the residual expression by $\psi$, integrate with respect to $\Omega$ and apply the integrand sum rule, e.g.,

!equation id=advection-sum
-\int_\Omega \psi (\nabla \cdot k \nabla u) + \int_\Omega \psi (\vec{\beta} \cdot \nabla u) - \int_\Omega \psi f = 0

Next, recall that the product rule of differentiation implies that

!equation id=advection-product
\psi (\nabla \cdot k \nabla u) = \nabla \cdot (\psi (k \nabla u)) - \nabla \psi \cdot k \nabla u

Substituting the right-hand side of [advection-product] results in integration by parts of the first integrand in [advection-sum], i.e.,

!equation id=advection-ibp
\int_\Omega \nabla \psi \cdot k \nabla u - \int_\Omega \nabla \cdot (\psi (k \nabla u)) + \int_\Omega \psi (\vec{\beta} \cdot \nabla u) - \int_\Omega \psi f = 0

Finally, notice that the second term in [advection-ibp] is a volume integral whose integrand is the divergence of the field, $\psi (k \nabla u)$. Therefore, it is possible to apply the divergence theorem and integrate the magnitude of this field along the volume's surface normal, $\hat{n}$, with respect to that surface's domain, $\Gamma = \partial \Omega$, to obtain the final expression for the weak form.

!equation id=advection-final
\int_\Omega \nabla \psi \cdot k \nabla u - \oint_\Gamma \psi(k \nabla u \cdot \hat{n}) + \int_\Omega \psi (\vec{\beta} \cdot \nabla u) - \int_\Omega \psi f = 0

From an application developer perspective, it is convenient to express [advection-final] using *inner product notation*, and to identify the [!ac](MOOSE) class that each term in the residual shall inherit from.

!equation id=advection-objects
\underbrace{(\nabla \psi, k \nabla u)}_{Kernel} - \underbrace{\langle \psi, k \nabla u \cdot \hat{n} \rangle}_{Boundary \, Condition} + \underbrace{(\psi, \vec{\beta} \cdot \nabla u)}_{Kernel} - \underbrace{(\psi, f)}_{Kernel} = 0

When using inner product notation, parenthesis denotes a volume integral and the angled brackets represents a surface integral. To be more clear, the terms enclosed form an inner product or, sometimes, a scalar one, and this represents the integrand.

The weak form expressed using inner product notation closely resembles the syntax used for the [!ac](MOOSE) object computing the residual contribution for each term. Later in this tutorial, the `Kernels` and `BCs` classes will be discussed in greater detail. Here, it should be clear that the kernel terms in [advection-objects] directly contribute to the residual on $\Omega$, while the boundary condition term contributes, indirectly, as a flux through $\Gamma$. This is part of the reason why the two classes are distinct. Also, it should now be evident why the boundary condition term was omitted from [laplace-weak], this term was taken as zero as part of the [!ac](BVP) used in [Step 2](tutorial01_app_development/step02_input_file.md#physics) and [Step 3](tutorial01_app_development/step03_moose_object.md#physics). Still, there was also Dirichlet type boundary conditions enforced at the inlet and the outlet of the pipe, but these types of boundary conditions override the residual at the specified boundary entirely, and do not reveal themselves in the weak form expressions until explicitly added to them.

### Procedure for Generating Weak Forms id=procedure

The general procedure for expressing the weak form of a [!ac](PDE) is as follows:

1. Write down the strong form of the equation.
2. Rearrange terms so that all are on one side of the equals sign, with zero on the other.
3. Multiply the whole equation by a test function, $\psi$.
4. Integrate the whole equation over the domain, $\Omega$, and apply the integrand sum rule to separate the terms.
5. Integrate by parts (use the product of rule of differentiation) any divergence, or higher-order derivatives, of the primary variable multiplied by $\psi$. Repeat this step until the appropriate order of the differential functions is achieved.
6. Apply the divergence theorem to generate boundary integrals, if necessary and/or possible.
7. Express the final weak form using inner product notation, if desired.

## Demonstration id=demo

The [#procedure] shall now be applied to [darcy-strong]. The result of each step of the procedure will be provided here, but the reader should make their own attempt and then verify their conclusions.

1. Write down the strong form:

   !equation
   -\nabla \cdot \dfrac{\mathbf{K}}{\mu} \nabla p = 0 \in \Omega

2. Rearrange terms: (omit)
3. Multiply the equation by $\psi$:

   !equation
   (-\nabla \cdot \dfrac{\mathbf{K}}{\mu} \nabla p) \psi = 0, \,\,\, \forall \, \psi

4. Integrate the equation over the domain, $\Omega$:

   !equation
   -\int_{\Omega} \psi (\nabla \cdot \dfrac{\mathbf{K}}{\mu} \nabla p) = 0

5. Integrate by parts:

   !equation
   \int_{\Omega} \nabla \psi \cdot \dfrac{\mathbf{K}}{\mu} \nabla p - \int_{\Omega} \nabla \cdot (\psi (\dfrac{\mathbf{K}}{\mu} \nabla p)) = 0

6. Apply the divergence theorem:

   !equation
   \int_{\Omega} \nabla \psi \cdot \dfrac{\mathbf{K}}{\mu} \nabla p - \oint_{\Gamma} \psi (\dfrac{\mathbf{K}}{\mu} \nabla p \cdot \hat{n}) = 0

7. Express the final weak form using inner product notation.

   !equation
   \underbrace{(\nabla \psi, \dfrac{\mathbf{K}}{\mu} \nabla p)}_{Kernel} - \underbrace{\langle \psi, \dfrac{\mathbf{K}}{\mu} \nabla p \cdot \hat{n} \rangle}_{Boundary \, Condition} = 0

   It is important to become comfortable with this notation for weak form expressions, as it shall henceforth be used exclusively in this tutorial.

!content pagination previous=tutorial01_app_development/step03_moose_object.md
                    next=tutorial01_app_development/step05_kernel_object.md
