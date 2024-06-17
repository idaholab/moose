# GhostingFromUOAux

!syntax description /AuxKernels/GhostingFromUOAux

# Description

`GhostingFromUOAux` allows you to visualize what the current algebraic and geometric ghosting functors (and RelationshipManagers) are going to do.  This is useful in tracking down both under and over-ghosting.

At any one time it will only show you the ghosted elements for one processor ID.

Normally, this class shouldn't be used directly. Instead set it up through the [DisplayGhostingAction.md].

!row!
!col! class=s12 m6 l6

!media media/aux_kernels/ghost_aux_geometric.png
       id=geometric
       caption=The default geometric ghosting for PID 1

!col-end!

!col! class=s12 m6 l6

!media media/aux_kernels/ghost_aux_algebraic.png
       id=algebraic
       caption=The default algebraic ghosting for PID 1

!col-end!

!row-end!

!syntax parameters /AuxKernels/GhostingFromUOAux

!syntax inputs /AuxKernels/GhostingFromUOAux

!syntax children /AuxKernels/GhostingFromUOAux

!bibtex bibliography
