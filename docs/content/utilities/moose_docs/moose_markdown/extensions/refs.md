# RefsExtension

The RefsExtension is used in conjunction with other extensions to enable refereeing to
[Numbered Floats](extensions/numbered_floats.md) as well as latex equations.

## Equations

When [mdx_math](https://github.com/mitya57/python-markdown-math) is utilized and the `\label{eqn:test}` was placed within the latex, as shown in \ref{eqref-example}, then it is possible to link to the equation using traditional latex syntax (`\eqref{eqn:test}`): Equation \eqref{eqn:test}.

\begin{equation}
\label{eqn:test}
x=\frac{1+y}{1+2z^2}.
\end{equation}

!listing id=eqref-example caption=Example of markdown syntax to enable equation references.
```markdown
\begin{equation}
\label{eqn:test}
x=\frac{1+y}{1+2z^2}.
\end{equation}
```

## Floats

As detailed in [Numbered Floats](extensions/numbered_floats.md) it is possible to create numbered
figures, listings, and tables as well as arbitrarily named floats. When the the "id" is supplied
to the command, as shown in \ref{media-example}, the `\refs{}` command can be used within the
the markdown to create label link to the float: \ref{cat}.

!media http://lorempixel.com/400/200/cats/ id=cat caption=A random cat image. style=width:100%;float:right

!listing id=media-example caption=Example of markdown syntax to enable float references.
```markdown
!media http://lorempixel.com/200/200/cats/ id=cat caption=A random cat image. style=width:32%;float:right;
```
