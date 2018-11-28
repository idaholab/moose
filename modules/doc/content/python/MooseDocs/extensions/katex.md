# KaTeX Extension

Rendering math is enabled using [KaTeX]. The extension adds the ability
to create numbered equations as well as reference the equations with traditional shortcut syntax:
e.g., `[eq-heat]`.

!devel settings module=MooseDocs.extensions.katex
                object=KatexExtension
                id=katex-extension-config
                caption=Available configure options for th KatexExtension object.



## Block Equations

Numbered and non-numbered equations are defined using the `\begin{equation}` and `\end{equation}`
environment common to [LaTeX mathematics](https://en.wikibooks.org/wiki/LaTeX/Mathematics),
as shown in [katex-numbered].

!devel! example id=katex-numbered
                caption=Example of syntax for creating numbered equations with [KaTeX].
\begin{equation}
y = a\cdot x + b
\end{equation}
!devel-end!

To include a non-numbered equation, simply use the `*` version of the environment, as shown in
[katex-no-number].

!devel! example id=katex-no-number
                caption=Example of syntax for creating non-numbered equations with Katex.
\begin{equation*}
c^2 = a^2 + b^2
\end{equation*}
!devel-end!

It is possible to reference numbered block equations. First, the equation must contain a label. A
label is added using traditional `\label{my-eq}` command. Then within the text this label can be used
within a shortcut link, e.g. `[my-eq]` (see [/core.md#shortcut-link]).

[eq-label-example] provides a complete example of creating and referencing an equation. The prefix
is dictated by the extension prefix configuration option (see [katex-extension-config]).

!devel! example id=eq-label-example
                caption=Example that references a labeled, numbered block equation.
[eq-label] is a famous equation.

\begin{equation}
\label{eq-label}
E = mc^2
\end{equation}
!devel-end!

## Inline Equations

Inline equations also use traditional LaTeX syntax, i.e., the content is wrapped in single `$` as
shown below.

!devel example id=katex-inline caption=Example of an inline LaTeX equation.
This $y=2\phi$ is inline.

[KaTeX]: https://khan.github.io/KaTeX
