# MOOSE Documentation - Review Checklist

MOOSE's docs build (MooseDocs) is part of the quality gate: a new user-facing object without
its documentation page fails the build, and the SQA system cross-checks that descriptions,
parameter tables, and requirements stay consistent. So documentation findings are often
*required*, not optional polish.

## New MooseObjects need two things

When the diff registers a new object (`registerMooseObject(...)`, `registerMooseAction(...)`),
check both:

1. **A class description in code.** `validParams()` should call
   `params.addClassDescription("...")` with a concise one-line summary of what the object does.
   This string drives the syntax tables and system overviews. Each added parameter
   (`addRequiredParam`/`addParam`/`addRequiredCoupledVar`/...) should also pass a clear
   documentation string - these become the user-facing parameter help.

2. **A markdown page** whose path mirrors the source path. The pattern is
   `src/<area>/<ClassName>.C` -> `<app>/doc/content/source/<area>/<ClassName>.md`. Examples:
   - `framework/src/kernels/Diffusion.C` -> `framework/doc/content/source/kernels/Diffusion.md`
   - `modules/<mod>/src/.../Foo.C` -> `modules/<mod>/doc/content/source/.../Foo.md`

   Check the page exists at that path and carries real prose - what the object does, when to use
   it, the math/algorithm if relevant - not just the MooseDocs auto-generation markers
   (`!syntax parameters` / `!syntax inputs` / `!syntax children`). A page that is only the markers
   is a reason to request more: they regenerate the parameter table but explain nothing unusual or
   non-obvious. How to write the page itself (the full marker set, listings, equations, linking
   conventions) is an authoring concern, out of scope for review.

## Modified objects

- If a PR changes an existing object's parameters or behavior, the corresponding `.md` page and
  any prose referencing it must be updated to stay accurate. Stale documentation is a finding.

## Inline (Doxygen) documentation

- Request it when the class/class-member use is non-obvious from the name alone. In-source docs should be
  Doxygen-formatted and aimed at the developer. When in doubt, ask for more.

## Newsletter (for notable changes)

- New features and user-visible changes warrant an entry in the current month's newsletter:
  `modules/doc/content/newsletter/<year>/<year>_<month>.md` (e.g. `2026/2026_06.md`). Minor
  bugfixes get a one-line mention; larger features get a short paragraph. This is encouraged
  rather than hard-blocking, so phrase it as a suggestion unless the change is clearly
  significant.

## Accuracy over volume

Per the reviewing guidelines, thorough documentation is not required, but what exists must be
accurate and error-free, and must clarify anything unusual. Prioritize correctness of external
documentation over inline typos (use GitHub suggestions for the latter).
