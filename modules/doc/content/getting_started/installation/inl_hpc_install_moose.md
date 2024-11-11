# INL HPC Cluster

The following instructions are for those operating on [!ac](INL) [!ac](HPC) machines. This includes
Bitterroot, Lemhi, and Sawtooth. It also includes the protected access hosts `rod` and `cone`.

Requesting access to [!ac](INL) [!ac](HPC) is handled by the [NCRC](https://inl.gov/ncrc/) group.
Once access has been granted, one can use [HPC OnDemand](hpc_ondemand.md) services to gain access to
a shell.

For information on running pre-built MOOSE-based NCRC applications (not the scope of this
document), please see [NCRC Applications](help/inl/applications.md) instead, and choose the
application applicable to you.

At the time of this writing, the following [!ac](INL) [!ac](HPC) clusters are available for use:

!include ncrc/hpc_cluster_information.md

!alert! warning title=+In order to git-clone a repository:+
You +*MUST*+ be on one of the above login nodes! The following instructions assume you are looking
at an interactive shell prompt on one of the above listed hosts, awaiting your commands.
!alert-end!

# Containerization

!style halign=left
The development environment is deployed using containerization, which is a deployment
practice that bundles all application and application dependencies (i.e., MOOSE or a MOOSE-based
app), needed to run on any host. This means that applications built within this environment can be
executed across all [!ac](INL) [!ac](HPC) clusters. For example, you can build an application within
a compute node on Sawtooth and execute the same executable of the application (providing you follow
the instructions that follow) on both Bitterroot and Lemhi.

## Obtain MOOSE

!style halign=left
If you have not yet cloned MOOSE or an application that uses moose, do this now according to that
application's instructions. Otherwise, skip to
[Loading the Environment](inl_hpc_install_moose.md#envloading) below.

!alert! tip title=Use `/scratch` while using INL HPC resources
Use the available `/scratch/<your INL HPC User ID>/` location whenever possible. The scratch volume
offers higher performance than what exists for your `/home/` directory.
!alert-end!

!template load file=installation/clone_moose.md.template PATH=/scratch/<your INL HPC User ID>/projects

## Loading the Environment id=envloading

!style halign=left
Figuring out the right environment to load is a two-step process involving our Versioner script,
and `module load` commands:

### Versioner id=versioning

!style halign=left
To aid you in selecting the right container to use at any given commit, we created a script called
`versioner.py`. Versioner returns a hash which we use as a naming suffix for the module you will be
loading. Your copy of MOOSE, and all the accompanying submodules (recursively), contribute to the
generation of this hash.

Enter the `moose` directory where ever that may be, and run the following command (the following
assumes you cloned MOOSE as directed in the previous step):

```bash
cd /scratch/<your INL HPC User ID>/projects/moose
# or cd to your application/moose directory
./scripts/versioner.py moose-dev
```

where the output will be a hashed version:

!versioner! code
__VERSIONER_VERSION_MOOSE_DEV__
!versioner-end!

In this case, the version that should be used is [!versioner!version package=moose-dev].
This is the version that should be used for loading the containerized environment module that
follows.

### Module Load

!style halign=left
To access any MOOSE related module, you must first load the `use.moose` module. The development
container modules are then located within the `moose-dev-MPI` modules, where `MPI` denotes the
variant of MPI is used. The version of these `moose-dev-MPI` modules used are the versions described
by the versioner script explained in the [#versioning] section. Currently, the only deployed variant
is `openmpi`, i.e. the development container module that should be used is `moose-dev-openmpi`.

With this, the containerized development environment is loaded with the following command:

!versioner! code
module load use.moose moose-dev-openmpi/__VERSIONER_VERSION_MOOSE_DEV__
!versioner-end!

Take special note of the version applied to the `moose-dev-openmpi` module, which is
[!versioner!version package=moose-dev]. Again, this version comes from the versioner script
as described in [#versioning]. It is very important that you use the specific version that
is required by the application that you are building. You will receive warnings (or worse) during
the compile process if the version is not correct.

!alert warning title=+loading moose-dev-openmpi without a version+
It is strongly recommended that you +NOT+ load `moose-dev-openmpi` without a version suffix. Doing
so invokes the default action of loading the latest available version (new versions are added
multiple times, daily).

### Environment Recap

!style halign=left
You +must+ repeat the above versioning/module-loading technique, whenever you are performing
MOOSE based work:

1. figure out the version you need:

  `moose/scripts/versioner.py moose-dev`


2. load the matching module version:

  `module load use.moose moose-dev-openmpi/<results of step 1>`

## Building Application id=building

!style halign=left
The containerized environment needs to be entered into in order to build or run. For the
purposes of this documentation, we will use the version [!versioner!version package=moose-dev],
although yours could be different!

To enter the containerized shell environment to proceed with building, run the following
(note that here we are requesting the specific version of the environment that we need):

!versioner! code
module load use.moose moose-dev-openmpi/__VERSIONER_VERSION_MOOSE_DEV__
moose-dev-shell
!versioner-end!

You are now within the versioned, containerized environment. The standard MOOSE dependencies (PETSc,
libMesh, WASP, etc) are pre-built and there is not a need to run any of the update-and-rebuild
scripts traditionally required.

Enter your application's source directory (or the MOOSE directory) and run the following:

```
cd test/
make -j 4
./run_tests -j 4
```

## Running Application id=running

!style halign=left
How your application is ran is dependent on whether or not you are running across multiple
hosts. In general, you are running across multiple hosts when you are running within a job
on HPC. We have two different methods of execution that we will reference later:

- +Shell:+ Enters an interactive shell within the container where you can run multiple commands within
  the environment, can only be used on a single host
- +Execute:+ Runs a single command within the container, required for running on multiple hosts

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

!style halign=left
You can utilize HPC OnDemand to enter an interactive code server (similar to VSCode) window
on a compute host within the same containerized development environment that was previously
described.

Proceed to the "VSCode Container" IDE application HPC OnDemand.

- [Internal Users](https://ondemand.hpc.inl.gov/pun/sys/dashboard/batch_connect/sys/vscode-container/session_contexts/new)
- [External Users](https://hpcondemand.inl.gov/pun/sys/dashboard/batch_connect/sys/vscode-container/session_contexts/new)

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

!include installation/conda_pagination.md optional=True
