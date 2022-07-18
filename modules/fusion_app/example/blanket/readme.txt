It is a 3D heat transfer simulation for a fusion blanket. The heat transfer is modeled as
two essential components: heat conduction on structures and 1D thermal-hydraulics for
cooling channels. The two components are coupled together via the MOOSE multiapp system.
The coupling is based on Picard iteration. The data transfer (temperature and heat transfer
coefficient) between the components is realized via a customized transfer object,
MultiAppMapNearestNodeTransfer.

In order to run this simulation, you must first build a fusion application. The building
 process is similar to any other moose-based application. To run this example, there are two steps.

Step 1 (prepare mesh):

mpirun -n 20 ../../fusion_app-opt -i blanket_heat_mesh.i create_bdries_from_blocks.i --mesh-only

This step is used to prepare a mesh that will be used for 3D heat transfer simulation.
Essentially, it generates boundaries and deletes extra mesh blocks. The output of this
step is a mesh file, create_bdries_from_blocks_in.e

Notice that this step should be only run once unless the original CAD changes. The same
mesh file should be reused for the following simulation as much as possible.

Step 2 (3D heat transfer simulation):

mpirun -n 20 ../../fusion_app-opt -i  blanket_heat_transfer.i create_bdry_blocks.i

This step takes a mesh file generated in step 1 and simulates heat transfer for a blanket.





More details:

1. Mesh generation

Mesh generation is done using Cubit with CAD as an input. CAD is generated using
a cubit-based python script, FNSF_cubit_design_files/FNSF-CB.py.

Once CAD is ready, import the CAD into cubit, and use the following steps to generate meshes:

1) Merge volume all

This command will merge all common surfaces so that the generated meshes are consistent across
the interfaces.

2) Select meshing schemes via GUI

  Select Tet and apply scheme
  Select Auto and apply scheme

3) Mesh volume all

This command will try to generate a volume mesh for the blanket


Once a mesh is generated, you can go back to step 1 and step 2 to do a thermal simulation


2. Input files

Overall, the thermal-hydraulics simulation for the entire fusion blanket is complicated, and
seven input files are developed to achieve this goal. We briefly describe each input file below.

blanket_heat_mesh.i: The main input file for mesh preparation reads in the blanket mesh generated
by cubit. This input file will be combined with another input file, create_bdries_from_blocks.i

create_bdries_from_blocks.i: An assistance input file for mesh preparation. The main content of this
input file includes many 3D channel boundary generators (SideSetsBetweenSubdomainsGenerator). The
generators take the blanket structures and 3D volume channels as inputs, and generate 3D channel boundaries.
After the boundary generation, all the 3D volume channels will be deleted. 3D channel boundaries will
be coupled to 1D thermal-hydraulics simulation later. This input file is generated using create_bdries_from_blocks.py


blanket_heat_transfer.i: The core thermal simulation input file for the fusion blanket. 3D heat
conduction is implemented for blanket structures, and the cooling channels are handled by using
1D thermal-hydraulics simulation. The coupling between the heat conduction and the 1D thermal-hydraulics
simulation is achieved using the MOOSE multiapp system with a customized transfer. The customized transfer
(MultiAppMapNearestNodeTransfer) is located in this repo. 1D THM simulations involved in this input file
are implemented in another three input files: channel_fw.i, channel_plate.i and channel_shell.i

create_bdry_blocks.i: A assistance input file for blanket_heat_transfer.i. This input file is generated using
a python script, create_bdry_blocks.py. The input file is employed to represent all the 3D channel boundaries.
The motivation is to have a clean core input file.

channel_fw.i, channel_plate.i and channel_shell.i: Main input files for 1D THM simulations. These files will
be called by blanket_heat_transfer.i. You should not run these input files directly unless you are debugging.
The three input files are for channels on the FW, the plate and the shell, respectively. The input files
need new 1phase flow object, FilePipe1Phase, that is able to read in a CSV file mesh and build 1THM simulation
for curved pipes. FilePipe1Phase is located in this repo.

cliargs_fw.txt, cliargs_plate.txt, and cliargs_shell.txt: Assistance command argument files. The files are
employed to setup an individual  mesh file and the corresponding 3D channel boundary for each 1D THM simulation.
These input files are generated using cliargs.py

positions_fw.txt, positions_plate.txt, and positions_shell.txt: Position (offset) files for 1D THM simulations.
The positions are trivial (0,0,0) because we use the actual 1D meshes with real coordinates and do not
need to offset these locations. The position files are generated using positions.py.
