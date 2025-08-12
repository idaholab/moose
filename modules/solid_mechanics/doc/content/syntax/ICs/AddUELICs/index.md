# AddUELICs Action

The `ICs/AddUELICs` action imports field initial conditions from the Abaqus input via the
[AbaqusUELMesh](mesh/AbaqusUELMesh.md), creating node-set boundaries and `ConstantIC` objects
accordingly. Use it inside the `[ICs]` block to initialize variables on Abaqus node sets.
