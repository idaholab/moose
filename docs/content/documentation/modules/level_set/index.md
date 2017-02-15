# Level Set Module
The level set module provides basic functionality to solve the level set equation, the following links provided
detailed information on the theory and use of the level set module:

* [Theory Manual](level_set/theory.md)
* [Example 1: Constant Velocity](level_set/example_circle.md)
* [Example 2: Rotating "Bubble"](level_set/example_rotate.md)
* [Example 3: Vortex](level_set/example_vortex.md)

For reference the following tables list the objects contained within the level set module and a brief description
of there purpose, each object may be selected to navigate to a detailed page.

!systems groups=level_set

## Level Set Module Tasks
The following additional tasks should be completed to make the level
set module more useful and robust for real-world applications:

* Develop automated techniques for setting the various re-initialization tuning parameters ($\Delta \tau$, $\epsilon$, etc.), see the [Theory](level_set/theory.md) page for more details.
* Implement a signed-distance-preserving re-initialization scheme based on established methods, e.g. \cite{min2010reinitializing}.
* Implement additional stabilization techniques such as the Galerkin Least Squares \cite{hughes1989VIII}
        method and "shock/discontinuity capturing" schemes \cite{hughes1986beyond,shakib1991compressible}.
* Create module-specific input file syntax for level set problems to simplify input file generation and usage.
* Solve additional benchmark problems with various stabilization and re-initialization schemes,
    and investigate different mesh refinement and adaptivity strategies for said problems.

## References
\bibliographystyle{unsrt}
\bibliography{docs/bib/level_set.bib}
