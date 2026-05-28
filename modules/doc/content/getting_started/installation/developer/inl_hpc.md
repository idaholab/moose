# INL HPC Development Environment

!alert note title=Feedback welcome
These instructions were recently updated. If anything is unclear or you have suggestions for improvement, please share your feedback in this [GitHub discussion](https://github.com/idaholab/moose/discussions/32966).

The following instructions are for those operating on [!ac](INL) [!ac](HPC) machines. This includes Bitterroot, Sawtooth, Teton, and Windriver.

## Usage

The development environment is deployed using containerization, which is a deployment
practice that bundles all application and application dependencies (i.e., MOOSE or a MOOSE-based
app), in a single environment. Of importance is that any applications that are built within this development environment must also be ran within the development environment. In the instructions that follow, pay close attention to the notes that describe how to enter and execute within the development environment.

!alert! note title=Build compatibility
The build and runtime environment for Bitterroot, Sawtooth, and Windriver is the same. Thus, if you perform a build on one of these systems using the following instructions, it will be compatible with the others. For example, if you perform a build on Bitterroot, the built application can be run on Bitterroot, Sawtooth, and Windriver.

This is not true for the environment on Teton. If you wish to run an application on Teton using these instructions, you must build it on Teton and it can only be ran on Teton.
!alert-end!

### Obtain a Build Session

You cannot build MOOSE or MOOSE-based applications on a INL HPC login node. Resource usage on login nodes is limited as login nodes are shared resources. Thus, you must perform a build on a compute node where more resources are available.

There are three suggested ways in obtaining a session for a build, of which you should choose one:

#### 1. OnDemand VSCode id=build_session_vscode

You can utilize OnDemand to obtain a VSCode window running on a compute node within the development environment.

First, login to OnDemand [here (for INL employees)](https://ondemand.hpc.inl.gov/) or [here (for non-INL employees)](https://hpcondemand.inl.gov). On the menu bar at the top, hover over the "Apps" dropdown and click on "VSCode". On this window, set the following:

- Cluster: Pick the desired cluster.
- Project Code: Enter a HPC project code.
- Cores: Request 24 cores (or more).
- Working Directory: Enter the directory where MOOSE or your MOOSE-based application is cloned.
- Module: Select either moose-dev-mpich/[!versioner!version package=moose-dev] or moose-dev-openmpi/[!versioner!version package=moose-dev]

Click "Launch" to request the session. It may take a few minutes for the session to start, but once it does click on "Connect to VS Code" under the requested job. Note that this connection may fail at first while the job fully starts. Wait a few moments and try again.

Once the VSCode window is available in your browser, click on the three horizontal dashes in the top left corner of the window. Hover your mouse over the "Terminal" option and click on "New Terminal".

A new terminal window will appear. The instructions that follow should be performed within this terminal window.

#### 2. Interactive Job id=build_interactive_job

An interactive job gives you an interactive shell on a compute node. From a login shell, run the following command to obtain a session on a compute node with 24 cores for building for 6 hours (replace `PROJECT` with the HPC project key):

```bash
srun --wckey=PROJECT --nodes=1 --ntasks=1 --cpus-per-task=24 --time=06:00:00 --pty bash
```

Once the job is running, load the MOOSE development module and enter a shell within the development container:

!versioner! code
module load use.moose moose-dev/__VERSIONER_VERSION_MOOSE_DEV__
moose-dev-shell
!versioner-end!

The instructions that follow should be performed within this shell.

#### 3. OnDemand Linux Desktop id=build_linux_desktop

You can utilize OnDemand to obtain a virtual desktop running on a compute node, in which you can use a terminal window within the virtual desktop.

First, login to OnDemand [here (for INL employees)](https://ondemand.hpc.inl.gov/) or [here (for non-INL employees)](https://hpcondemand.inl.gov). On the menu bar at the top, hover over the "Apps" dropdown and click on "Linux Desktop". On this window, set the following:

- Project: Enter a HPC project code.
- Job Type: Choose "Build".
- Cluster: Pick the desired cluster.
- Number of hours: 8 (or more, max is 24).
- Cores requested: Request 24 cores.

Click "Launch" to request the session. It may take a few minutes for the session to start, but once it does click on "Launch Linux Desktop".

Once the virtual desktop appears, click on the terminal icon at the bottom of the window in the dock.

A new terminal window will appear. Within the terminal window, load the MOOSE development module and enter a shell within the development container:

!versioner! code
module load use.moose moose-dev/__VERSIONER_VERSION_MOOSE_DEV__
moose-dev-shell
!versioner-end!

The instructions that follow should be performed within this shell in the terminal window.

### Using the Build Session

The commands that follow should be ran within the terminal window as described in one of the three choices for obtaining a build session described above.

#### Building and Testing MOOSE

!include installation/developer/includes/testing_moose.md

#### Building and Testing MOOSE Applications

!include installation/developer/includes/testing_moose_applications.md

### Running the Application

The instructions above described how to build MOOSE or a MOOSE-based application. Now, you need to run the built application. The build above was performed within a specific containerized environment. What this means is that you cannot run the built application without doing so inside the build environment.

For all of the examples that follow, we will assume that you are running a built MOOSE test application found at `~/projects/moose/test/moose_test-opt`. Depending on which application you built, this may not be the path to your application. Thus, you should replace `~/projects/moose/test/moose_test-opt` with the path to the application that you built.

Additionally, you will need to replace the special string `PROJECT` with the correct project key.

The most common ways of running the application are described below:

#### 1. Running: Slurm Submission Script

You can run the built application within a Slurm submission script (submitted using the `sbatch`) command. This method is most commonly used for running large-scale production jobs.

In this example, there is a slight difference in how the job is executed on Teton versus the other hosts. On teton, the command `srun` is used to launch a MPI job. On the remaining hosts (Bitterroot, Sawtooth, and Windriver), the command `mpiexec` is used to launch a MPI job. Thus, we will show examples for both.

These examples will run a 2 process job for one hour. The configuration options found at the top of the submission script prefixed by `#SBATCH` should be customized to your needs.

For Bitterroot, Sawtooth, and Windriver:

!versioner! code language=bash
#!/bin/bash
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=2
#SBATCH --wckey=PROJECT

module load use.moose moose-dev/__VERSIONER_VERSION_MOOSE_DEV__
mpiexec -n 2 moose-dev-exec ~/projects/moose/test/moose_test-opt -i input.i
!versioner-end!

For Teton:

!versioner! code language=bash
#!/bin/bash
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=2
#SBATCH --wckey=PROJECT

module load use.moose moose-dev/__VERSIONER_VERSION_MOOSE_DEV__
srun moose-dev-exec ~/projects/moose/test/moose_test-opt -i input.i
!versioner-end!

!alert note title=Must use `moose-dev-exec`
Prepending `moose-dev-exec` before the path to the executable is required. This is a special command that executes your executable within the build environment. If you forget this your command will fail!

The most significant component of this submission is the addition of `moose-dev-exec` before your path to your built executable. This is a special command that executes your executable within the build environment. If you forget this your command will fail!

#### 2. Running: OnDemand VSCode

You can also utilize an interactive VSCode window to execute the built application. See the instructions in [#build_session_vscode] for obtaining the session and a terminal window within the VSCode window. You will likely want to request more cores than in the given instructions if running larger jobs.

Within the terminal window in the VSCode session, simply run your application. For example:

```
mpiexec -n 4 ~/projects/moose/test/moose_test-opt -i input.i
```

will run the input file `input.i` with a built application with 4 cores.

!alert note title=Do not use `moose-dev-exec`
It is not necessary prepend `moose-dev-exec` to the path of the executable. This is because the VSCode window is already ran within the development environment.

#### 3. Running: Shell Execution

If running within a shell on HPC (like in an interactive job or a login node), you can enter a new shell within the development environment using the `moose-dev-shell` command. This will enter a shell within the containerized environment.

To enter the shell and execute the application directly, run the following:

!versioner! code language=bash
module load use.moose moose-dev/__VERSIONER_VERSION_MOOSE_DEV__
moose-dev-shell
mpiexec -n 4 ~/projects/moose/test/moose_test-opt -i input.i
!versioner-end!

which will run the input file `input.i` with a built application with 4 cores.

!alert note title=Do not use `moose-dev-exec`
It is not necessary prepend `moose-dev-exec` to the path of the executable. This is because commands ran within the shell obtained by the `moose-dev-shell` command are then ran within the development environment.

!alert note title=Single node only
When executing an application within `moose-dev-shell`, your application will only execute on a single node. That is, if you request a session with more than one node, you can only use the cores available on the node you are logged into.

#### 4. Running: General Execution

If running within a shell on HPC (like in an interactive job, but not in an OnDemand VSCode window), you can use the `moose-dev-exec` command as a prefix to the path to the application executable to run the application executable within the development environment.

For example:

!versioner! code language=bash
module load use.moose moose-dev/__VERSIONER_VERSION_MOOSE_DEV__
mpiexec -n 4 moose-dev-exec ~/projects/moose/test/moose_test-opt -i input.i
!versioner-end!

will run the input file `input.i` with a built application with 4 cores.

!alert note title=Must use `moose-dev-exec`
Prepending `moose-dev-exec` before the path to the executable is required. This is a special command that executes your executable within the development environment. If you forget this your command will fail!
