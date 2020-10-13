# What is the Finite Element Method? id=FEM?

The [!ac](FEM) is a means for solving ordinary or, more often,
[!ac](PDEs), which are continuous over a given domain. These equations are often difficult and, sometimes, even impossible to evaluate analytically, and the geometry of their domains may be of any arbitrary, complex shape. The [!ac](FEM) closely approximates the solution and in certain cases, it may produce an exact solution.

In the [!ac](FEM), solutions to [!ac](PDEs) are assumed to take on a certain form. Problem domains are discretized into an irregular (or regular) grid, known as a mesh, and the assumed solution functions (interpolations) are evaluated at special points on that grid space. As with any approximation to some real function, the more discrete points at which evaluations are made, the more precise the approximate solution.

There are many approaches to the [!ac](FEM). One of the most common is the [Direct Stiffness Method](https://en.wikipedia.org/wiki/Direct_stiffness_method), which is probably what most people are first taught when they take an introduction to [!ac](FE) analysis course. This method usually involves assembling a system of linear equations in the following form,

!equation id=stiffness
\bold{A} x = b

Here, the term $\bold{A}$, is often referred to as the *stiffness matrix*. In the direct stiffness method, $\bold{A}$ is developed by adding the known coefficients corresponding to an action, $b$, generated along the component of a [!ac](DOF), $x$, when $x$ takes on a unit value - one [!ac](DOF) at a time. As mentioned, these coefficients are usually well-known, and would normally be obtained from some table available in the literature. For example, the stiffness corresponding to all six [!ac](DOFs) of an Euler-Bernoulli beam (three displacements plus three rotations) could normally be found in a textbook on Mechanics of Materials.

Methods such as the Direct Stiffness one, however, are misleading about the basic theory behind the [!ac](FEM). They are merely convenient solutions to certain differential equations. To demonstrate this, consider the Euler-Bernoulli beam equation:

!equation id=beam
\dfrac{d^{2}}{dx^{2}} \left(E I \dfrac{d^{2} u}{dx^{2}} \right) = w

[beam] is often overlooked in structural analysis, since nearly every possible solution to it has been found and documented. This makes it possible to use a direct stiffness approach, which downplays the true purpose of [!ac](FE) analysis. The beauty of the [!ac](FEM) is that it is possible to solve [beam],
as well as many other differential equations, with no preexisting knowledge of any sort of solution. The [!ac](FEM) is also a general purpose one, i.e., the same procedure is applied to any given [!ac](PDE), and the end result is usually a system of equations like [stiffness], which are readily solved by a computer.

The intent of the foregoing discussion is to clear up any confusion the reader may have about what exactly the [!ac](FEM) is. Again, *+the Finite Element Method is a method for solving differential equations+*. This is a simple, yet important, definition for [!ac](MOOSE) users to keep in mind. [!ac](MOOSE) is designed to couple any arbitrary number of [!ac](PDEs), which are developed by a multi-disciplinary team of scientists and engineers, and so it is necessary to fall back to the fundamentals of [!ac](FE) analysis, as methods like direct stiffness fail to provide the level of flexibility needed for this cause. Generally, [!ac](MOOSE) employs the [Galerkin method](https://en.wikipedia.org/wiki/Galerkin_method). For more information regarding the [!ac](FEM), as it is implemented in [!ac](MOOSE), refer to the [help/finite_element_concepts/index.md] page.

## Where is the Stiffness Matrix?

[!ac](MOOSE) places no special emphasis on the concept of the stiffness matrix, i.e., the term, $\bold{A}$, in [stiffness],
or any other matrices which emerge from the [!ac](FE) analysis procedure. In fact, developers may ignore matrices altogether, i.e., they need not worry about the exact details of the algebra. All that they need to concern themselves with is their governing equations and the means by which they wish to evaluate those equation's integrals and approximate their solutions. Some people seem to believe that matrices are the defining characteristic of the [!ac](FEM), when, really, they are simply a consequence of the formulation. Don't worry, the so-called stiffness matrix exists during a [!ac](MOOSE) execution, in some form or another, but matrix operations are handled externally, i.e., with the [Portable, Extensible Toolkit for Scientific Computation (PETSc)](https://www.mcs.anl.gov/petsc/). During a solve procedure, [!ac](MOOSE) and PETSc communicate back and forth to build, condition and produce a solution to [stiffness].
