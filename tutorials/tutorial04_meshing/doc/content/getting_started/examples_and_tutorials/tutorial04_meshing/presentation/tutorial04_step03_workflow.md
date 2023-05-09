# Meshing Workflow in MOOSE

!---

## Constructing a Reactor Mesh Hierarchically

MOOSE's Mesh System and Reactor Module contains numerous "MeshGenerator" objects which either (a) create a mesh from scratch or (b) perform operations on existing meshes. To create a mesh, the user must define a sequence of MeshGenerator object calls inside the `[Mesh]` block to construct a geometry beginning with the smallest components (pins) and building up to larger components (core). For example, after pins are defined, they can be patterned into an assembly, and assemblies can then be patterned into a core. Application of a peripheral core zone,
trimming along symmetry lines and extrusion to 3D are optional in the final steps.

The meshing workflow for a standard reactor core follows the general hierarchical process of identifying key features in the geometry and building them hierarchically in terms of smallest to largest (pins, assemblies, core, core periphery):

1. Define pins
2. Define assembly by patterning existing pins into a lattice and adding coolant background and/or duct region
3. Define core by patterning assemblies into a lattice
4. Apply core periphery zones like a circular shield
5. Trim along lines of symmetry to reduce computational expense
6. Extrude to 3D

!---

## Execution

Use any executable with the Reactor module compiled, such as  or `$MOOSE_DIR/modules/reactor-opt` along with the `--mesh-only` command line option:

```bash
$MOOSE_DIR/modules/reactor/reactor-opt -i <my_meshing_input.i> --mesh-only
```

The `--mesh-only` optional command line parameter executes only the `[Mesh]` block of the input file and outputs the generated mesh. This is useful while building and testing the mesh as it doesn't require the rest of the MOOSE problem be defined in the input file. Recent updates to the `--mesh-only` option now allow the mesh output in Exodus format to include the extra element integer IDs defined on the mesh by default for convenient visualization purposes.

When you are satisfied with your mesh input, you may invoke MOOSE without the `--mesh-only` option to execute the entire input file (mesh building and physics input).

Executables of any MOOSE applications that contain the Reactor module in their `Makefile` can also be used, such as Griffin, Sockeye, Pronghorn, Cardinal, and BlueCRAB.

!---

## Visualization with ParaView

The [Exodus](outputs/Exodus.md) output format is the preferred way to write out simulation results from MOOSE simulations. This format is supported by [ParaView](https://www.paraview.org/), [VisIt](https://visit-dav.github.io/visit-website/), and other postprocessing applications. ParaView is most commonly used, but the visualization procedure is similar for other programs.

To save a lot of clicks, the following settings are recommended (in `Edit`->`Settings`):

!row!
!col small=12 medium=6 large=8

- +Auto Apply+: Automatically apply changes in the 'Properties' panel
- +Load All Variables+: Load all variables when loading a data set
- +Default Time Step+: Go to last timestep

!col small=12 medium=6 large=4

!media tutorial04_meshing/paraview-defaults.png
       id=tutorial04-paraview-defaults
       caption=Recommended default settings in ParaView. [!cite](ParaView2005)
       style=width:40%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

To visually inspect a mesh, first load the Exodus output file (ending in `.e`) into ParaView. Select the `Open` button in the top left corner, browse to the Exodus mesh file, click `OK`, and click the `Apply` button in the Properties dialogue in the lower left corner.

!media tutorial04_meshing/paraview-open.png
       id=tutorial04-paraview-open
       caption=Open Exodus mesh file in ParaView. [!cite](ParaView2005)
       style=width:70%;display:block;margin-left:auto;margin-right:auto;

!---

With the mesh loaded, there are two key visualization options in the top center of the menu: the visualization style on the right, which is good to set to +Surface with Edges+ to show where the element boundaries are located, and the visualization property on the left, which can be switched between the various properties defined on the mesh.

!media tutorial04_meshing/paraview-vis-selectors.png
       id=tutorial04-paraview-vis-selectors
       caption=ParaView visualization selectors. [!cite](ParaView2005)
       style=width:70%;display:block;margin-left:auto;margin-right:auto;

!---

There are also a variety of toggles in the Properties dialog in the lower left corner, which can control which elements of the mesh are visualized. After any modification of the properties, be sure to click `Apply` for the changes to apply.

!media tutorial04_meshing/paraview-properties.png
       id=tutorial04-paraview-properties
       caption=ParaView properties dialogue. [!cite](ParaView2005)
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!---

In this example, the periphery block was removed and outer core side set is highlighted (in green).

!media tutorial04_meshing/paraview-example.png
       id=tutorial04-paraview-example
       caption=Example of selected blocks/sidesets. [!cite](ParaView2005)
       style=width:60%;display:block;margin-left:auto;margin-right:auto;
