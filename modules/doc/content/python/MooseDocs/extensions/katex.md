# KaTeX Extension

Rendering math is enabled using [KaTeX]. The extension adds the ability
to create numbered equations as well as reference the equations with traditional shortcut syntax:
e.g., `[eq-heat]`.

!devel settings module=MooseDocs.extensions.katex
                object=KatexExtension
                id=katex-extension-config
                caption=Available configure options for the KatexExtension object.



## Block Equations

Numbered and non-numbered equations are defined using the `\begin{equation}` and `\end{equation}`
environment common to [LaTeX mathematics](https://en.wikibooks.org/wiki/LaTeX/Mathematics),
as shown in [katex-numbered].

!devel! example id=katex-numbered
                caption=Example of syntax for creating numbered equations with [KaTeX].
!equation id=line-eq
y = a\cdot x + b
!devel-end!

To include a non-numbered equation, simply exclude the "id" in the command, as shown in
[katex-no-number].

!devel! example id=katex-no-number
                caption=Example of syntax for creating non-numbered equations with Katex.
!equation
c^2 = a^2 + b^2
!devel-end!

It is possible to reference numbered block equations. First, the equation must contain an "id".  Then
within the text this label can be used within a shortcut link, e.g. `[my-eq]` (see [/core.md#shortcut-link]).

## Equation References

[eq-label-example] provides a complete example of creating and referencing an equation. The prefix
is dictated by the extension prefix configuration option (see [katex-extension-config]).

!devel! example id=eq-label-example
                caption=Example that references a labeled, numbered block equation.
[!eqref](eq-label) is a famous equation.

!equation id=eq-label
E = mc^2
!devel-end!

It is possible to reference equations on other pages. If the page containing the referenced
equation has a top-level heading then that will be used. The filename of the page is used when
no heading exists.

!devel! example id=eq-external-label-example
                caption=Example that references a labeled, numbered block equation from another page.
[!eqref](katex.md#eq-label) is a famous equation.
!devel-end!

## Inline Equations

Inline equations can use traditional LaTeX syntax, i.e., the content is wrapped in single `$` or
the inline `!eq` command, as shown in [katex-inline].

!devel example id=katex-inline caption=Example of an inline LaTeX equation.
This $y=2\phi$ is inline and so it this: [!eq](\phi=\beta^2).

## Macros

It is possible to define macros within extension configuration. This is done using the
'macros' configuration parameter (see [katex-extension-config]). For example, the main configuration
file for contains the following allowing for equation in [katex-macro] to be defined.

!listing modules/doc/config.yml start=MooseDocs.extensions.katex end=MooseDocs.extensions.appsyntax

!devel example id=katex-macro caption=Example use of a macro defined in configuration file.
$\pd{T}{t} = c$


[KaTeX]: https://khan.github.io/KaTeX
