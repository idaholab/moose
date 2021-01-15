# INSFVNoSlipWallBC

This object simply wraps [`FVFunctionDirichletBC`](FVFunctionDirichletBC.md). So
the required parameter is `function` describing the boundary wall velocity for
the velocity component specified with `variable`. If applying
`INSFVNoSlipWallBC` for any velocity component on a given `boundary`, then an
`INSFVNoSlipWallBC` should be specified for every velocity component on that
`boundary`.

!syntax parameters /FVBCs/INSFVNoSlipWallBC

!syntax inputs /FVBCs/INSFVNoSlipWallBC

!syntax children /FVBCs/INSFVNoSlipWallBC
