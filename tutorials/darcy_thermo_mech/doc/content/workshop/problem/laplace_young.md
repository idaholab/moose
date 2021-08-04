# Hands-on: Laplace Young

!---

## Problem Statement

Given a domain $\Omega$, find $u$ such that:

!equation
-\nabla \cdot \left( k(u) \nabla u \right) + \kappa u = 0 \in \Omega,

and

!equation
\quad k(u) \nabla u \cdot \hat{n} = \sigma \in \partial \Omega,

where

!equation
k(u) \equiv \frac{1}{\sqrt{1 + |\nabla u|^2}} \\
\kappa=1 \\
\sigma=0.2

!---

## Laplace-Young Solution

!media darcy_thermo_mech/laplaceyoung.png style=width:70%;margin-left:auto;margin-right:auto;display:block;
