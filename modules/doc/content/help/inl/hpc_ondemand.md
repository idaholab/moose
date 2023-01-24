# HPC OnDemand

[HPC OnDemand](https://hpcondemand.inl.gov/pun/sys/dashboard), is a service provided by the INL, which allows a user direct access to the resources contained within the HPC enclave via their web browser. In order to utilize this service, you must first [request an account](https://inl.gov/ncrc).

Once your request has been accepted, and you have been given the necessary credentials provided by the HPC team, head on over to [HPC OnDemand](https://hpcondemand.inl.gov/pun/sys/dashboard).

## Dashboard

The Dashboard is your homepage when using HPC OnDemand. It will be the first page you see, after you log in.

## Home Directory

You can view your home directory by clicking File, Home Directory. This will launch a 'File Explorer' web browser tab.

File Explorer is your access to the files contained within your home directory. From here, you can perform just about any file operation normally achieved as if browsing your files using a native file explorer. File Explorer will also allow you to download and upload files to and from your machine.

## Jobs

The Jobs menu allows you to create simple bash scripts to be executed on a selected HPC Cluster.

!alert note
This is not to be confused with Portable Batch System (PBS) jobs.

## Clusters id=clusters

Clicking any item in this menu will launch an interactive shell terminal for that HPC machine. From here, you will be able to do anything you normally would with a native shell.

- #### Interactive Shell id=interactive-shell

  One of the more exciting features of HPC OnDemand, is having a terminal-like window using a web browser:

  !media large_media/hpc/hpcondemand_terminal.png style=filter:drop-shadow(0 0 0.25rem black);width:90%;

  With this prompt, you can launch jobs, build MOOSE, obtain a Civet-like testing environment (see below) and more.

## Civet-like Environment on Rod id=rod-civet

Often times your code works on your machine, but fails continuous integration tests on Civet... If this happens to you, you may be surprised how easy it is to interactively simulate the same environment you see on our [build page](https://civet.inl.gov).

Launch an Interactive Shell, to any of the Cluster machines as seen above, and then SSH into rod:

```bash
ssh rod
```

Next, launch `moosebuild`, to enter an interactive civet-like environment:

```bash
moosebuild
```

You may notice some similarities (Civet uses moosebuild as well). The first thing you will want to do, is create the same environment Civet was operating with. To do this, visually scan the log from a build of interest on Civet, and look for a 'Loaded modules' line. It should be located within the first 20 lines or so. Example:

```language=yaml
//: cp /home/moosetest/singularity/start_moosebuild.sh /home/moosetest/singularity/.civet_buildq5_start_moosebuild.3kI7O
//: source /home/moosetest/singularity/.civet_buildq5_start_moosebuild.3kI7O
//: Instructing Singularity to use default set forth by moosebuild: configs/release_build_Ubuntu-16
//: mkdir -p /tmp/Ubuntu-16
//: chmod o-rwx /tmp/Ubuntu-16
//: mapping /tmp/Ubuntu-16 as /tmp within container
//: mkdir /home/moosetest/singularity/civet_map_path/Ubuntu-16_15518_24741cf63767b610aeec7e69dcc51e8ff05b5ada
//: Using moose-environment release: 6f3c438e838564d48bf591191986ef747f50c8e1
//: ARCH=Ubuntu-16.04
//: BUILD_DATE=20191104
//: PR_VERSION=https://github.com/idaholab/package_builder/pull/213
//: rm /home/moosetest/singularity/.civet_buildq5_start_moosebuild.3kI7O
//: cp /tmp/tmpUn4yTG /tmp/Ubuntu-16/
/tmp/: /opt/singularity/bin/singularity exec --no-home /home/moosetest/singularity/containers/Ubuntu-16.simg /tmp/tmpUn4yTG
Loaded modules 'civet/.civet mpich-gcc-petsc_default-vtk advanced_modules autotools cmake'
```

The information immediately following 'Loaded modules' is what you are looking for. Copy & Paste the contents of that line to load an identical environment to that of Civet's (without the quotes):

```bash
module load civet/.civet mpich-gcc-petsc_default-vtk advanced_modules autotools cmake
```

You are now ready to clone, build, and troubleshoot your application just as Civet did. If all goes well (eh, poorly?), you should encounter the same error as Civet.

!alert note title=Be courteous to other users on Rod
Rod is a single standing workstation used by multiple people. Please only use for quick troubleshooting, or other light duty work. If you are wishing to run your app using moosebuild extensively, please see [Civet-like Environment on Cluster Nodes](hpc_ondemand.md#cluster-civet) below.

## Civet-like Environment on Cluster Nodes id=cluster-civet

If you need more compute power (perhaps a failure only occurs after many hours), know that moosebuild is also available on Sawtooth, Lemhi, and Falcon. A quick rundown on starting `moosebuild` on these machines is as follows, and once running, the above instructions detailing how to run on rod applies here (the following example is while running on Sawtooth):

```bash
module load pbs
qsub -I -lselect=1:ncpus=48 -lwalltime=1:00:00 -P moose
module load use.moose moosebuild
moosebuild
```

!alert note
Using moosebuild (or rather Singularity) in this fashion, limits you to only having the resources available on that one physical node. We are looking into how we might allow PBS to launch Singularity containers (wouldn't that be cool), but this feature is some ways out.

