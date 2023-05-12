# INL HPC Pre-Built MOOSE

!style! halign=left
While operating on one of the [!ac](INL) [!ac](HPC) clusters, there exists the option of using
pre-built versions of MOOSE. To request access to these clusters, please follow the instructions on
[INL's Nuclear Computational Resource Center](https://inl.gov/ncrc/) website.
!style-end!

Once access has been granted, log into Sawtooth or Lemhi using either [inl/hpc_ondemand.md]
Interactive Shell services, or directly by following our [SSH Primer](inl/hpc_remote.md).

!alert! tip
If you are waiting to be granted access, you can instead follow our
[Conda Pre-Built MOOSE](getting_started/installation/moose_conda_binary.md) in the meantime.
!alert-end!

## Load Modules

!style! halign=left
Load the following modules:
!style-end!

```bash
module load use.moose moose-apps moose
```

!alert warning
If you receive an error about modules not being known, please make sure you are logged into either
Sawtooth or Lemhi.

!alert tip
In generally, Sawtooth may have a more up-to-date version.

Once loaded, `moose` becomes available. You need now only provide input files to run simulations.
Example input files are also available while this module is loaded.

!template load file=getting_started/installation/workshop_tutorial.md.template MOOSE_SHARE=$MOOSE_DIR/moose/share/moose TUTORIAL=reactor_tutorial

## View Results

!style! halign=left
You can use HPC OnDemand to view the results file remotely. Head on over to
[HPC OnDemand Dashboard](https://hpcondemand.inl.gov/pun/sys/dashboard), and select:
`Interactive Apps` and then `Linux Desktop with Visualization`. Next, select your cluster (such as
Sawtooth), the amount of time you believe you need, and then click `Launch`.
!style-end!

It may take some time before your 'Visualization Job' becomes available. When it does, simply click
on it, and you will be presented a [!ac](GUI) desktop within your web browser. From here, you can
open visualization applications (such as Paraview), and open your results file.

To use Paraview, open a terminal by clicking `Applications` at the top left, then click
`Terminal Emulator`. A terminal window will open. Enter the following commands:

```bash
module load paraview
paraview
```

Paraview should open. From here, you can select `File`, `Open`, and navigate to the directory
containing your exodus file, and open it.

In many cases it is more desirable to view results using your local machine. This is done by copying
result files from the remote machine to your local machine using `scp` or `rsnyc`.

!alert note title=Copying files from remote HPC machine to your machine
Copying files from an HPC cluster to your machine first requires that you follow the instructions:
[inl/hpc_remote.md]. This workflow is for advanced users, comfortable with modifying their SSH
settings, and familiar with their terminal in general.

Example commands you would use with a terminal to copy files from HPC to your local machine's
Downloads folder:

```bash
cd ~/Downloads
scp <your hpc user id>@hpclogin:<path to where your exodus file is> .
```

!alert! note
Perform the above command while on your machine. Not while on an HPC machine.
!alert-end!

## Examples

!style! halign=left
Continue on to see more examples and tuturials using MOOSE! However, most of the next section is
geared towards developing your own application.
!style-end!

!content pagination use_title=True
                    previous=installation/index.md
                    next=examples_and_tutorials/index_without_new_users.md
