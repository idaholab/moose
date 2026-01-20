# Growing Edge Crack

The XFEM module has several methods to grow cracks.  For 2D crack growth using [MeshCut2DFractureUserObject.md] see [this tests directory](https://github.com/idaholab/moose/tree/next/modules/xfem/test/tests/mesh_cut_2D_fracture/) and for 3D crack growth using [CrackMeshCut3DUserObject.md] see [this tests directory](https://github.com/idaholab/moose/tree/next/modules/xfem/test/tests/solid_mechanics_basic/).

# Double Cantilever Beam Center Crack

The double cantilever beam shown in [!ref](setup) with a running center crack is a common tests case for fracture algorithms [!citet](belytschko_elastic_1999).  The beam is initially cut with a center crack extending a distance $a$ into the beam.  At the tip of the initial prescribed crack is a segment that deviates from the main crack by small angle, $\theta$, with length $da$.  This offsets the crack tip from the centerline where the crack tip will experience mixed-mode loading (i.e., with nonzero mode-$I$ and mode-$II$ stress intensity factors $K_I$ and $K_{II}$, causing the crack to curve as it grows).  The crack growth direction is determined by the maximum hoop stress direction.  The input files for this test in [2D](https://github.com/idaholab/moose/tree/next/modules/xfem/test/tests/mesh_cut_2D_fracture/double_cantilever_crack_2d.i) and [3D](https://github.com/idaholab/moose/tree/next/modules/xfem/test/tests/solid_mechanics_basic/double_cantilever_crack.i) closely match the results given in Figure 9 of [!citet](belytschko_elastic_1999).  The growth increment used in the these input files was smaller than that given in the paper. This smaller increment was required to obtain a solution that matched that reported in the paper.  The results are shown in [!ref](3D_skew_view) and [!ref](3D_normal_view).

!media large_media/xfem/double_cantilever_setup.png
      style=width:70%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=setup
      caption=Double cantilever setup from [!citet](belytschko_elastic_1999).

!media large_media/xfem/double_cantilever_results_3D.png
      style=width:70%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=3D_skew_view
      caption=Showing results with cut geometry.  The red surface is the 3D xfem cutter mesh for the 3D simulation.  The blue line is the xfem cutter mesh for the 2D simulation.  The black dots are the results from [!citet](belytschko_elastic_1999) for $\theta=5.71^{\circ}$.

!media large_media/xfem/double_cantilever_results.png
      style=width:40%;margin:auto;padding-top:2.5%;background-color: white;color: black;
      id=3D_normal_view
      caption=Close-up view of the crack tip results projected normal to the crack plane.  The red dashed line is the 3D xfem cutter mesh for the 3D simulation.  The blue line is the xfem cutter mesh for the 2D simulation.  The black dots are the results from [!citet](belytschko_elastic_1999) for $\theta=5.71^{\circ}$.

!bibtex bibliography



