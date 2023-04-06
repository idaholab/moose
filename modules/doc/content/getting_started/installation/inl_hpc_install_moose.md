# INL HPC Cluster

The following instructions are for those operating on [!ac](INL) [!ac](HPC) machines.

Requesting access (shell account) to [!ac](INL) [!ac](HPC) machines is handled by the
[NCRC](https://inl.gov/ncrc/) group.

For information on developing or running MOOSE-based NCRC applications (not the scope of this
document), please head over to [NCRC Applications](help/inl/applications.md) instead, and choose the
application applicable to you.

## Environment id=environment

!style! halign=left
While operating on one of our [!ac](INL) [!ac](HPC) clusters, you need only load a couple of
modules to obtain a proper compiler environment:
!style-end!

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

!style! halign=left
Follow the below instructions, and replace `cluster_name` placeholder with either `lemhi` or
`sawtooth` accordingly.
!style-end!

!alert! warning title=Replace +'cluster_name'+ accordingly!
[!ac](INL) [!ac](HPC) machines use a shared home directory structure (things you create/do on Lemhi
will be available on Sawtooth). If you do not separate your projects directory based on the cluster
you are operating on (or some other identifier), you risk developing under one environment and then
executing on another. Mistakes like this will cause the odd failure and cost you time to solve/fix.
!alert-end!

!style! halign=left
!template load file=installation/clone_moose.md.template PATH=~/cluster_name/projects
!style-end!

## Build PETSc and libMesh

!style! halign=left
MOOSE requires several support libraries in order to build or run properly. Both of these libraries
(PETSc and libMesh) can be built using our supplied scripts:
!style-end!

!template load file=installation/build_petsc_and_libmesh.md.template PATH=~/cluster_name/projects

## Build and Test MOOSE

!style! halign=left
!template load file=installation/build_moose.md.template PATH=~/cluster_name/projects
!style-end!

!template load file=installation/test_moose.md.template PATH=~/cluster_name/projects

Now that you have a working MOOSE, proceed to 'New Users' to begin your tour of MOOSE!

!content pagination use_title=True
                    previous=installation/inl_hpc_index.md
                    next=getting_started/inl_hpc_new_users.md