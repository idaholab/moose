# RELAP5-3D on the INL-HPC

!style! halign=left
To access the HPC, familiarize yourself with [inl/hpc_ondemand.md]. There are three options for
gaining access to the HPC machines Sawtooth and Lemhi. The first two are accessed via the
[HPC OnDemand Dashboard](https://hpcondemand.inl.gov/pun/sys/dashboard).
!style-end!


Option 1, From the OnDemand Dashboard, click on the Clusters menu item and select one of the listed
options. This gives you a terminal on the chosen machine. This is called an interactive shell.


Option 2, From the OnDemand Dashboard, click on the Interactive Apps menu item and select Linux
Desktop. This takes you to another webpage where you will input the following information.


1. Under the Project label, enter the project from the projects page for which you will be
  performing your work. Contact the HPC staff for help determining which project your work pertains
  to.

1. Under the Cluster label, select your choice of machine. (Sawtooth or Lemhi)

1. Under the Job Type label, select Login.

1. Click Launch

Your Interactive Session may take some time before it becomes available. When it does, simply click
on it, and you will be presented within your web browser, your GUI Desktop.

Option 3, use Secure Shell rather than OnDemand. To do so use the following steps:

1. Start a local Linux terminal or Linux emulator.

1. Type: ssh hpclogin.inl.gov

1. Type your pin and the 6-digit passcode from your secure token. (2-Factor authentication)

1. Once you have accessed hpclogin, from there you must SSH login to your machine of choice.

NCRC Application binaries, like RELAP5-3D, are only available on Sawtooth and Lemhi.


## Load Environment

!style! halign=left
Once logged into either Sawtooth or Lemhi using any of the methods above, load the RELAP5
environment:
!style-end!

```bash
module load use.exp_ctl relap53D
```

This will put your terminal inside the RELAP53D container where you will be able to run relap53D.
For this example, RELAP5-3D's `--help` can be displayed by running the following command:

```bash
relap53D --help
```

## Create Input File

!style! halign=left
To demonstrate the use of RELAP5-3D and to verify that it is operating correctly, create a test
input file. For example, the following input file models reactor kinetics in the Edward’s pipe
experiment:
!style-end!

```pre
= Study reactor kinetics problem
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
100 new transnt
*  time step control
201 2.000 1.0-7 0.0001 3  1 10000 10000
*
301 rkfipow
302 rkreac
303 rkrecper
*
0030000 edwards pipe
0030001 2
0030101 4.56037-3,2
0030301 2.04801-1,2
0030601 0,2
0030801 1.177911-6,0,2
0031001 0,2
0031101 0,1
0031201 0,7.0+6,9.78293+5,2.58184+6,0,0,2
0031301 0,0,0,1
* reactivity table
20201100 reac-t
20201101  0.0,0.0, 1.1,1.1  2.0,-2.0
* reactor kinetics input
30000000 point
30000001 no-gamma  1.0+6  0.0  200.0
30000011 11
. end of case
```

Name the test file `rk.i`. This file can be created in your INL-HPC home directory or copied via scp
from your local machine. In both cases, first create a location for the file. For example, the
following creates a "testing" folder in your remote home directory:

```bash
mkdir ~/testing
```

You will need to use a terminal editor to create the file, such as emacs, vi, or vim. Please search
the internet for how to use these tools, if unfamiliar.

Alternatively, you can create the file on your local machine using your favorite editor and copy it
to the remote by running scp or rsync commands. However, doing so will cause your file to have
carriage-return and line-feeds. These must be removed prior to running relap53D. To do so use the
`dos2unix` linux command.

!alert! tip title=Copying files from your local machine
Copying files from your local machine to an HPC cluster first requires that you follow the
instructions: [inl/hpc_remote.md]. This workflow is for advanced users, comfortable with modifying
their SSH settings, and familiar with their terminal in general.

Example commands you would use with a terminal opened on your machine to copy files to HPC:

```bash
scp /path/to/the/local/input.i <your hpc user id>@hpclogin.inl.gov:~/testing/input.i
# or
rsync /path/to/the/local/input.i <your hpc user id>@hpclogin.inl.gov:~/testing/input.i
```
!alert-end!


## Running Simulation

### (1) Start-up an interactive job

!style! halign=left
For those using an interactive shell, simulations should not be executed on the login nodes (the
terminal you access via the Clusters menu item). If you are using an Interactive App or Secure Shell
skip to step 2. While this example is extremely lightweight, we will start an interactive job to get
you in the habit of doing so.
!style-end!

Start by requesting a single processor for one hour within the "moose" project queue:

```bash
qsub -I -l select=1:ncpus=1 -l walltime=01:00:00 -P moose
```

This command will likely take a few seconds or even minutes to execute depending the activity on the
machine. When it completes you should see a message similar to the following:

```bash
qsub: waiting for job 141541.sawtoothpbs to start
qsub: job 141541.sawtoothpbs ready
```

This command will place you back in your home directory. When running on an interactive node, you
will need to load the application environment once more:

```bash
module load use.exp_ctl relap53d
```

### (2) Run the simulation

!style! halign=left
In the previous section a test input file was created in your INL-HPC home directory within the
`testing` folder. RELAP5-3D is designed to read arguments for the file location and where to put
the output and plot files using flags `-i`, `-o`, `-p` respectively. RELAP5-3D also requires fluid
property files.

Examples of these files can be found in the relap53D container in the
`/opt/relap53D_fluids/` directory. Best practice is to execute relap53D from the location of the
input file, so start by navigating to this location:
!style-end!

```bash
cd ~/testing
```

Next, execute the application with the `-i` argument followed by the input filename:

```bash
relap53D -i rk.i
```

!alert! note
The above command only works if the case uses water, and the tpfh20 fluid property file is in the
`testing/` directory. Also, the output, restart and plot files are named with the defaults.
!alert-end!

The recommended command for running relap53D specifies the directory that holds the fluid property
file and the names of the output, restart, and plot files. See the command below:

```bash
relap53D -i rk.i -o rk.p -r rk.r -tpfdir /opt/relap53D_fluids
```

Check your output using the Linux command `ls`. You should see the following files in your testing
directory: `rk.i`, `rk.p`, `rk.plt`, and `rk.r`. The file with extension `.plt`, is the plot file
and is used for the visualization described below.

!alert! note title=Permission Denied?
The available applications such as RELAP5 will depend on being granted access to the specific
application. Please refer to [inl/index.md] for information about requesting access and the
different levels available.
!alert-end!

### (3) Viewing Results

!style! halign=left
If using an interactive app, you can view the results in your GUI desktop using AptPlot. To open
AptPlot, type the following into a terminal:
!style-end!

```bash
aptplot.sh
```

AptPlot should open. From the AptPlot window, under the File menu, select "Read" and then select the
"Relap5 data" option and navigate to the testing folder you created earlier. Double click on the
`rk.plt` file. This will open a second window where you can select which variables to plot. Note: if
you didn’t specify the output file name during execution, your plot file will be named `plotfil`.
You will need to rename this file with the `.plt` extension to be able to open it in AptPlot.

However, in many cases it is more desirable to view the results on your local machine. This is done
by copying the results file from the remote machine to your local machine using scp or rsnyc.


!alert! tip title=Copying files from remote HPC machine to your machine
Copying files from an HPC cluster to your machine first requires that you follow the instructions:
[inl/hpc_remote.md]. This workflow is for advanced users, comfortable with modifying their SSH
settings, and familiar with their terminal in general.

Example commands you would use with a terminal opened on your machine to copy files from HPC:

```bash
scp <your hpc user id>@hpclogin:~/testing/input_out.e /path/to/the/local/destination
```
!alert-end!