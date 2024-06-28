# ActionComponents system

`ActionComponents` are derived from [Actions](actions/Action.md). They are meant to facilitate the setup of
complex simulations by splitting the definition of each part of the spatial systems involved.

!alert note
`ActionComponents` are not compatible with [Components](Components/index.md optional=True). `ActionComponents` are intended
to be a rework on `Components` that does not hard-code the equations to be defined on the components and
can co-exist with the `[Mesh]` block.

!syntax parameters /ActionComponents/AddActionComponentAction

!syntax list /ActionComponents objects=True actions=False subsystems=False

!syntax list /ActionComponents objects=False actions=False subsystems=True

!syntax list /ActionComponents objects=False actions=True subsystems=False
