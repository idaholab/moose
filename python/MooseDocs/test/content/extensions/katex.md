# KaTeX Extension

## Inline

Inline [!eq](y=ax+b).

## Block

!equation id=pythagorean
a^2 + b^2 = c^2


!equation
G_{\mu\nu} = 8\pi G(T_{\mu\nu} + \rho_{\Lambda} g_{\mu\nu})

!include katex_include.md

!equation id=calculus
\int_a^b f'(x)dx = f(b) - f(a)

The fundamental theorem of calculus: [calculus].

## Macros

!equation
c = \pm\sqrt{a^2 + b^2}\in\RR

!equation
\pd{T}{t} = c


## Equation References

[!eqref](calculus)

[!eqref](second_law)

[!eqref](katex_include.md#second_law)

[!eqref](katex_include2.md#second_law)

## Deprecated

$y=2x$

\begin{equation}
y = 3x
\end{equation}
