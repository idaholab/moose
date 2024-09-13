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

!alert! warning title=+In order to clone an application:+
You +*MUST*+ be on one of the above login nodes!
!alert-end!

## Containerization

The development environment is deployed using containerization, which is a deployment
practice that bundles all application (i.e., MOOSE or a MOOSE-based app) dependencies need to run
on any host. This means that applications built within this environment can be executed
across all INL HPC clusters. For example, you can build an application within a compute node on
Sawtooth and execute the same executable of the application (providing you follow the instructions
that follow) on both Bitterroot and Lemhi.

## Versioning id=versioning

The version of the dependencies used within MOOSE (PETSc, libMesh, etc) are tied directly
to a given version of MOOSE. Given a version of MOOSE (typically a Git commit hash), we produce
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

In this case, the version of `moose-dev` that should be used is [!versioner!version package=moose-dev].
For most applications in which a MOOSE submodule is used, you will commonly excute:

!versioner! code
./moose/scripts/versioner.py moose-dev
!versioner-end!

Note that this version will change in time as MOOSE changes, or if you use an older version of MOOSE.

## Building

The containerized environment needs to be entered in order to build or execute. If you haven't yet,
read the section above, [#versioning]. At this point, you should know the version of the MOOSE
development environment to use. For the purposes of this documentation, we will use the version
[!versioner!version package=moose-dev], although yours could be different!

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
make -j 6
./run_tests -j 6

## Running

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

When you are running on multiple hosts (i.e., in a HPC job) you must use the "execute"
method to run your application as described above. This is done by appending the command
`moose-dev-exec` before the path to the binary of your application. That is, within a HPC
job submission script or within a HPC job interactive session, you will run your application
with:

!versioner! code
module load use.moose moose-dev-openmpi/__VERSIONER_VERSION_MOOSE_DEV__
mpiexec -n 2 moose-dev-exec /path/to/your/application-opt -i ...
!versioner-end!

Note that the "execute" method will also work on a single host.
