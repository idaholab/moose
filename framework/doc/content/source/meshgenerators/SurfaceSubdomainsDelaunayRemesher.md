# SurfaceSubdomainsDelaunayRemesher

!syntax description /Mesh/SurfaceSubdomainsDelaunayRemesher

## Overview

This `SurfaceSubdomainsDelaunayRemesher` utilizes the 2D triangulation capabilities in the XY-plane, which is adopted by [`XYDelaunayGenerator`](XYDelaunayGenerator.md) and extends it to support more generalized 2D surface cases. The input of this generator is not limited to a 2D mesh residing in the XY-plane. Instead, it can accept a 2D mesh with curvature in the 3D space.
The subdomains that will be remeshed must be such that all their elements are sufficiently aligned with the average surface normal of the subdomain.

The input of this mesh generator is defined by a surface mesh name ([!param](/Mesh/SurfaceSubdomainsDelaunayRemesher/input)) along with a list of subdomains names ([!param](/Mesh/SurfaceSubdomainsDelaunayRemesher/included_subdomains) parameter) within it. The generator will extract the specified subdomains from the input mesh and generate a 2D mesh through Delaunay triangulation within their (subdomain) boundary.

## Methods

The same methods adopted by [`XYDelaunayGenerator`](XYDelaunayGenerator.md) are called by this mesh generator. To support a generalized 2D surface mesh, the following algorithm is implemented. The algorithm is similar to the one used in the [Boundary2DDelaunayGenerator.md]. This process is illustrated for a single subdomain (see [bdry_2d_delaunay]a)

- Each surface mesh subdomain is extracted to form its own surface mesh.
- An average normal vector is computed based on the normal vectors of the subdomain elements weighted by their areas.
- The surface mesh is then rotated and translated so that its centroid is on the XY-plane and the average normal vector is aligned with the Z-axis.
- The surface mesh is projected onto the XY-plane so that the triangulation method can be applied (see [bdry_2d_delaunay]c).
- A new mesh in the XY-plane is generated through Delaunay triangulation (same as [`XYDelaunayGenerator`](XYDelaunayGenerator.md))(see [bdry_2d_delaunay]d).
- The new surface mesh is reversely projected back to the original 3D space based on the original 2D subdomain surface.
- The new surface mesh is translated and rotated back to the original position of the 2D surface (see [bdry_2d_delaunay]e).
- Optionally, the nodes in the new 2D mesh can be corrected using a provided level set function of the surface ([!param](/Mesh/SurfaceSubdomainsDelaunayRemesher/level_set))(see [bdry_2d_delaunay]f).
- The newly generated subdomain surface meshes are stitched one by one to the first newly generated surface mesh.

!media framework/meshgenerators/bdry_2d_delaunay.png
      style=width:80%;
      id=bdry_2d_delaunay
      caption=The workflow of the `SurfaceSubdomainsDelaunayGenerator` mesh generator for a single subdomain.

As projection is used to enable the triangulation of the 2D subdomain mesh surface, it is crucial to limit the angle deviation of the element normals in the surface subdomains from the average normal vector of the surface. This is controlled by the [!param](/Mesh/SurfaceSubdomainsDelaunayRemesher/max_angle_deviation) parameter. Deviation must be less than 90 degrees to avert overlapping elements. The projection also introduces errors in the element area control. The actual element area to be controlled are the projected areas of the elements in the surface mesh.

## Application

One common application of this mesh generator is to prepare 2D surface meshes to be used as input for 3D Delaunay mesh generation([XYZelaunayGenerator](XYZDelaunayGenerator.md)). For a raw 3D mesh with a closed 2D surface, the 2D surface can be divided into multiple regions to be compatible with the triangulation method with projection. `SurfaceSubdomainsDelaunayRemesher` can be used to generate the 2D surface mesh with better mesh density and quality control.

!syntax parameters /Mesh/SurfaceSubdomainsDelaunayRemesher

!syntax inputs /Mesh/SurfaceSubdomainsDelaunayRemesher

!syntax children /Mesh/SurfaceSubdomainsDelaunayRemesher
