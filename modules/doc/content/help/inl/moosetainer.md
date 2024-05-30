# Civet-like Environment

Often times your code works on your machine, but fails continuous integration tests on Civet... If
this happens with your PR, you can simulate the failing environment and debug your error in an easy
interactive fashion using Apptainer Containers.

[build page](https://civet.inl.gov).

First, we need the ORAS URI detailing what container your job is failing in. You can do so by
studying the very first few lines in the step which failed. As an example:

```language=yaml
//: Running in versioned apptainer container moose-dev
BUILD_ROOT/moose/: Executing moosebuild in environment oras://mooseharbor.hpc.inl.gov/moose-dev/moose-dev-x86_64:4b79189
INFO:    Using cached SIF image
Date: Tue May 28 16:16:38 MDT 2024
```

The information we need from the failing step:
`oras://mooseharbor.hpc.inl.gov/moose-dev/moose-dev-x86_64:4b79189`

Next, launch an [Interactive Shell](hpc_ondemand.md#interactive-shell-idinteractive-shell) to any of
the Cluster machines as noted above, and perform the following:

```bash
ssh rod
```

Next, launch `moosebuild`, to enter an interactive civet-like environment:

```bash
moosebuild
```

You may notice some similarities (Civet uses moosebuild as well). The first thing you will want to
do, is create the same environment Civet was operating with. To do this, visually scan the log from
a build of interest on Civet, and look for a 'Loaded modules' line. It should be located within the
first 20 lines or so. Example:


The information immediately following 'Loaded modules' is what you are looking for. Copy & Paste the
contents of that line to load an identical environment to that of Civet's (without the quotes):

```bash
module load civet/.civet mpich-gcc-petsc_default-vtk advanced_modules autotools cmake
```

You are now ready to clone, build, and troubleshoot your application just as Civet did. If all goes
well (eh, poorly?), you should encounter the same error as Civet.

!alert note title=Be courteous to other users on Rod
Rod is a single standing workstation used by multiple people. Please only use for quick
troubleshooting, or other light duty work. If you are wishing to run your app using moosebuild
extensively, please see [Civet-like Environment on Cluster Nodes](hpc_ondemand.md#cluster-civet)
below.

### Civet-like Environment on Cluster Nodes id=cluster-civet

If you need more compute power (perhaps a failure only occurs after many hours), know that
moosebuild is also available on Sawtooth, Lemhi, and Falcon. A quick rundown on starting
`moosetainer` on these machines is as follows, and once running, the above instructions detailing
how to run on rod applies here (the following example is while running on Sawtooth):

```bash
module load pbs
qsub -I -lselect=1:ncpus=48 -lwalltime=1:00:00 -P moose
module load use.moose moosetainer
moosetainer shell moose-dev
```

!alert note
Using moosebuild (or rather Singularity) in this fashion, limits you to only having the resources
available on that one physical node. We are looking into how we might allow PBS to launch
Singularity containers (wouldn't that be cool), but this feature is some ways out.
