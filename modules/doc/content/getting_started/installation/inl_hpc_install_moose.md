# INL HPC Cluster

This page aims at preparing your HPC environment for proper MOOSE installation while operating on
[!ac](INL) [!ac](HPC) machines.

Requesting access (shell account) to [!ac](INL) [!ac](HPC) machines is handled by the
[NCRC](https://inl.gov/ncrc/) group.

For information on developing or running MOOSE-based NCRC applications (not the scope of this
document), please head over to [NCRC Applications](help/inl/applications.md) instead, and choose the
application applicable to you.

## Environment id=environment

While operating on one of our [!ac](INL) [!ac](HPC) clusters, you need only load a couple of
modules to obtain a proper compiler environment:

- +[!ac](HPC) Sawtooth or Lemhi+ (required each time you log in):

  ```bash
  module load use.moose moose-dev
  ```

If you would prefer not having to perform the above step each time you log in, you can append the
above command to your shell initialization file:

```bash
echo "module load use.moose moose-dev" >> ~/.bash_profile
```

## Cloning MOOSE id=clone

MOOSE is hosted on [GitHub](https://github.com/idaholab/moose) as an open-source project.
Many open-source MOOSE-based applications may also be obtained from GitHub, while others will
require officially requesting source code access through the [NCRC](https://inl.gov/ncrc).

MOOSE and the application you want should be cloned on the scratch storage space. This file system
is much faster than the file system that hosts your home directory. For example to clone MOOSE:

```bash
cd /scratch/$USER
mkdir projects
cd projects
git clone https://github.com/idaholab/moose
cd moose
git checkout master
```

## Building MOOSE id=build

Building MOOSE involves applying updates, building PETSc and libMesh, and finally running make.

### Applying Updates

Changes being made by other developers become available to you via your origin remote. This section
will show you how to apply these changes to your local moose repository.

First, perform a fetch operation:

```bash
cd /scratch/$USER/projects/moose
git fetch origin
```

The `fetch origin` git command updates your local references with that of the origin remote. It
does not modify any source files.

Once the fetch operation completes, we can perform a `reset` operation. If there are any changes,
this process will conform your local copy to that of origin:

```bash
git reset --hard origin/master
```

### Building PETSc and libMesh

MOOSE requires several support libraries in order to build or run properly. Both of these libraries
(PETSc and libMesh) can be built using our supplied scripts:

```
  cd /scratch/$USER/projects/moose/scripts
  export MOOSE_JOBS=6
  ./update_and_rebuild_petsc.sh
  ./update_and_rebuild_libmesh.sh
```

### Running Make

With everything up to date, we can now build MOOSE:

```bash
cd /scratch/$USER/projects/moose/test
make -j 6
```

!alert note title=If you are here updating and `make` fails above
Try: `make clobberall` and then run `make` again. +clobberall+ deletes the stale object/library
files left behind from a previous build, which is sometimes necessary for a successful build.

## Running Tests id=runtests

With MOOSE/test built, we can now run tests to verify a working binary (and environment).

```bash
cd /scratch/$USER/projects/moose/test
./run_tests -j 6
```

This operation can take some time to complete. There are more ways to test MOOSE than the above
example. Vist our [TestHarness](TestHarness.md) page to learn more about this system,
available influential environment variables, modifying how the output is displayed, and more.
