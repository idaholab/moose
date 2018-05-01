<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# FunctionIC

The FunctionIC class "couples" to a [Function](Functions/index.md) to produce values and optionally gradients to
initialize a spatial variable. Functions when appropriate, work well for initializing field because they can provide
values anywhere within the domain. This means that they can be used with [initial_adaptivity](/AdaptivityAction.md) to
create an area of refinement within the mesh before a simulation begins.

## Class Description

!syntax description /ICs/FunctionIC

!syntax parameters /ICs/FunctionIC

!syntax inputs /ICs/FunctionIC

!syntax children /ICs/FunctionIC

!bibtex bibliography
