## Build Issues id=buildissues

Build issues are normally caused by an invalid environment, or perhaps an update to your repository occurred, and you now have a mismatch between MOOSE and your application, or a combination of the two with the moose-libmesh Conda package being out of date.

- Verify the Conda Environment is active and up to date, with the latest version of our moose packages:

  ```bash
  mamba activate moose
  mamba update --all
  ```

  if `mamba activate moose` failed, see [Conda Issues](troubleshooting.md#condaissues) above.

  !alert note
  When ever an update is performed in Conda, it is a good idea to re-build MOOSE and your application. While specific updates to moose-libmesh and/or moose-petsc may not have occurred, there are several other libraries out of our control which may have been upgraded, requiring you to rebuild.

- Verify the MOOSE repository is up to date, with the correct vetted version of libMesh:

  !alert warning
  Be sure you have committed/saved your work. The following commands will delete untracked files!

  ```bash
  cd moose
  git checkout master
  git clean -xfd

  <output snipped>

  git fetch upstream
  git pull
  git submodule update --init
  ```

- Verify you either have no moose directory set, or it is set correctly.

  ```bash
  [~] > echo $MOOSE_DIR

  [~] >
  ```

  The above should return nothing, or it should point to the correct moose repository.

  !alert note
  Most users, do not use or set MOOSE_DIR. If the above command returns something, and you are not sure why, just unset it:

  ```bash
  unset MOOSE_DIR
  ```

- Try building a simple hello world example (there is more text than what is visible, be sure to copy it all):

  ```bash
  cd /tmp
  cat << EOF > hello.C
  #include <mpi.h>
  #include <stdio.h>

  int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);

    // Finalize the MPI environment.
    MPI_Finalize();
  }
  EOF

  mpicxx -fopenmp hello.C
  ```

  If the above build fails, and you have the correct Conda environment loaded (`mamba activate moose`), then something is failing beyond the scope of this document, and you should now contact us via the [disussion forum](faq/discussion_forum.md).

  If the build was successfull, attempt to execute the hello word example:

  ```bash
  mpiexec -n 4 /tmp/a.out
  ```

  You should receive a response similar to the following:

  ```bash
  Hello world from processor my_hostname, rank 0 out of 4 processors
  Hello world from processor my_hostname, rank 1 out of 4 processors
  Hello world from processor my_hostname, rank 3 out of 4 processors
  Hello world from processor my_hostname, rank 2 out of 4 processors
  ```

- If all of the above has succeeded, you should attempt to rebuild MOOSE or your application again. If you've made it this far, and the above is working, but MOOSE fails to build, then it is time to ask us why on the [discussion forum](faq/discussion_forum.md).
