# Developer Install: INL HPC

!alert note title=Feedback welcome
These instructions were recently updated. If anything is unclear or you have suggestions for improvement, please share your feedback in this [GitHub discussion](https://github.com/idaholab/moose/discussions/32966).

The following instructions describe the use of pre-built containerized development environments on [!ac](INL) [!ac](HPC) to build and execute MOOSE and MOOSE-based applications. This install method also contains pre-built versions of the optional dependencies libTorch, MFEM, and NEML2.

The supported [!ac](INL) [!ac](HPC) hosts are Bitterroot, Sawtooth, Teton, and Windriver.

Any applications that are built within this development environment must also be ran within the development environment. In the instructions that follow, pay close attention to the notes that describe how to enter and execute within the development environment.

!alert! note title=Build compatibility
The build and runtime environment for Bitterroot, Sawtooth, and Windriver is the same. Thus, if you perform a build on one of these systems using the following instructions, it will be compatible with the others. For example, if you perform a build on Bitterroot, the built application can be run on Bitterroot, Sawtooth, and Windriver.

This is not true for the environment on Teton. If you wish to run an application on Teton using these instructions, you must build it on Teton and it can only be ran on Teton.
!alert-end!

To begin, follow these instructions:

1. [#get_a_build_session]: Get a session with the required resources and environment needed to build an application.
1. [#build_and_test]: Build and test an application.
1. [#run]: Run the previously built application on HPC compute nodes.

## Get a Build Session id=get_a_build_session

You cannot build MOOSE or MOOSE-based applications on a INL HPC login node. Resource usage on login nodes is limited as login nodes are shared resources. If you attempt to build an application on a login node, it is likely that your build will be automatically killed. Thus, you must perform a build on a compute node in which you can request more resources for the build and test process.

The three suggested methods to obtain a session for building and testing follow. A summary of these methods are as follows:

- [#build_session_ondemand_vscode]: Use a remote VSCode running on a compute node spawned with OnDemand.
- [#build_session_interactive_job]: Use an interactive Slurm job running on a compute node to obtain a login shell.
- [#build_session_ondemand_linux_desktop]: Use a terminal window in virtual linux desktop running on a compute node spawned with OnDemand.

The preferred method of obtaining a build session is [#build_session_ondemand_vscode], because it is the most user-friendly and is less error-prone.

### Build Session: OnDemand VSCode id=build_session_ondemand_vscode

This session utilizes OnDemand to obtain a remote VSCode window in a web browser that is running on a compute node within the development environment. This session method is preferred because it is the most user-friendly and less error-prone method to enter the development environment.

First, login to OnDemand [here (for INL employees)](https://ondemand.hpc.inl.gov/) or [here (for non-INL employees)](https://hpcondemand.inl.gov). On the menu bar at the top, hover over the "Apps" dropdown and click on "VSCode". On this window, set the following:

- Cluster: Pick the desired cluster.
- Project Code: Enter a HPC project code.
- Cores: Request 24 cores (or more).
- Working Directory: Enter the directory where MOOSE or your MOOSE-based application is cloned.
- Module: Select either moose-dev-mpich/[!versioner!version package=moose-dev] or moose-dev-openmpi/[!versioner!version package=moose-dev]

Click "Launch" to request the session. It may take a few minutes for the session to start, but once it does click on "Connect to VS Code" under the requested job. Note that this connection may fail at first while the job fully starts. Wait a few moments and try again.

Once the VSCode window is available in your browser, click on the three horizontal dashes in the top left corner of the window. Hover your mouse over the "Terminal" option and click on "New Terminal".

A new terminal window will appear. This terminal window is ran within the development environment container and contains the necessary dependencies to build and run an application.

The instructions that follow should be performed within this terminal window.

### Build Session: Interactive Job id=build_session_interactive_job

This session uses a Slurm interactive job to obtain a login shell running on a compute node.

From a session on a HPC login node (like `windriver1`, for example), run the following command to obtain a session on a compute node with 24 cores for 6 hours (replace `PROJECT` below with the HPC project key):

```bash
srun --wckey=PROJECT --nodes=1 --ntasks=1 --cpus-per-task=24 --time=06:00:00 --pty bash
```

Once the job is running, load the MOOSE development module and enter a shell within the development container:

!versioner! code
module load use.moose moose-dev/__VERSIONER_VERSION_MOOSE_DEV__
moose-dev-shell
!versioner-end!

By calling `moose-dev-shell`, the window you are now in is an interactive shell within the development environment container that contains the necessary dependencies to build and run an application.

The instructions that follow should be performed within this shell.

### Build Session: OnDemand Linux Desktop id=build_session_ondemand_linux_desktop

This session utilizes a terminal window in virtual linux desktop running on a compute node spawned with OnDemand.

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

By calling `moose-dev-shell`, the window you are now in is an interactive shell within the development environment container that contains the necessary dependencies to build and run an application.

The instructions that follow should be performed within this shell in the terminal window in the virtual linux desktop.

## Build and Test id=build_and_test

After following the instructions for one of the methods to obtain a build session in [#get_a_build_session], we will next build and test MOOSE or a MOOSE-based application.

The commands that follow should be ran within the terminal window that was instructed based on which method you chose above.

Follow the instructions in [#build_and_test_moose] if you are building MOOSE. Otherwise, follow the instructions in [#build_and_test_moose_application].

### Build and Test: MOOSE id=build_and_test_moose

!template load file=installation/developer/includes/build_test_moose.md.template moose_jobs=16

### Build and Test: MOOSE Application id=build_and_test_moose_application

!template load file=installation/developer/includes/build_test_moose_app.md.template moose_jobs=16

## Run id=run

The instructions above described how to build MOOSE or a MOOSE-based application. Now that the application is built and tested, the execution of the application to run input files will be described.

The build above was performed within a containerized development environment. With this, you cannot run the built application without also doing so within the same containerized development environment. There are a few ways to do this, which will be described in this section.

For all of the examples that follow, we will assume that you are running a built MOOSE test application found at `~/projects/moose/test/moose_test-opt`. Depending on which application you built, this may not be the path to your application. Thus, you should replace `~/projects/moose/test/moose_test-opt` with the path to the application that you built.

Additionally, you will need to replace the special string `PROJECT` with the correct project key in the commands that follow.

The common methods for running that follow are summarized as:

- [#run_slurm_submission_script]

  - Run an application in a Slurm submission script.
  - Often used to run large-scale simulations in the background (non-interactively).

- [#run_ondemand_vscode]

  - Run an application in a VSCode window in a web browser spawned by OnDemand.
  - Great for building inputs and quickly running them in the same session.

- [#run_shell_execution]:

  - Run an application within a shell in the containerized environment.

- [#run_general_execution]

  - Run an application directly in a terminal session.

### Run: Slurm Submission Script id=run_slurm_submission_script

You can run the built application within a Slurm submission script (submitted using the `sbatch`) command. This method is most commonly used for running large-scale production jobs.

In this example, there is a slight difference in how the job is executed on Teton versus the other hosts. On teton, the command `srun` is used to launch a parallel (MPI) job. On the remaining hosts (Bitterroot, Sawtooth, and Windriver), the command `mpiexec` is used to launch a parallel job. Thus, we will show examples for both.

These examples will run a 2 process job for one hour. The configuration options found at the top of the submission script prefixed by `#SBATCH` should be customized to your needs.

For Bitterroot, Sawtooth, and Windriver, an example submission script is:

!versioner! code language=bash
#!/bin/bash
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=2
#SBATCH --wckey=PROJECT

module load use.moose moose-dev/__VERSIONER_VERSION_MOOSE_DEV__
mpiexec -n 2 moose-dev-exec ~/projects/moose/test/moose_test-opt -i input.i
!versioner-end!

For Teton, an example submission script is:

!versioner! code language=bash
#!/bin/bash
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=2
#SBATCH --wckey=PROJECT

module load use.moose moose-dev/__VERSIONER_VERSION_MOOSE_DEV__
srun moose-dev-exec ~/projects/moose/test/moose_test-opt -i input.i
!versioner-end!

The above script should be saved into a file and then submitted with the `sbatch` command.

!alert note title=Must use `moose-dev-exec`
Prepending `moose-dev-exec` before the path to the executable is required. This command executes your executable within the containerized development environment. If you forget this your command will fail!

### Run: OnDemand VSCode id=run_ondemand_vscode

You can run the built application within an interactive VSCode window ran on a compute node and spawned with OnDemand. See the instructions in [#build_session_ondemand_vscode] for obtaining the session and a terminal window within the VSCode window. You will likely want to request more cores than in the given instructions if running larger jobs.

Within a terminal window in the VSCode session, simply run your application. For example:

```
mpiexec -n 4 ~/projects/moose/test/moose_test-opt -i input.i
```

will run the input file `input.i` with a built application with 4 cores.

!alert note title=Do not use `moose-dev-exec`
You should not prepend the command `moose-dev-exec` before the path to the executable. The VSCode window is already ran within the development environment.

### Run: Shell Execution id=run_shell_execution

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

### Run: General Execution id=run_general_execution

If running within a shell on HPC (like in an interactive job, but not in an OnDemand VSCode window), you can use the `moose-dev-exec` command as a prefix to the path to the application executable to run the application executable within the development environment.

For example:

!versioner! code language=bash
module load use.moose moose-dev/__VERSIONER_VERSION_MOOSE_DEV__
mpiexec -n 4 moose-dev-exec ~/projects/moose/test/moose_test-opt -i input.i
!versioner-end!

will run the input file `input.i` with a built application with 4 cores.

!alert note title=Must use `moose-dev-exec`
Prepending `moose-dev-exec` before the path to the executable is required. This is a special command that executes your executable within the development environment. If you forget this your command will fail!
