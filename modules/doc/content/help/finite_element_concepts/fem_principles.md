# Polynomial Fitting

- To introduce the idea of finding coefficients to functions, let's consider simple polynomial fitting.
- In polynomial fitting (or interpolation) you have a set of points and you are looking for the coefficients to a function that has the form:
  \begin{equation}
  f(x) = a + bx + cx^2 + \dots
  \end{equation}
- Where $a$, $b$ and $c$ are scalar coefficients and $1$, $x$, $x^2$$ are "basis functions".
- Find $a$, $b$, $c$, etc. such that $$f(x)$$ passes through the points you are given.
- More generally you are looking for:
  \begin{equation}
  f(x) = \sum_{i=0}^d c_i x^i
  \end{equation}
  where the $c_i$ are coefficients to be determined.
- $f(x)$ is unique and interpolary if $d+1$ is the same as the number of points you need to fit.
- Need to solve a linear system to find the coefficients.

# Example

- Define a set of points:
  \begin{equation}
  \begin{array}{rl}
  (x_1, y_1) &= (1,4) \\
  (x_2, y_2) &= (3,1) \\
  (x_3, y_3) &= (4,2)\end{array}
  \end{equation}
- Substitute $(x_i, y_i)$ data into the model:
  \begin{equation}
  y_i = a + bx_i + cx_i^2, i=1,2,3.
  \end{equation}
- Leads to the following linear system for $$$a$$$, $$$b$$$, and $$$c$$$:
  \begin{equation}
  \begin{bmatrix}
  1 & 1 &  1 \\
  1 & 3 &  9 \\
  1 & 4 & 16
  \end{bmatrix}
  \begin{bmatrix}
  a \\
  b \\
  c
  \end{bmatrix}
  =
  \begin{bmatrix}
  4 \\
  1 \\
  2
  \end{bmatrix}
  \end{equation}


- Solving for the coefficients results in:
  \begin{equation}
  \begin{bmatrix}
  a \\
  b \\
  c
  \end{bmatrix}
  =
  \begin{bmatrix}
  8 \\
  \frac{29}{6} \\
  \frac{5}{6}
  \end{bmatrix}
  \end{equation}
- These define the solution *function*:
  \begin{equation}
  f(x) = 8 - \frac{29}{6} x + \frac{5}{6} x^2
  \end{equation}

- Important! The solution is the function, *not* the coefficients.

!media media/finite_element_concepts/quadratic-polynomial.jpg
       style=width:50%;

- The coefficients themselves don't mean anything, by themselves they are just numbers.
- The solution is *not* the coefficients, but rather the *function* they create when they are multiplied by their respective basis functions and summed.
- The function $f(x)$ does go through the points we were given, *but it is also defined everywhere in between*.
- We can evaluate $f(x)$ at the point $$$x=2$$$, for example, by computing:
  $f(2) = \sum_{i=0}^2 c_i 2^i$ or more generically: $f(2) = \sum_{i=0}^2 c_i g_i(2),$
  where the $c_i$ correspond to the coefficients in the solution vector, and the $g_i$ are the respective functions.
- Finally, note that the matrix consists of evaluating the functions at the points.

# Finite Elements Simplified

- A method for numerically approximating the solution to Partial Differential Equations (PDEs).
- Works by finding a solution function that is made up of "shape functions" multiplied by coefficients and added together.
- Just like in polynomial fitting, except the functions aren't typically as simple as $x^i$ (although they can be).
- The Galerkin Finite Element method is different from finite difference and finite volume methods because it finds a piecewise continuous function which is an approximate solution to the governing PDE.
- Just as in polynomial fitting you can evaluate a finite element solution anywhere in the domain.
- You do it the same way: by adding up "shape functions" evaluated at the point and multiplied by their coefficient.
- FEM is widely applicable for a large range of PDEs and domains.
- It is supported by a rich mathematical theory with proofs about accuracy, stability, convergence and solution uniqueness.

# Weak Form

- Using FE to find the solution to a PDE starts with forming a "weighted residual" or "variational statement" or "weak form".
    - We typically refer to this process as generating a Weak Form.
- The idea behind generating a weak form is to give us some flexibility, both mathematically and numerically.
- A weak form is what you need to input to solve a new problem.
- Generating a weak form generally involves these steps:

1.  Write down strong form of PDE.
2.  Rearrange terms so that zero is on the right of the equals sign.
3.  Multiply the whole equation by a "test" function $$$\psi$$$.
4.  Integrate the whole equation over the domain $$$\Omega$$$.
5.  Integrate by parts (use the divergence theorem) to get the desired derivative order on your functions and simultaneously generate boundary integrals.

# Refresher: The divergence theorem

- Transforms a volume integral into a surface integral:
  \begin{equation}
  \int_{\Omega} \nabla \cdot \vec{g} \;\text{d}x = \int_{\partial \Omega} \vec{g} \cdot \hat{n} \;\text{d}s
  \end{equation}
- Slight variation: multiply by a smooth function, $\psi$:
  \begin{equation}
  \begin{array}{rl}
  \int_{\Omega} \psi (\nabla \cdot \vec{g}) \;\text{d}x &= \int_{\Omega}\nabla \cdot (\psi \vec{g}) \;\text{d}x - \int_{\Omega}\nabla \psi \cdot \vec{g} \;\text{d}x
  \\
  &= \int_{\partial \Omega} \psi \vec{g} \cdot \hat{n} \;\text{d}s - \int_{\Omega}\nabla \psi \cdot \vec{g} \;\text{d}x
  \end{array}
  \end{equation}

- In finite element calculations, for example with $\vec{g} = -k(x)\nabla u$, the divergence theorem implies:
  \begin{equation}
   -\int_{\Omega} \psi \left(\nabla \cdot k(x)\nabla u  \right) \;\text{d}x =
   \int_{\Omega}\nabla\psi \cdot k(x)\nabla u \;\text{d}x -
   \int_{\partial\Omega} \psi\left(k(x)\nabla u \cdot \hat{n} \right)  \; \text{d}s
  \end{equation}

- We often use the following inner product notation to represent integrals since it is more compact:
  \begin{equation}
  -\left( \psi,\nabla \cdot k(x)\nabla u \right)   =
  \left(\nabla\psi, k(x)\nabla u \right) -
  \langle \psi, k(x)\nabla u \cdot \hat{n} \rangle
  \end{equation}
- [http://en.wikipedia.org/wiki/Divergence_theorem](http://en.wikipedia.org/wiki/Divergence_theorem)

# Example: Convection Diffusion

- Write the strong form of the equation:
  \begin{equation}
  -\nabla\cdot k\nabla u + \vec{\beta} \cdot\nabla u = f  \phantom{\displaystyle \int}
  \end{equation}
- Rearrange to get zero on the right-hand side:
  \begin{equation}
  -\nabla\cdot k\nabla u + \vec{\beta} \cdot \nabla u - f = 0 \phantom{\displaystyle \int}
  \end{equation}

- Multiply by the test function $\psi$:
  \begin{equation}
  - \psi \left(\nabla\cdot k\nabla u\right) +
  \psi\left(\vec{\beta} \cdot=\nabla u\right) - \psi f = 0 \phantom{\displaystyle \int}
  \end{equation}

- Integrate over the domain $\Omega$:
  \begin{equation}
  {- \displaystyle\int_\Omega\psi \left(\nabla\cdot k\nabla u\right)} + \displaystyle\int_\Omega\psi\left(\vec{\beta} \cdot\nabla u\right) -
  \displaystyle\int_\Omega\psi f = 0 \phantom{\displaystyle \int}
  \end{equation}


- Apply the divergence theorem to the diffusion term:
  \begin{equation}
  \displaystyle\int_\Omega\nabla\psi\cdot k\nabla u -
  \displaystyle\int_{\partial\Omega} \psi \left(k\nabla u \cdot \hat{n}\right) +
  \displaystyle\int_\Omega\psi\left(\vec{\beta} \cdot\nabla u\right) - \displaystyle\int_\Omega\psi f = 0
  \end{equation}

- Write in inner product notation. Each term of the equation will inherit from an existing MOOSE type as shown below.
  \begin{equation}
  \underbrace{\left(\nabla\psi, k\nabla u \right)}_{Kernel} -
  \underbrace{\langle\psi, k\nabla u\cdot \hat{n} \rangle}_{BoundaryCondition} +
  \underbrace{\left(\psi, \vec{\beta} \cdot\nabla u\right)}_{Kernel} -
  \underbrace{\left(\psi, f\right)}_{Kernel} = 0 \phantom{\displaystyle \int}
  \end{equation}
