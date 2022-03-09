# SolutionRasterizer

!syntax description /UserObjects/SolutionRasterizer

Mesoscale microstructure simulations using the phase field method can be utilized to generate initial structures for large sacle molecular dynamics simulations. One application is the construction of nano-foam atomistic samples from simulations of spinodal decomposition.

!media media/phase_field/solutionrasterizer.png style=width:30%;padding-left:20px;float:right; caption=Nano foam atomistic configuration generated using the `SolutionRasterizer`.

MOOSE provides a built in system to template an atomic coordinate file in the `.xyz` format from the simulation result in a given mesh file. The workflow is as follows

1. Set up a phase field simulation and output a finite element mesh file
2. Create a bulk `.xyz` input file with the desired underlying lattice and crystal structure
3. Use the minimal input file shown below to load the mesh and `.xyz` input

MOOSE will load the `.xyz` input and finite element mesh superpose both and use thresholding to carve out _rejected_ atoms while retaining _accepted_ atoms. Finally a new `.xyz` output is written containing only the _accepted_ atoms, generating a porous microstructure.

The `SolutionRasterizer` user object is derived from the [`SolutionUserObject`](/wiki/MooseSystems/UserObjects/SolutionUserObject) and inherits its input parameters. It takes the following additional input parameters:

|Input Parameter | Description|
|----------------|------------|
|`xyz_input`     | An `.xyz` file as an input file. This input file should contain an atomistic simulation cell completely filled with atoms of a chosen crystal structure (a polycrystalline sample can be supplied). |
|`xyz_output`    | **Output file** containing the filtered (or mapped) atomic coordinate file. |
| `raster_mode = FILTER`  | In `FILTER` mode the `SolutionRasterizer` uses thresholding on a specified non-linear variable to reject or accept atom from the input file to pass to the output `.xyz` file.|
| `raster_mode = MAP`     | In `MAP` mode the `SolutionRasterizer` takes the atoms from the input file and add an additional column to the data set containing the value of a specified non-linear variable.|
| `threshold`    | Value of the selected non-linear variable to use in `FILTER` mode used to _accept_ or _reject_ atoms from the `xyz_input` file.|

### Minimal working input file

The following input can be executed with the MOOSE Phase Field module executable and will

- load the mesh file `diffuse_out.e`
- load the atomic coordinate file `in.xyz`
- accept all atoms at the coordinates where the value of the non-linear variable `c` is above a value of `0.5`
- write out the atomic coordinate file `out.xyz`

!listing modules/phase_field/test/tests/solution_rasterizer/raster.i

!syntax parameters /UserObjects/SolutionRasterizer

!syntax inputs /UserObjects/SolutionRasterizer

!syntax children /UserObjects/SolutionRasterizer
