# PatchSidesetGenerator

## Description

This mesh generator splits a given sideset (`sideset` parameter) into `n` pieces (`n_patches`).
The pieces are referred to as patches. Patches are used for net radiation transfer method via
view factors. The sideset is divided into `n` patches using partitioner that are available from
libmesh. The new sidesets are named `<old_side_name>_<id>`, where `<id>` is a number running from
`0` to `n - 1`.

In addition to libmesh partitioners, this mesh generator also supports a grid partitioner. The
grid partitioner superimposes a 3-dimensional (where $d$ is the dimensionality of the problem) uniform, orthogonal grid
over the sideset and partitions the elements according to where the centroids of the
sideset faces fall on the orthogonal grid. The superimposed grid is congruent to the bounding box of the sideset to ensure
that the sideset is completely encompassed within the superimposed grid.

The superimposed orthogonal grid is _always_ 3-dimensional because the sideset (surface in 3D and line in 2D problems)
does not need to be orthogonal to any of the coordinate axes. The superimposed 3-dimensional mesh is aligned with the coordinate
axis and will have a single element in 1 (2) directions for 3 (2)-dimensional problems. These are discussed in turn:

- for 2-dimensional problems, sidsets are a collection of line segments so sidesets are 1-dimensional. The bounding box of the sideset
  is computed and the largest distance along any of the three axes is computed (i.e. $\max (x_{\text{max}}-x_{\text{min}}, y_{\text{max}}-z_{\text{min}},x_{\text{max}}-z_{\text{min}})$). The superimposed mesh has `n_patches` subdivisions along the longest axis and a single
  subdivision along the two shorter axes.
- for 3-dimensional problems, sidsets are a collection of faces so sidesets are 2-dimensional. The bounding box of the sideset
  is computed and the two largest distance along any of the three axes is computed (i.e. two largest from set $\{x_{\text{max}}-x_{\text{min}}, y_{\text{max}}-z_{\text{min}},x_{\text{max}}-z_{\text{min}}\}$). The superimposed mesh has a single subdivision along the axes with the smallest extent.
  The number of elements along the two larger dimensions are determined as follows:

  \begin{equation}
  \begin{aligned}
    n_1 &= \lfloor \sqrt{\frac{\Delta_1}{\Delta_2} n_p} \rfloor \\
    n_2 &= \lfloor \sqrt{\frac{\Delta_2}{\Delta_1} n_p} \rfloor,
  \end{aligned}  
  \end{equation}

  where $n_1$ and $n_2$ are the subdivisions along the longest and second longest axes and $n_p$ is equal to `n_patches`. Due to the rounding,
  the $n_1 \cdot n_2$ does not need to be equal to `n_patches` so the number of patches might be adjusted. A warning is printed if that happens.

## Example Input Syntax

!listing modules/heat_conduction/test/tests/generate_radiation_patch/generate_radiation_patch.i start=[patch] end=[] include-end=true

!syntax parameters /Mesh/PatchSidesetGenerator

!syntax inputs /Mesh/PatchSidesetGenerator

!syntax children /Mesh/PatchSidesetGenerator
