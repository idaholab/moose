# KatexExtension
The [KaTeX](https://khan.github.io/KaTeX/) claims to be "the fastest math typesetting library for
the web." The main alternative being [MathJax](https://www.mathjax.org/). The KaTeX package is
used for MooseDocs for one reason: portability. MathJax is widely used, complete, and
well-supported; however, using it offline is difficult and requires a large number of files.

To the contrary KaTeX is more traditional in its web-based implementation and requires only a
stylesheet (CSS) and javascript (JS) file, which are included in repository to allow for offline
building.

A complete list of the supported LaTeX syntax is provided here: [LaTeX function support](https://khan.github.io/KaTeX/function-support.html).

## Inline Math
The KaTeX extension uses `$` with markdown for inline equations. For example, `$y=ax+b$` produces
the following: $y=ax+b$.

## Numbered Equations
Number equations must be included within traditional latex begin and end equation. For example,
the markdown in \ref{num-eqn} produces the following.

\begin{equation}
a^2+b^2=c^2
\end{equation}

!listing id=num-eqn caption=Example latex equation within markdown.
```
\begin{equation}
a^2+b^2=c^2
\end{equation}
```

## Non-numbered equations
It is also possible to create display style equations without numbering using the traditional latex star version of begin/end
equation. The following equation is generated from the markdown in \ref{no-num-eqn-0}.

\begin{equation*}
y=e^{1/t}
\end{equation*}

!listing id=no-num-eqn-0 caption=Example latex non-numbered display equation within markdown using stared latex equation block.
```
\begin{equation}
a^2+b^2=c^2
\end{equation}
```

It is also possible do use two dollars signs (`$$`) enclosing the latex to create a non-numbered equation. The following equation is generated from the markdown in \ref{no-num-eqn-1}.

$$
y = a + bx + cx^2 + dx^3
$$


!listing id=no-num-eqn-1 caption=Example latex non-numbered display equation within markdown using double dollar (`$$`).
```
$$
y = a + bx + cx^2 + dx^3
$$
```

## Errors
If the supplied latex does not evaluate you will see the content rendered as red text within the
browser, as shown below.

$$
y = \cos(2\pi x)\sine(2\pi y)
$$
