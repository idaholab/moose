# Basis Functions and Shape Functions

- While the weak form is essentially what you need for adding physics to MOOSE, in traditional finite element software more work is necessary.
- We need to discretize our weak form and select a set of simple "basis functions" amenable for manipulation by a computer.

!media media/finite_element_concepts/basis-function-example.jpg
       caption=Example of linear Lagrange shape function associated with single
       node on triangular mesh
       style=width:50%;

!media media/finite_element_concepts/Oden-linear-lagrange.jpg
       caption=1D linear Lagrange shape functions
       style=width:25%;

# Shape Functions

- Our discretized expansion of $$u$$ takes on the following form:
  $$$u \approx u_h = \sum_{j=1}^N u_j \phi_j$$$
- The $$$\phi_j$$$ here are called "basis functions"
- These $$$\phi_j$$$ form the basis for the "trial function", $$$u_h$$$
- Analogous to the $$$x^n$$$ we used earlier
- The gradient of $$$u$$$ can be expanded similarly:
   $$$\nabla u \approx \nabla u_h = \sum_{j=1}^N u_j \nabla \phi_j$$$
- In the Galerkin finite element method, the same basis functions are used for both the trial and test functions:
    $$$\psi = \{\phi_i\}_{i=1}^N$$$
- Substituting these expansions back into our weak form, we get:
    $$$\left(\nabla\psi_i, k\nabla u_h \right) - \langle\psi_i, k\nabla u_h\cdot \hat{n} \rangle +
       \left(\psi_i, \vec{\beta} \cdot \nabla u_h\right) - \left(\psi_i, f\right) = 0, \quad i=1,\ldots,N$$$
- The left-hand side of the equation above is what we generally refer to as the $$$i^{th}$$$ component of our "Residual Vector" and write as $$$R_i(u_h)$$$.
- Shape Functions are the functions that get multiplied by coefficients and summed to form the solution.
- Individual shape functions are restrictions of the global basis functions to individual elements.
- They are analogous to the $$$x^n$$$ functions from polynomial fitting (in fact, you can use those as shape functions).
- Typical shape function families: Lagrange, Hermite, Hierarchic, Monomial, Clough-Toucher
    - MOOSE has support for all of these.
- Lagrange shape functions are the most common.
    -  They are interpolary at the nodes, i.e., the coefficients correspond to the values of the functions at the nodes.

# Example 1D Shape Functions

!media media/finite_element_concepts/linear-lagrange-graph.jpg
       style=width:50%;

Linear Lagrange

!media media/finite_element_concepts/quadratic-lagrange-graph.jpg
       style=width:50%;

Quadratic Lagrange

!media media/finite_element_concepts/cubic-lagrange-graph.jpg
       style=width:50%;

Cubic Lagrange

!media media/finite_element_concepts/cubic-hermite-graph.jpg
       style=width:50%;

Cubic Hermite

# 2D Lagrange Shape Functions

Example bi-quadratic basis functions defined on the Quad9 element:

- $$$\psi_0$$$ is associated to a "corner" node, it is zero on the opposite edges.
- $$$\psi_4$$$ is associated to a "mid-edge" node, it is zero on all other edges.
- $$$\psi_8$$$ is associated to the "center" node, it is symmetric and $$$\geq 0$$$ on the element.

!media media/finite_element_concepts/quad9-corner.jpg
       style=width:50%;

$$$\psi_0$$$

!media media/finite_element_concepts/quad9-edge.png
       style=width:50%;

$$$\psi_4$$$

!media media/finite_element_concepts/quad9-center.jpg
       style=width:50%;

$$$\psi_8$$$
