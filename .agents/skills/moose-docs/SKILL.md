---
name: moose-markdown-documentation
description: Write a documentation file for a MOOSE class or MOOSE object
---

# MOOSE Markdown Documentation Skill

This skill helps write MOOSE documentation using MooseDocs markdown extensions.

## File Organization

- Framework docs: `framework/doc/content/source/[type]/ClassName.md`
- Module docs: `modules/[module]/doc/content/source/[type]/ClassName.md`
- Each `.md` file corresponds to a `.C` file containing `registerMooseObject()`
- File name must exactly match the class name (case-sensitive)

## Standard Object Documentation Structure

```markdown
# ClassName

!syntax description /ObjectType/ClassName

## Description

`ClassName` [description of what the object does, mentioning the class name
without spaces in the first paragraph].

[Detailed explanation with equations if relevant]

## Example Input Syntax

!listing path/to/test/file.i block=BlockName

!syntax parameters /ObjectType/ClassName

!syntax inputs /ObjectType/ClassName

!syntax children /ObjectType/ClassName
```

For classes with AD variants, use: `# ClassName / ADClassName`

The `!syntax description` line is optional if you're writing a full description yourself.

## Core Syntax Commands

### Automatic Content Generation

```markdown
!syntax description /Kernels/Diffusion     # Class description from addClassDescription()
!syntax parameters /Kernels/Diffusion      # Parameter table from validParams()
!syntax inputs /Kernels/Diffusion          # List of input files using this object
!syntax children /Kernels/Diffusion        # Child classes that inherit from this
!syntax list /Kernels                      # List all objects in a system
```

The syntax path matches the input file structure: `/SystemType/ClassName` (e.g., `/Kernels/Diffusion`, `/BCs/DirichletBC`, `/Materials/GenericConstantMaterial`)

## Parameter References

**ALWAYS use `[!param]()` syntax when referring to parameters.** Every time you mention a parameter in the documentation text, wrap it with this syntax:

```markdown
The [!param](/Kernels/Diffusion/variable) parameter specifies the variable.
Set [!param](/Materials/DerivativeParsedMaterial/coupled_variables) to list coupled variables.
The [!param](/BCs/DirichletBC/boundary) parameter defines which boundaries to apply the condition on.
```

The path format is: `/SystemType/ClassName/parameter_name`

This corresponds to the input file syntax structure:
- `[Kernels]` block -> `/Kernels/`
- Object type (e.g., `type = Diffusion`) -> `/Kernels/Diffusion/`
- Parameter name (e.g., `variable = u`) -> `/Kernels/Diffusion/variable`

**Important:** Use `[!param]()` for EVERY parameter reference in your documentation, not just the first mention.

## Linking Conventions

**Page links do NOT use leading slashes.** The system searches for matching filenames:

```markdown
[Diffusion](Diffusion.md)                    # Explicit link syntax - "Diffusion" renders as text, links to Diffusion.md
[text](core.md#heading-id)                   # With bookmark to specific heading
[syntax/Kernels/index.md]                    # Partial path for disambiguation
[Diffusion]                                  # SHORTCUT syntax only - automatically finds Diffusion.md
[core.md]                                    # SHORTCUT syntax - uses first heading as text
```

**Use explicit link syntax `[ObjectName](ObjectName.md)` when linking to MOOSE objects.** The text in square brackets (display text) should NOT include .md, but the link target in parentheses MUST include .md. The syntax `[ObjectName]` without parentheses is a shortcut only.

**Source code links DO use leading slashes:**
```markdown
[/Diffusion.C]                               # Opens modal with full source
[Diffusion Kernel](/Diffusion.C)             # Custom link text
[`run_tests`](/test/run_tests language=python)  # With syntax highlighting
```

The distinction:
- `.md` files (documentation pages): NO leading slash
- `.C`, `.h`, `.py` files (source code): Leading slash for modal display

## Code Listings

Include code from repository files (never copy-paste):

```markdown
!listing framework/src/kernels/Diffusion.C                    # Full file
!listing test/tests/kernels/simple_diffusion/simple_diffusion.i block=Kernels  # HIT block
!listing framework/src/kernels/Diffusion.C start=computeQp end=}               # Line range
!listing path/to/file.i block=Kernels id=example caption=Example usage.        # With caption
```

## Mathematical Equations

Inline: `$\nabla \cdot \nabla u = 0$`

Display with label:
```markdown
\begin{equation}
R_i(u_h) = (\nabla \psi_i, \nabla u_h) = 0 \quad \forall \psi_i
\label{eq:weak-form}
\end{equation}
```

Formatting standards:
- Powers: `$6 \times 10^{-6}$` (not `\cdot`)
- Exponentials: `$\exp \left( x \right)$` (not `e^x`)
- Units: `$\mathrm{\mu}$m` (upright font)
- Always blank line between equation and explanation

## Alerts

```markdown
!alert note
This is a note.

!alert warning title=Important
This is a warning with custom title.

!alert tip title=Pro Tip
Helpful tip here.

!alert error
Error message.
```

Block version for complex content:
```markdown
!alert! warning title=Caution
- Item one
- Item two
!alert-end!
```

## Images and Media

```markdown
!media path/to/image.png
       style=width:50%;
       id=fig-example
       caption=Description of the figure.
```

Always include `caption=` and `id=` for figures.

## Citations

```markdown
[!cite](reference_key)           # Smith et al. (2020)
[!citep](reference_key)          # (Smith et al., 2020)
[!cite](ref1, ref2)              # Multiple citations
```

Add references to appropriate `.bib` files in `doc/` directories.

## Tables

```markdown
!table id=my-table caption=Table description.
| Column 1 | Column 2 |
| :- | -: |
| Left aligned | Right aligned |
```

## Writing Guidelines

1. **Target end-users** - Focus on usage, not implementation
2. **Include theory** - Explain the physics/math with equations
3. **Show working examples** - Use `!listing` to reference test files
4. **Document limitations** - Note validity ranges and assumptions
5. **Use consistent headings** - Only `##` for main sections (appears in sidebar)
6. **Class name in first paragraph** - Mention the class name without spaces
7. **No implementation details** - Avoid inheritance info, internal workings
8. **Always use `[!param]()` for parameters** - Every parameter reference must use this syntax
9. **Use explicit object links** - Link to objects with `[ObjectName](ObjectName.md)` syntax

## Cross-Referencing Other Documentation

Link to related objects without leading slashes, using explicit link syntax:

```markdown
## Similar Objects

- For a function-based value: [FunctionDirichletBC](FunctionDirichletBC.md)
- Using a penalty method: [PenaltyDirichletBC](PenaltyDirichletBC.md)
- See the [Kernels overview](syntax/Kernels/index.md)
```

Note: Use `[ObjectName](ObjectName.md)` syntax where the display text doesn't include .md, but the link target does.

## Complete Example

A boundary condition documentation page:

```markdown
# DirichletBC

!syntax description /BCs/DirichletBC

## Description

`DirichletBC` is the simplest type of `NodalBC`, and is used for
imposing so-called "essential" boundary conditions on systems of
partial differential equations (PDEs). Such boundary conditions force
a particular set of degrees of freedom (DOFs) defined by the
[!param](/BCs/DirichletBC/boundary) parameter to take on a single, controllable value.

This class is appropriate to use for PDEs of the form

\begin{equation}
\begin{aligned}
  -\nabla^2 u &= f && \quad \in \Omega \\
  u &= g && \quad \in \partial \Omega_D
\end{aligned}
\end{equation}

where $\Omega \subset \mathbb{R}^n$ is the domain.

## Similar Dirichlet BCs

- To use a Function instead of a constant value: [FunctionDirichletBC](FunctionDirichletBC.md)
- To impose a Dirichlet BC using a penalty method: [PenaltyDirichletBC](PenaltyDirichletBC.md)

## Example Input Syntax

!listing test/tests/bcs/dirichlet_bc/dirichlet_bc_test.i block=BCs

The [!param](/BCs/DirichletBC/value) parameter sets the boundary value.

!syntax parameters /BCs/DirichletBC

!syntax inputs /BCs/DirichletBC

!syntax children /BCs/DirichletBC
```

## Finding the Correct Syntax Path

The syntax path corresponds to input file structure:
- Look at how the object is used in input files
- A kernel used as `[Kernels] [diff] type = Diffusion` has path `/Kernels/Diffusion`
- A BC used as `[BCs] [left] type = DirichletBC` has path `/BCs/DirichletBC`
- Parameters are appended: `/BCs/DirichletBC/value`

Check `registerMooseObject()` calls in `.C` files to find the registered name.

## Finding Test Input Files for Examples

Use grep to find test files that use a particular object:

```bash
grep -r "type = ClassName" test/tests/ modules/*/test/tests/
```

Good example files:
- Should demonstrate typical usage
- Should be simple and focused
- Avoid files that test edge cases or error conditions

## Generating Documentation Stubs

For new objects, generate a stub file:

```bash
cd modules/[module]/doc
./moosedocs.py generate [AppName]
```

This creates template files that you can then fill in with content.

## Common Mistakes to Avoid

1. **Leading slashes on page links** - Use `[FunctionDirichletBC](FunctionDirichletBC.md)` not `[/FunctionDirichletBC.md]`
2. **Missing class name in first paragraph** - Always mention `ClassName` in the description
3. **Copy-pasting code** - Use `!listing` to include code from repository
4. **Implementation details** - Focus on usage, not internal workings
5. **Missing blank line after equations** - Required before "where..." explanations
6. **Wrong heading levels** - Only `##` appears in sidebar navigation
7. **Forgetting `[!param]()` syntax** - EVERY parameter reference must use `[!param](/Path/To/parameter)`
8. **Using shortcut `[Object]` syntax** - Use explicit `[ObjectName](ObjectName.md)` for clarity
9. **Adding .md to display text** - Use `[ObjectName](ObjectName.md)` not `[ObjectName.md](ObjectName.md)`

## Test Specification Requirements

When adding new tests, include documentation metadata in the `tests` file:

```
[Tests]
  [my_test]
    type = 'Exodiff'
    input = 'my_test.i'
    exodiff = 'my_test_out.e'

    requirement = "The system shall compute the diffusion equation correctly."
    design = 'Diffusion.md'
    issues = '#1234'
  []
[]
```

- `requirement`: Describes what the test verifies (complete sentence)
- `design`: Links to relevant documentation files (partial paths OK)
- `issues`: GitHub issue numbers associated with this feature/fix
