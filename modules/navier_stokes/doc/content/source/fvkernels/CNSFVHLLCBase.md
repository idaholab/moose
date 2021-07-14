# CNSFVHLLCBase

`CNSFVHLLCBase` is the base class from which all Harten-Lax-Van Leer-Contact
(HLLC) residual computing objects inherit from. It computes the wave speeds for
the HLLC formulation. Because it is the base class for all HLLC residual
objects, we will use its documentation page to outline the HLLC equations. The
HLLC flux is defined as:

\begin{equation}
\bm{F}_{HLLC} =
\begin{cases}
\bm{F}_L & \textrm{if } 0 \leq S_L \\
\bm{F}_{*L} & \textrm{if } S_L \leq 0 \leq S_* \\
\bm{F}_{*R} & \textrm{if } S_* \leq 0 \leq S_R \\
\bm{F}_R & \textrm{if } 0 \geq S_R
\end{cases}
\end{equation}

where $\bm{F}_{L,R}$ are the left and right evaluations respectively of the
convective flux:

\begin{equation}
\bm{F} =
\begin{bmatrix}
\rho a_n\\
\rho a_n u + p n_x\\
\rho a_n v + p n_y\\
\rho a_n w + p n_z\\
\rho e_t + p
\end{bmatrix}
\end{equation}

where $a_n = \bm{a}\cdot\hat{n}$ where $\bm{a} = \lbrace u, v, w\rbrace$ and
$\hat{n}$ is the normal vector at the face. $n_x$, $n_y$, and $n_z$ represent
the $x$, $y$, and $z$ components of the normal vector respectively, while $u$,
$v$, and $w$ represent the $x$, $y$, and $z$ components of the velocity $\bm{a}$
respectively. $p$ is the static pressure and $e_t = e +
\lparen\bm{v}\cdot\bm{v}\rparen/2$ where $e$ is the specific internal
energy. $\rho$ is the density. $S_L$, $S_*$, and $S_R$ are the left, middle, and
right wave speeds respectively. $S_*$ is given by [!citep](toro2009riemann):

\begin{equation}
S_* = \frac{p_R - p_L + \rho_L a_{n,L}\left(S_L - a_{n,L}\right) - \rho_R a_{n,R}
\left(S_R - a_{n,R}\right)}{\rho_L \left(S_L - a_{n,L}\right) -
\rho_R\left(S_R - a_{n,R}\right)}
\end{equation}

The intermediate flux states (denoted by *) are described by [!citep](toro2009riemann):

\begin{equation}
\bm{F}_{*L} = \bm{F}_L + S_L\left(\bm{U}_{*L} - \bm{U}_L\right)\\
\bm{F}_{*R} = \bm{F}_R + S_R\left(\bm{U}_{*R} - \bm{U}_R\right)
\end{equation}

where the solution states $\bm{U}_{L,R}$ are given by the left and right
evaluations of

\begin{equation}
\bm{U}_K =
\begin{bmatrix}
\rho_K\\
\rho_K u_K\\
\rho_K v_K\\
\rho_K w_K\\
\rho_K e_{t,K}
\end{bmatrix}
\end{equation}

where $K=L$ and $K=R$ respectively and the intermediate solution states (the * states) are described by the left
and right evaluations of [!citep](toro2009riemann):

\begin{equation}
\bm{U}_{*K} = \rho_K \left(\frac{S_K - a_{n,K}}{S_K - S_*}\right)
\begin{bmatrix}
1\\
n_x \left(S_* - a_{n,K}\right) + u_K\\
n_y \left(S_* - a_{n,K}\right) + v_K\\
n_z \left(S_* - a_{n,K}\right) + w_K\\
e_{t,K} + \left(S_* -
a_{n,K}\right)\left[S_* + \frac{p_K}{\rho_K\left(S_K - a_{n,K}\right)}\right]
\end{bmatrix}
\end{equation}

The final piece to finish the definition of the HLLC flux is to specify the
evaluation of the left and right wave speeds, $S_L$ and $S_R$. These can be
evaluated in a variety of ways, but the current `CNSFVHLLCBase` implementation
evaluates them as follows, following [!citep](batten1997average):

\begin{equation}
S_L = \text{min}\lparen a_{n,L} - c_L, a_{n,\text{Roe}} - c_{\text{Roe}}\rparen\\
S_R = \text{max}\lparen a_{n,R} + c_R, a_{n,\text{Roe}} + c_{\text{Roe}}\rparen
\end{equation}

where $c$ denotes the local speed of sound in the medium and $\text{Roe}$
denotes Roe-averaged quantities which are computed via

\begin{equation}
b_{\text{Roe}} = \frac{\sqrt{\rho_L} b_{L} + \sqrt{\rho_R}
b_{R}}{\sqrt{\rho_L} + \sqrt{\rho_R}}
\end{equation}

where $b$ can be any quantity such as $a_n$ or $c$.

!bibtex bibliography
