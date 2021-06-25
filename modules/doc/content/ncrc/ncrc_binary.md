# Binary Access with INL-HPC

The following information is provided for those with "HPC Binary Access" for an application and
aims to get you up and running.

These instructions assume some comfort with running commands on a terminal. To aid in having the
instructions on this page be clear, there is a convention used for the code blocks. Blocks with dark
blue backgrounds are commands executed on a local machine and those with a black background
are being executed on a remote INL machine.

```bash style=background-color:#151B54
# commands with a dark blue background are executed on your local machine
```

```bash
# commands with a black background are executed on a remote machine
```

!alert note title=Internal INL-HPC Website Access
This page provides various links to the internal INL-HPC website:
[https://hpcweb.hpc.inl.gov](https://hpcweb.hpc.inl.gov).  To access this site you will need to have
a browser setup with the correct settings, see [hpc_remote.md#socks-proxy].


## Login to INL-HPC

1. Setup is to connect to the INL-HPC, this is accomplished by first configuring your local
   machine (see [hpc_remote.md#ssh-config]). This step only needs to be completed once for each
   machine you will be using to access INL-HPC resources.

2. Login into INL-HPC.

   ```bash  style=background-color:#151B54
   ssh <your hpc user id>@hpclogin.inl.gov
   ```

## Connect to an INL-HPC Machine

When you login to "hpclogin" you will then need to connect to an HPC machine such as Sawtooth, for
a complete list of machines please refer to the internal INL HPC website:
[hpcweb.hpc.inl.gov](https://hpcweb.hpc.inl.gov).

Assuming you want to work on Sawtooth, you can access it by logging into one of the two
available login nodes: "sawtooth1" or "sawtooth2".

```bash
ssh sawtooth2
```

This will log you into the login node of a an HPC machine, you can verify that you connected to
the machine by displaying the host name:

```bash
$ echo $HOSTNAME
sawtooth2
```

!alert note title=Login nodes should not be used for simulations
The login nodes, as the name suggests, are simply for accessing the machine and scheduling jobs or
requesting an interactive job. Please do not run simulations on the login nodes, your account could
be suspended if this occurs too often.

## Load Application Environment

Next, it is necessary to load the desired application environment. For this example it is
assumed that BISON is the desired application. Others include "GRIFFIN" and "BLUE_CRAB". The
correct environment is loaded using the module command as follows.

```bash
module load use.moose moose-apps BISON
```

At this point you should be able to run the executable. For this example, the BISON executable
help can be displayed by running the following command.

```bash
/apps/herd/bison/bison-opt --help
```

## Create a Test Input File

To demonstrate the use of the binary and to verify that it is operating correctly, create a
test input file. For example, the following input file models steady-state diffusion and
works with any MOOSE-based application.

!listing moose/test/tests/kernels/simple_diffusion/simple_diffusion.i

This file can be created in your INL-HPC home directory or copied via scp from your local machine.
In both cases, first create a location for the file. For example, the following creates
a "testing" folder in your remote home directory.

```bash
mkdir ~/testing
```

If you want to create the file on the remote machine you will need to use a terminal editor to create
the file such as "emacs" or "vi". Alternatively, you can create the file on a local machine and copy
it to the remote, by running the following from your local machine, assuming you created a
file "input.i" with the aforementioned content.

```bash  style=background-color:#151B54;
scp /path/to/the/local/input.i <your hpc user id>@hpclogin.inl.gov:~/testing/input.i
```

## Running Test Simulation

### (1) Start-up an interactive job

As mentioned above simulations should not be executed on the login nodes. For this example
start an interactive job by requesting a single processor for one hour within the "moose"
project queue.

```bash
qsub -I -l select=1:ncpus=1 -l walltime=01:00:00 -P moose
```

This command will likely take a few seconds or even minutes to execute depending the activity
on the machine. When it completes you should see a message similar to the following.

```bash
qsub: waiting for job 141541.sawtoothpbs to start
qsub: job 141541.sawtoothpbs ready
```

Again, you can verify that you are on a compute node using the `echo $HOSTNAME` command, which
should give you a node name. For example, on Sawtooth an example node name is "r1i2n34".

When running in interactive mode you will also need to load the environment again.

```bash
module load use.moose moose-apps BISON
```

### (2) Run the simulation

In the previous section a test input file was created in your INL-HPC home directory within
the "testing" folder. MOOSE-based applications are designed to be executed from the location
of the input file, so start by navigating to this location.

```bash
cd ~/testing
```

Next, execute the application with the "-i" argument followed by the input filename. For
example, when using BISON with the created test input file.

```bash
/apps/herd/bison/bison-opt -i input.i
```

!alert note title=Available binaries are user dependent
The available applications such as BISON will depend on being granted access to the specific
application. Please refer to [inl/index.md] for information about requesting access and the
different levels available.

### (3) Viewing Results id=viewing-results

There are many ways to view results including options using the remote machine, please refer
to the visualization systems linked on [hpcweb.hpc.inl.gov/hardware](https://hpcweb.hpc.inl.gov/hardware).

However, in many cases it is desirable to view the results on your local machine. This is done
by copying the content from the remote machine to your local machine using `scp`. For example,
to copy the output of this test run use the following command on your local machine.

```bash  style=background-color:#151B54;
scp <your hpc user id>@hpclogin.inl.gov:~/testing/input_out.e /path/to/the/local/destination
```

## Scheduling Jobs with PBS

This section will provide you with a basic example of using [!ac](PBS) for scheduling jobs to run in
INL resources. For detailed information regarding using [!ac](PBS) please visit the [INL-HPC PBS
website](https://hpcweb.hpc.inl.gov/home/pbs).

In general, scheduling jobs requires two steps:

1. create a [!ac](PBS) script that describes the required resources and the commands to execute and
1. submit the script to the scheduler.

### (1) Create [!ac](PBS) Script

Using the example problem from above a simple script for scheduling may be created. The top
portion of the script provides [!ac](PBS) directives that are passed to scheduler. Notice that these
lines are simply the arguments that are being passed to the scheduler, as was done via the
command line in the interactive job above.

```bash
#!/bin/bash
#PBS -N test_run
#PBS -l select=1:ncpus=48:mpiprocs=48
#PBS -l walltime=5:00
#PBS -P moose

cd $PBS_O_WORKDIR
source /etc/profile.d/modules.sh
module load use.moose moose-apps PETSc BISON

mpirun /apps/herd/bison/bison-opt -i input.i Mesh/uniform_refine=2
```

It is recommended that this script be created in the same location as input file(s) it will
execute. This will ensure that the associated output from the simulation ends up in the same
location. For this example the script is located in the testing folder: `~/testing/test.sh`.

The [!ac](PBS) directives for this script include:

1. `-N test_run` to set the job name;
1. `-l select=1:ncpus=48:mpiprocs=48` indicates to utilize one compute node (`select=1`), utilizing all
   48 cores and running with 48 mpi processes;
1. `-l walltime=5:00` allocates 5 minutes of time for the simulation; and
1. `-P moose` dictates that the job should part of the MOOSE queue. For a complete list
   of queues available see [HPC PBS page](https://hpcweb.hpc.inl.gov/home/pbs).

!alert note title=The number of cores differ on each machine
Sawtooth has 48 processors per compute nodes, the other machines will have different numbers.  Please
refer to the [hpcweb.hpc.inl.gov](https://hpcweb.hpc.inl.gov/home/) for a list of the available
systems.

The second portion provides the commands to be executed. The `cd $PBS_O_WORKDIR` changes to the
directory that was used when the job was submitted (the environment variables is defined with the
leter "O"). The commands that follow enable the "module" command and then load the required modules
for the problem. Finally, the "mpirun" command is followed by the application executable and the
input file to execute. This example file is very simple, so the mesh is refined extensively to make
the problem a bit more suited for 48 cores.

### (2) Submit Script

Submitting the job to the scheduler is trivial. First, be sure to log in to one the desired machines
login nodes (e.g., `ssh sawtooth2`). Change directory to the location of the script
and associated input files and then use the "qsub" command.

```bash
cd ~testing
qsub test.sh
```

Running this command will return the job id (e.g., `141861.sawtoothpbs`). The `qstat` command
can be used to check the status of the jobs.

```
$ qstat -u <your hpc user id>

sawtoothpbs:
                                                            Req'd  Req'd   Elap
Job ID          Username Queue    Jobname    SessID NDS TSK Memory Time  S Time
--------------- -------- -------- ---------- ------ --- --- ------ ----- - -----
141861.sawtooth slauae   short    test_run      --    2 192  725gb 00:05 Q   --
```

For more information about monitoring job status
please refer to the [INL Job Status](https://hpcweb.hpc.inl.gov/home/pbs#Job_Status) page for
more information on the `qstat` command. There also are comprehensive status websites for each machine:
[hpcweb.hpc.inl.gov/status](https://hpcweb.hpc.inl.gov/status/).

When the job completes the expected output files will be available in the same location as the
input file (`~/testing`). Refer to [#viewing-results] in the previous example for more information
regarding visualization.
