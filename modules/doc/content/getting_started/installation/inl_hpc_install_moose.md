# INL HPC Cluster

The following instructions are for those operating on [!ac](INL) [!ac](HPC) machines.

Requesting access (shell account) to [!ac](INL) [!ac](HPC) machines is handled by the
[NCRC](https://inl.gov/ncrc/) group. Once access has been granted, one can use
[HPC Ondemand](hpc_ondemand.md) services to gain access to a shell.

For information on developing or running MOOSE-based NCRC applications (not the scope of this
document), please head over to [NCRC Applications](help/inl/applications.md) instead, and choose the
application applicable to you.

At the time of this writing the following [!ac](INL) [!ac](HPC) clusters are available for use:

!include ncrc/hpc_cluster_information.md

!alert! warning
In order to build MOOSE, you +*must*+ be on one of the above login nodes!
!alert-end!

## Environment id=environment

!style! halign=left
While operating on one of our [!ac](INL) [!ac](HPC) clusters, you need only load a couple of
modules to obtain a proper developer environment:
!style-end!

```bash
module load use.moose moose-dev
```

!style! style=position:relative;top:-15px;left:5px;font-style:italic;font-size:small;
(required each time you log in)
!style-end!

If you would prefer not having to perform the above step each time you log in, you can append the
above command to your shell initialization file:

```bash
echo "module load use.moose moose-dev" >> ~/.bash_profile
```

## Cloning MOOSE

!style! halign=left
Follow the below instructions, and replace all occurrences of `cluster_name` with either `lemhi`,
`sawtooth`, or `bitterroot` accordingly with the machine you chose to operate on.
!style-end!

!alert! warning title=Replace occurrences of +'cluster_name'+ accordingly!
The results of your activity is shared amongst all [!ac](INL) [!ac](HPC) cluster machines. Therefore
it is important to use a directory naming convention to separate your work as you jump from cluster
to cluster. Example: if you are operating on `Sawtooth` you should also operate while in
`~/sawtooth/projects`.
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
