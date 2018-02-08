# NodalNormals System

Nodal normals system allows users to use normals at node-based systems such as nodal boundary
conditions.  The basic question in defining nodal normals is how they are defined, because there
is multiple ways how to do so.  The nodal normal system in MOOSE is using a method that is based
on mass conservation as described in \cite{FLD:FLD1650020302}.

To activate the system, introduce a top level `[NodalNormals]` block. Then, define multiple
boundaries where the nodal normals should be computed. That is done by listing a sub-block with
a unique name, see \ref{main_structure} for an example.

!listing id=main_structure caption=Main structure of the nodal normals block
```
[NodalNormals]
  [./nodal_normals_left]
    parameters
  [../]

  [./nodal_normals_right]
    parameters
  [../]
[]
```

Then to use the nodal normals, use the name of the block in a object that works with nodal normals,
see \ref{usage}

!listing id=usage caption=Main structure of the nodal normals block
```
[BCs]
  [./nodal_bc]
    type = NodalNormalBC
    boundary = left
    nodal_normals = nodal_normals_left
    ...
  [../]
[]
```


## Defining Nodal Normals

The nodal normals sub-block requires `boundary` parameter, which is the boundary where nodal normals
will be available.


## Computing Nodal Normals on Internal Sideset

Nodal normals can be computed on an internal sideset. Additionally, a `block` parameter has to
specified and the normals will be pointing outwards from this block, see \ref{internal_sideset} for
syntax.

!listing id=internal_sideset caption=Example for computing nodal normals on an internal sideset
```
[NodalNormals]
  [./internal_left]
    boundary = internal_side_set_name
    block = block_name
  [../]
[]
```

See \ref{complete_internal_sideset_example} for a complete example

!listing test/tests/bcs/nodal_normals/internal_sideset.i id=complete_internal_sideset_example caption=Complete example of nodal normals restricted to an internal sideset start=Mesh end=Outputs

## Nodal Normals at Corners

It is often the case that nodal normals are restricted to a subsection of a boundary. In this case,
the normals at the ends of the boundary may not be pointing in the desired direction.

!listing id=corners caption=Correction of normals at corners
```
  (no correction)       (with correction)

   \
     *---*                 <- *---*
     |   |                    |   |
  <- *---*                 <- *---*
     |   |                    |   |
     *---*                 <- *---*
    /
```

The \ref{corners} compares a situation where nodal normals are restricted to the left side with and
without the correction applied.  Typically, we want the normals pointing horizontally, thus we
need to apply the correction. That is done by specifying a `corner_boundary` parameters, which is
the name of nodeset that contains the two end nodes.  This can be done via a [`AddExtraNodeset`](/AddExtraNodeset.md)
mesh modifier or can be specified at the time the mesh is built in an external application.


## Recomputing Nodal Normals

By default, the nodal normals are computed just once at the beginning of the simulation. This
behavior can be changed by specifying `execute_on` parameter.


## Visualization of Normal Normals

To visualize nodal normals, use the [`NodalNormalsAux`](/NodalNormalAux.md) kernel.  Define one
Lagrange variable per component and let this kernel act on it. See [`NodalNormalsAux`](/NodalNormalAux.md)
for a complete example. Note, that if you have more than one nodal normal at a node, you will need
as many nodal variables for visualizing them.


!syntax parameters /NodalNormals/AddNodalNormalsAction

!syntax actions /NodalNormals

##References

\bibliographystyle{unsrt}
\bibliography{nodal_normals.bib}
