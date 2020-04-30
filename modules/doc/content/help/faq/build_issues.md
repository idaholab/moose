## Build Issues id=buildissues

Build issues are normally caused by an invalid environment, or perhaps an update to your repository occurred, and you now have a mismatch between MOOSE and your application, or a combination of one or the other with libMesh's submodule.

- Verify you have a functional [Modules](help/troubleshooting.md#modules) environment.

- Verify the MOOSE repository is up to date, with the correct vetted version of libMesh:

  !alert warning
  Before performing the following commands, be sure you have committed your work. Because... we are about to delete stuff!

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

  The above should return nothing, or, it should point to the correct moose repository.

  !alert note
  Most users, do not use or set MOOSE_DIR. If the above command returns something, and you are not sure why, just unset it:

  ```bash
  unset MOOSE_DIR
  ```

- Try building a simple hello world example (copy and paste the entire box):

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

  If the build failed, and you have the correct [Modules](help/troubleshooting.md#modules) environment loaded, then you should attempt to perform the 'Uggh! None of this is working' step in the [Modules](help/troubleshooting.md#modules) section. As it would seem, there is something else in your environment that is inhibiting your ability to compile simple programs.

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

- If all of the above has succeeded, you should attempt to [rebuild libMesh](help/troubleshooting.md#libmesh) again.
