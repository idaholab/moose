# INL HPC Cluster

The following instructions are for those operating on [!ac](INL) [!ac](HPC) machines. This includes
Bitterroot, Lemhi, and Sawtooth. It also includes the protected access hosts `rod` and `cone`.

Requesting access to [!ac](INL) [!ac](HPC) is handled by the [NCRC](https://inl.gov/ncrc/) group.

For information on running pre-built MOOSE-based NCRC applications (not the scope of this
document), please see [NCRC Applications](help/inl/applications.md) instead, and choose the
application applicable to you.

## Containerization

The development environment is deployed using containerization, which is a deployment
practice that bundles all application (i.e., MOOSE or a MOOSE-based app) dependencies need to run
on any host. This means that applications built within this environment can be executed
across all INL HPC clusters. For example, you can build an application within a compute node on
Sawtooth and execute the same executable of the application (providing you follow the instructions
that follow) on both Bitterroot and Lemhi.

## Versioning id=versioning

The version of the dependencies used within MOOSE (PETSc, libMesh, etc) are tied directly
to a given version of MOOSE. Given a version of MOOSE (or a MOOSE-based application), we produce
a version of the development environment that contains the correct version of these dependencies.
Whenever you are compiling MOOSE or a MOOSE-based application, you should use the corresponding
version of the development environment.

In order to determine the version of the development environment to use when building, run the
following within the MOOSE directory:

```bash
./scripts/versioner.py moose-dev
```

where the output will be a hashed version:

!versioner! code
__VERSIONER_VERSION_MOOSE_DEV__
!versioner-end!

In this case, the version of that should be used is [!versioner!version package=moose-dev].
This is the version that should be used for loading the containerized environment module that
follows.

## Loading the Environment

To access all MOOSE related modules, you must first load the `use.moose` module. The development
container modules are then located within the `moose-dev-MPI` modules, where `MPI` denotes the variant
of MPI is used. The version of these `moose-dev-MPI` modules used are the versions described by
the versioner script explained in the [#versioning] section. Currently, the only deployed variant is
`openmpi`, i.e. the development container module that should be used is `moose-dev-openmpi`.

With this, the containerized development environment is loaded with the following:

!versioner! code
module load use.moose moose-dev-openmpi/__VERSIONER_VERSION_MOOSE_DEV__
!versioner-end!

Take special note of the version applied to the `moose-dev-openmpi` module, which is
[!versioner!version package=moose-dev]. Again, this version comes from the versioner script
as described in [#versioning]. It is very important that you use the specific version that
is required by the application that you are building. You will receive warnings during the
compile process if the version is not correct.

!alert note
It is strongly recommended that you do not use default versions of the `moose-dev-openmpi`
module. Due to the fast-paced development of MOOSE, these development environments change often,
and thus the default version of `moose-dev-openmpi` changes often. In order to separate your
development from the fast-paced development of MOOSE, you should use specific module versions.

## Building id=building

The containerized environment needs to be entered in order to build or execute. For the purposes
of this documentation, we will use the version [!versioner!version package=moose-dev], although
yours could be different!

To enter the containerized shell environment to proceed with building, run the following
(note that here we are requesting the specific version of the environment that we need):

!versioner! code
module load use.moose moose-dev-openmpi/__VERSIONER_VERSION_MOOSE_DEV__
moose-dev-shell
!versioner-end!

You are now within the versioned, containerized environment. The standard
MOOSE dependencies (PETSc, libMesh, WASP, etc) are built and there is not a need to run
any of the update-and-rebuild scripts within MOOSE. You +must+ repeat the commands
above whenever you build any code.

At this point, we have assumed that you already have the source code for your application
available and are in the directory that contains that application. Build your application
and test it as normal:

```
make -j 4
./run_tests -j 4
```

## Running id=running

How your application is ran is dependent on whether or not you are running across multiple
hosts. In general, you are running across multiple hosts when you are running within a job
on HPC. We have two different methods of execution that we will reference later:

- Shell: Enters an interactive shell within the container where you can run multiple commands within the environment, can only be used on a single host
- Execute: Runs a single command within the container, required for running on multiple hosts

When running on a single host (i.e., not in a HPC job), enter the container in a shell as
done above and execute your application as normal:

!versioner! code
module load use.moose moose-dev-openmpi/__VERSIONER_VERSION_MOOSE_DEV__
moose-dev-shell
mpiexec -n 2 /path/to/your/application-opt -i ...
!versioner-end!

When you are running on multiple hosts (i.e., in an HPC job) you must use the "execute"
method to run your application as described above. This is done by appending the command
`moose-dev-exec` before the path to the binary of your application. That is, within an HPC
job submission script or within an HPC job interactive session, you will run your application
with:

!versioner! code
module load use.moose moose-dev-openmpi/__VERSIONER_VERSION_MOOSE_DEV__
mpiexec -n 2 moose-dev-exec /path/to/your/application-opt -i ...
!versioner-end!

Note that the "execute" method will also work on a single host.

!alert note
Due to an issue with OpenMPI (the MPI variant used within the environment that is described here),
you must always specify the number of cores to run on (i.e., `mpiexec -n 2` for two cores) when
running with MPI.

## OnDemand Execution

You can utilize HPC OnDemand to enter an interactive code server (similar to VSCode) window
on a compute host within the same containerized development environment that was previously
described.

Proceed to the "VSCode Container" IDE application HPC OnDemand. If you are within the internal INL
network (on an INL device), you can access this application
[here](https://ondemand.hpc.inl.gov/pun/sys/dashboard/batch_connect/sys/vscode-container/session_contexts/new).
If you are an external (non-INL) user, you can access this application
[here](https://hpcondemand.inl.gov/pun/sys/dashboard/batch_connect/sys/vscode-container/session_contexts/new).

On the HPC OnDemand application page described above, take note of the "Module" dropdown box.
Here you should see as an option moose-dev-openmpi/[!versioner!version package=moose-dev]. This module
listing should contain the same module that you would have loaded within the [#building] and
[#running] steps described above. Note again that the specific version that you need may
not necessarily be [!versioner!version package=moose-dev], in which case you should reference the
[#versioning] step to obtain the proper version given your application.

Set the other options on the application page as desired. We suggest the following:

- Cores: At least four, although you are free to request more as needed.
- Working directory: The root directory of your application source. When developing/running on HPC, you should use your personal scratch storage over your home directory storage as scratch offers better performance.

Next, launch the job using the Launch button and connect to the instance once it is running. It will
sometimes take a minute or two for the connection to succeed.

Once you have entered the code server window (the window that looks similar to that of VSCode),
you may execute within the container using a terminal window in the instance. While within
an OnDemand code server instance, there is no need to execute any of the `moose-dev-shell` or
`moose-dev-exec` commands as described in the [#running] step. The code server window
is ran within the container, thus you can run commands normally, like:

```
make -j 6
mpiexec -n 6 /path/to/myapp-opt -i ...
```
