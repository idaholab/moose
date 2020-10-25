#!/bin/bash

NCPU="8"
CHECKPOINT_FILE_NAME="testfile.cpa"
MOOSE_TEST_EXEXCUTIONER_TYPE="dbg"


## this is tthe first step taht normally isn't needed, it simply generates a single exodus file that will be used by step 2
$MOOSE_DIR/test/moose_test-$MOOSE_TEST_EXEXCUTIONER_TYPE -i simple_diffusion_test_mesh.i --mesh-only

##STEP 2 and 3 below can now be aggreaagated in one step, it seems safe to do. If you are extra paranoid you can use them instead of step 2-3

# ## Step 2: here we are breaking the mesh in SERIAL and saving info we will need later
# $MOOSE_DIR/test/moose_test-$MOOSE_TEST_EXEXCUTIONER_TYPE -i simple_diffusion_test_mesh_break.i


# ## Step 3: actual mesh split, the generated checkpoint files  will be used to run the simualtion
# $MOOSE_DIR/test/moose_test-$MOOSE_TEST_EXEXCUTIONER_TYPE -i simple_diffusion_test_mesh_prepare_split.i Mesh/msh/file=simple_diffusion_test_mesh_break_out.e Mesh/msh/exodus_extra_element_integers='bmbb_element_id' --split-mesh $NCPU --split-file $CHECKPOINT_FILE_NAME

## Step 2-3: break and split the mesh this seems safe
$MOOSE_DIR/test/moose_test-$MOOSE_TEST_EXEXCUTIONER_TYPE -i simple_diffusion_test_mesh_break.i --split-mesh $NCPU --split-file $CHECKPOINT_FILE_NAME

##Step 4: runnign the simulation for the first time
mpirun -np $NCPU $MOOSE_DIR/test/moose_test-$MOOSE_TEST_EXEXCUTIONER_TYPE -i simple_diffusion_test_mesh_prepare_split.i --distributed-mesh Mesh/msh/file=$CHECKPOINT_FILE_NAME Outputs/nemesis=true Outputs/checkpoint=true

##Step 5: restarting the simulation using the msae input file, notice the problem paramter to relaod stateful var
RESTART_CHECKPOINT_FILE_LOCATION="simple_diffusion_test_mesh_prepare_split_out_cp"
CHECKPOINT_ID="0001"
mpirun -np $NCPU $MOOSE_DIR/test/moose_test-$MOOSE_TEST_EXEXCUTIONER_TYPE -i simple_diffusion_test_mesh_prepare_split.i --distributed-mesh Mesh/msh/file=$RESTART_CHECKPOINT_FILE_LOCATION/${CHECKPOINT_ID}_mesh.cpr  Problem/restart_file_base=$RESTART_CHECKPOINT_FILE_LOCATION/$CHECKPOINT_ID Outputs/nemesis=true Outputs/checkpoint=true Outputs/file_base="restart"

## STEP 5a and 5b are two restart procedure you should not try!!!!! In way way or another tehy will broak teh output. hence they are commented

##Step 5b: restartwith same output file name: DON'T DO THIS, data in the exodus file will be lost
# RESTART_CHECKPOINT_FILE_LOCATION="simple_diffusion_test_mesh_prepare_split_out_cp"
# CHECKPOINT_ID="0001"
# mpirun -np $NCPU $MOOSE_DIR/test/moose_test-$MOOSE_TEST_EXEXCUTIONER_TYPE -i simple_diffusion_test_mesh_prepare_split.i --distributed-mesh Mesh/msh/file=$RESTART_CHECKPOINT_FILE_LOCATION/${CHECKPOINT_ID}_mesh.cpr  Problem/restart_file_base=$RESTART_CHECKPOINT_FILE_LOCATION/$CHECKPOINT_ID Outputs/nemesis=true Outputs/checkpoint=true

##Step 5c: recover  same output file name: DON'T USE RECOVER, things get lost as we can't reconnect the mesh
# RESTART_CHECKPOINT_FILE_LOCATION="simple_diffusion_test_mesh_prepare_split_out_cp"
# CHECKPOINT_ID="0003"
# mpirun -np $NCPU $MOOSE_DIR/test/moose_test-$MOOSE_TEST_EXEXCUTIONER_TYPE -i simple_diffusion_test_mesh_prepare_split.i --distributed-mesh Mesh/msh/file=$RESTART_CHECKPOINT_FILE_LOCATION/${CHECKPOINT_ID}_mesh.cpr Outputs/nemesis=true Outputs/checkpoint=true --recover


##Step 6, comparison with monolitic mesh
mpirun -np $NCPU $MOOSE_DIR/test/moose_test-$MOOSE_TEST_EXEXCUTIONER_TYPE -i simple_diffusion_monolithic.i
