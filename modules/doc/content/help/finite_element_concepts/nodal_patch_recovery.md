# Introduction

MOOSE offers an option to use the Zienkiewicz-Zhu patch recovery technique to evaluate nodal gradients (and other values that do not live on nodes), which leads to an accurate and superconvergent nodal gradient.

The Zienkiewicz-Zhu patch recovery algorithm relies on finding an element patch for a given node, and then performing a Least Squares Regression/Fitting (LSR) over all sample points in the constructed patch.

# Element Patch

In general, the element patch is defined as:
\begin{equation}
\Omega_{patch}=\bigcup_{i=1}^{n_{ce}} \Omega_i
\end{equation}
where $n_{ce}$ is the number of connected elements on the current node. Gauss points on each element are proved to be superconvergent sampling points. Therefore, sampling points of the patch is simply the union of all quadrature points on connected elements.

As will be discussed in the next section, nodal gradient will be reconstructed by fitting to a set of monomial terms. The number of monomial terms (or degrees of freedom) is given by $n = (d+p)!/(d!p!)$, where $d$ is the dimension of the mesh, $p$ is the polynomial order. Therefore, we need at least $n$ sampling points on the patch.

On internal nodes and side nodes, we have enough sampling points to perform the fitting. On corner nodes, the algorithm recommends using an interior patch that contains the corner node.

# Complete Multi-Indexing of Monomials

The gradient field is best fitted to a complete set of monomials $P_{\boldsymbol\alpha}$ up to certain order $p$. To enumerate combinations of $P_{\boldsymbol\alpha}$, let $\boldsymbol\alpha$ be a multi-index of length $d$, where $d$ is the dimension of the mesh:
\begin{equation}
\boldsymbol\alpha=(\alpha_1,...,\alpha_d),\hspace{0.5in}\alpha_i\in\mathbb{N}_{\geq 0}
\end{equation}
and let $|\boldsymbol\alpha|=\sum_{i=1}^{d}\alpha_i$. Define a family of monomials $\{P_{\boldsymbol\alpha}\}_{\boldsymbol\alpha}$ as functions of spatial coordinates $\{x,y,z\}$.
\begin{equation}
P_{|\boldsymbol\alpha|=m}=\{x^{\alpha_1}y^{\alpha_2}z^{\alpha_3}\}_{\boldsymbol\alpha}
\end{equation}
For example, $d=2$ and $p=2$, we will have
\begin{equation}
\boldsymbol P=[1,x,y,x^2,xy,y^2]
\end{equation}
Again, the number of monomial terms $n = (d+p)!/(d!p!)=6$ in this case.

# Least Squares Fitting

Rest of the algorithm remains straight-forward, as it follows a standard Least Squares Fitting:
\begin{equation}
\boldsymbol A\boldsymbol c=\boldsymbol b
\end{equation}
where $\boldsymbol A$ can be interpreted as the patch stiffness matrix, $\boldsymbol b$ the patch load vector, and $\boldsymbol c$ is a vector containing fitted coefficients. The stiffness matrix and the load vector are assembled over all sampling
points:
\begin{equation}
\begin{aligned}
\boldsymbol A&=\sum_{i=1}^n \boldsymbol P^T(\boldsymbol x_i)\boldsymbol P(\boldsymbol x_i)\\
\boldsymbol b&=\sum_{i=1}^n \boldsymbol P^T(\boldsymbol x_i)\nabla u(\boldsymbol x_i)\label{eq: load vector}
\end{aligned}
\end{equation}
note that $\nabla u(\boldsymbol x_i)$ is the solution gradient component-wise. All components of the solution gradient share the same stiffness matrix, so $A$ can be cached to reduce computational cost. The fitted nodal gradient can then be computed as:
\begin{equation}
\nabla u(\boldsymbol x^*)=\boldsymbol P(\boldsymbol x^*)\boldsymbol c
\end{equation}
