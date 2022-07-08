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
