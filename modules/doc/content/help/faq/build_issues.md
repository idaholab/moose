## Build Issues id=buildissues

Build issues are commonly caused by an invalid environment, an update to your repository (leading to
a mismatch between MOOSE and your application), or one of our MOOSE Conda packages being out of
date.

- Verify the Conda Environment is active and up to date, with the latest version of our moose
  packages:

  !versioner! code
  conda activate base
  conda env remove -n moose
  conda create -n moose moose-dev=__VERSIONER_CONDA_VERSION_MOOSE_DEV__
  conda activate moose
  !versioner-end!

  if `conda activate moose` failed, see [Conda Issues](help/troubleshooting.md#condaissues) above.

  !alert note
  Whenever an update is performed in Conda, an update should also be performed on your MOOSE
  repository, and vice versa. It is important to keep both of these in sync.

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
  Most users, do not use or set MOOSE_DIR. If the above command returns something, and you are not
  sure why, just unset it:

  ```bash
  unset MOOSE_DIR
  ```

- Try building a simple hello world example (there is more text than what is visible, be sure to
  copy it all):

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

  If the above build fails, and you have the correct Conda environment loaded
  (`conda activate moose`), then something is failing beyond the scope of this document, and you
  should now contact us via the [discussion forum](faq/discussion_forum.md).

  If the build was successful, attempt to execute the hello word example:

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

- Sometimes a build will fail due to running out of memory. When a build fails in this way, it is
  not always apparent. The compiler will simply die while not explaining why:

  ```bash
  make -j 8
  ...<trimmed>
  Compiling C++ (in opt mode) moose/framework/contrib/exodiff/FileInfo.C...
  Compiling C++ (in opt mode) moose/framework/contrib/exodiff/stringx.C...
  Compiling C++ (in opt mode) moose/framework/contrib/exodiff/iqsort.C...
  (standard input): Assembler message:
  (standard input): 488982: Warning: end of file not at end of a line; newline inserted
  x86_64-conda-linux-gnu-c++: fatal error: Killed signal terminated program cc1plus
  compilation terminated.
  make: *** [moose/framework/build.mk:150: moose/test/build/unity_src/object.x86_64-conda-linux-gnu.opt.lo] Error 1
  make: *** Waiting for unfinished jobs....
  ```

  If you are receiving a similar result, try reducing how many cores you are compiling with (try
  `make -j 4` instead). Each core consumes roughly 2GB of RAM (more if you are compiling in debug
  mode). Errors of this type are common among users who may be running on some form of virtual
  machine, or when operating within an HPC cluster with strict resource availability guidelines.

- If all of the above has succeeded, you should attempt to rebuild MOOSE or your application again.
  If you've made it this far, and the above is working, but MOOSE fails to build, then it is time to
  ask us why on the [discussion forum](faq/discussion_forum.md).
