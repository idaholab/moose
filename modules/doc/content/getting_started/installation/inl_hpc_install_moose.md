# INL HPC Cluster

The following instructions are for those operating on [!ac](INL) [!ac](HPC) machines.

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

## Cloning MOOSE

!template load file=installation/clone_moose.md.template PATH=~/cluster_name/projects

## Build PETSc and libMesh

MOOSE requires several support libraries in order to build or run properly. Both of these libraries
(PETSc and libMesh) can be built using our supplied scripts:

!template load file=installation/build_petsc_and_libmesh.md.template PATH=~/cluster_name/projects

## Build and Test MOOSE

!template load file=installation/build_moose.md.template PATH=~/cluster_name/projects

!template load file=installation/test_moose.md.template PATH=~/cluster_name/projects

With tests passing you can continue to
[building your own application](installation/inl_hpc_new_users.md).
