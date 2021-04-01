# Quantitative Two Phase Polynomial Free Energies

## Introduction

In a species separation phase field model, a single concentration is used to model a two phase system. The evolution of the concentration is predicted by taking derivatives of a double-well free energy. This free energy can be represented with a simple thermodynamic model such that

\begin{equation}
f_0(c) = k_b T (c \ln c + (1 - c) \ln(1 - c)) + Wc(1-c)
\end{equation}

However, the natural logs in this expression can significantly complicate the solution, typically requiring small time steps to converge.

An alternative to the thermodynamic free energy is to use polynomial free energy, which simplifies the numerical solution but decrease the physical accuracy. We have implemented polynomial free energies of three different orders in the phase field module that can be used to solve this problem.

## Polynomial Equations

We have implemented three polynomial free energies of different orders, as shown in the figure.

!media phase_field/free_energies.png style=width:250px;padding-left:20px;float:right;
    caption=Plot of the three polynomial free energies, with the thermodynamic free energy shown for reference.

The simplest model is the fourth order free energy, where

\begin{equation}
f^4_0(c) = 16 W (c - c_{eq})^2 (1 - c - c_{eq})^2,
\end{equation}

Where $W$ is the height of the energy barrier and $c_{eq}$ is the equilibrium concentration.

The sixth and eighth order polynomials are defined by the equation

\begin{equation}
f^N_0(c) = 2^N W \sum_{n=1}^N H^N_n c^n
\end{equation}

where

\begin{equation}
\begin{aligned}
  H^6_1 =& \frac{3}{2} c_{eq} (c_{eq} - 1) \\
  H^6_2 =& - \frac{9}{2} c_{eq}^2 + \frac{9}{2} c_{eq} + \frac{3}{4} \\
  H^6_3 =& 6 c_{eq}^2 - 6 c_{eq} - \frac{7}{2} \\
  H^6_4 =& - 3.0 c_{eq}^2 + 3.0 c_{eq} + \frac{27}{4} \\
  H^6_5 =& -6\\
  H^6_6 =& 2
\end{aligned}
\end{equation}

and

\begin{equation}
\begin{aligned}
  H^8_1 =& \frac{3}{4} c_{eq}(c_{eq} - 1) \\
  H^8_2 =&  - \frac{15}{4}c_{eq}^2 + \frac{5}{4} c_{eq} + \frac{3}{8} \\
  H^8_3 =& 10 c_{eq} - 10 c_{eq} - \frac{11}{4} \\
  H^8_4 =& - 15 c_{eq}^2 + 15 c_{eq} + \frac{75}{8} \\
  H^8_5 =& 12 c_{eq}^2 - 12 c_{eq} - 18 \\
  H^8_6 =& -4 c_{eq}^2 + 4 c_{eq} + 20 \\
  H^8_7 =& - 12 \\
  H^8_8 =& 3
\end{aligned}
\end{equation}

## Quantitative Model Development

To create a quantitative model using the polynomial free energies, values for the mobility $M$, the barrier height $W$, $\kappa$ and $c_{eq}$ must be determined in terms of measurable quantities.  The equilibrium concentration $c_{eq}$ defines the locations of the wells in the free energy according to

\begin{equation}
  c_{eq} = e^{-\frac{E_f}{k_b T}}
\end{equation}

To determine the other parameters, we follow the derivation from [Moelans et al. (2008)](https://doi.org/10.1103/PhysRevB.78.024113).  To simplify the derivation, we consider an interface along the $y$-direction, such that the interfacial width is defined as

\begin{equation}
\begin{aligned}
	l_i =& \frac{1}{|d x/d c|_{x=0}} \\
	    =& \sqrt{\frac{\kappa_N}{2\ \mathrm{max}({f^N_{loc}})}}.
\end{aligned}
\end{equation}

The polynomial free energies have been developed such that $\mathrm{max}(f^N_{loc}) = W$, thus

\begin{equation}
	l_i =  \sqrt{\frac{\kappa}{2 W}}.
\end{equation}

Therefore, $\kappa_N$ can be expressed as a function of the barrier height and the interfacial width according to

\begin{equation}
	\kappa = 2 W l_i^2 .\label{eq:kappa}
\end{equation}

To define the barrier height $W$, we first determine the surface energy.  The surface energy is calculated according to

\begin{equation}
	\sigma = \int^{+\infty}_{-\infty}  \left( f^N_{loc} + \frac{\kappa}{2} \left( \frac{\partial c}{\partial x} \right)^2 \right) dx.\label{eq:gbenergy1}
\end{equation}

According to the principles of variational calculus, the functions that extremize the function have zero first derivatives, i.e.

\begin{equation}
	\frac{\partial f_{loc}}{\partial c} - \kappa \frac{\partial^2 c}{\partial x^2} = 0,
\end{equation}

or, the integrated equation

\begin{equation}
	f_{loc} - \frac{\kappa}{2} \left( \frac{\partial c}{\partial x} \right)^2 = 0.
\end{equation}

By rearranging the equation, we obtain

\begin{equation}
	\frac{\partial c}{\partial x} = \sqrt{ \frac{2 f^N_{loc}}{\kappa}}. \label{eq:dcdx}
\end{equation}

Combining these equations gives

\begin{equation}
\begin{aligned}
	\sigma =& \int^{+\infty}_{-\infty}  \left( f^N_{loc} + \frac{\kappa}{2} \frac{2 f^N_{loc}}{\kappa} \right) dx \\
	       =& 2 \int^{+\infty}_{-\infty}   f_{loc}(c) \ dx.
\end{aligned}
\end{equation}

After a change of variables and substitution,

\begin{equation}
\begin{aligned}
	\sigma =& 2 \int^{1}_{0}  f_{loc}(c) \frac{d x}{d c} dc \\
	       =& \sqrt{2 \kappa} \int^{1}_{0}  \sqrt{f_{loc}(c)}\ dc. \label{eq:gbenergy2}
\end{aligned}
\end{equation}

To define the value of the integral $\int^{1}_0 \sqrt{f_{loc}(c)} \ dc$, we express it as

\begin{equation}
\begin{aligned}
	\int^{1}_{0}  \sqrt{f_{loc}(c)}\ dc =& \sqrt{W} \ 2^{\frac{N}{2}}  \int^{1}_{0} \sqrt{\sum_{n=0}^N H^N_n  c^{\frac{n}{2}}} dc \\
	                                    =& \sqrt{W} \ K_N \label{eq:int}
\end{aligned}
\end{equation}

where $K_N  = 2^{\frac{N}{2}}  \int^{1}_{0} \sqrt{\sum_{n=0}^N H^N_n  c^{\frac{n}{2}}} dc$ is a function of $c_{eq}$ that varies for each polynomial order $N$.  If we assume that $c_{eq} \ll 1$, $K_N$ becomes a constant value which we can calculate for each polynomial order, according to

\begin{equation}
\begin{aligned}
	K_4 =& \frac{2}{3} \\
	K_6 =& \frac{3}{16} \sqrt{3} - \frac{9}{64} \sqrt{2} \ln \left(\frac{\sqrt{3} - \sqrt{2}}{\sqrt{3} + \sqrt{2}} \right) \\
	K_8 =& 0.83551
\end{aligned}
\end{equation}

with $K_8$ evaluated numerically.  Thus, the surface energy is equal to

\begin{equation}
	\sigma = K_N \sqrt{2 \kappa W}. \label{eq:surfenergy}
\end{equation}

We can then solve for $W$ to obtain

\begin{equation}
	W = \frac{\sigma^2}{2 K_N^2 \kappa_N}.
\end{equation}

By combining these equations, we obtain

\begin{equation}
	\kappa = \frac{\sigma l_i}{ K_N}.
\end{equation}

This expression is then substituted to give

\begin{equation}
	W = \frac{\sigma}{2 K_N l_i}.
\end{equation}


The form of the mobility must be derived to produce a quantitative physical behavior.  In order for the phase field model to accurately describe vacancy diffusion, the Cahn-Hilliard equation must be equivalent to Fickian diffusion when $c \ll 1$, i.e.

\begin{equation}
	\nabla \cdot M \nabla \frac{\partial F}{\partial c} = \nabla \cdot D \nabla c,
\end{equation}

where $M$ is the mobility for the polynomial free energy and $D$ is the diffusivity.  Thus, for $c \ll 1$

\begin{equation}
	M = D \frac{\nabla c}{\nabla {\partial f^N_{loc}}/{\partial c}} \label{eq:mob}
\end{equation}

For each of the polynomial free energies, the value of $\nabla \partial f_{loc}^N/\partial c$ for $c \ll 1$ can be written in the form

\begin{equation}
	\nabla \frac{\partial f_{loc}^N}{\partial c} = 2^{N+1} H^N_2 W \nabla c \label{eq:mob2}
\end{equation}

where $H^N_2$ is a polynomial coefficient defined above.   Thus, the value for the mobility is determined by combining the equations to obtain

\begin{equation}
\begin{aligned}
	M =&  \frac{D}{2^{N+1} C^N_2 W_N} \\
	  =& \frac{K_N}{ 2^{N+1} C^N_2} \frac{D l_i}{ \sigma}\label{eq:Mi}
\end{aligned}
\end{equation}


To summarize, the quantitative expressions for the four model parameters required by the polynomial free energies can be obtained using the equations above.  We restate them here:

\begin{equation}
\begin{aligned}
	c_{eq} =& e^{-\frac{E_f}{k_b T}} \\
	\kappa =& \frac{\sigma l_i}{K_N}  \\
	W      =& \frac{\sigma}{2 K_N l_i} \\
	M      =& \frac{K_N}{ 2^{N+1} H^N_2} \frac{D l_i}{ \sigma}
\end{aligned}
\end{equation}


## Numerical Implementation in the Phase Field Module

The polynomial free energies have been implemented in the [`PolynomialFreeEnergy`](/PolynomialFreeEnergy.md) material in the MOOSE phase field module. The free energies are all contained in the same material, where the order is an input parameter. To minimize required code and coding errors, the free energy derivatives are obtained using automatic differentiation via the [`ExpressionBuilder`](FunctionMaterials/ExpressionBuilder.md) tools. The free energy is used by the parsed kernels, [`SplitCHParsed`](/SplitCHParsed.md) and [`CahnHilliard`](/CahnHilliard.md).

The material properties required for the model have been implemented in the [`PFParamsPolyFreeEnergy`](/PFParamsPolyFreeEnergy.md), which also takes the polynomial order as an input.
