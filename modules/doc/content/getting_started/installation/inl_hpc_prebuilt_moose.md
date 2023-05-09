# INL HPC Pre-Built MOOSE

!style! halign=left
While operating on one of the [!ac](INL) [!ac](HPC) clusters, there exists the option of using
pre-built versions of MOOSE. To request access to these clusters, please follow the instructions on
[INL's Nuclear Computational Resource Center](https://inl.gov/ncrc/) website.
!style-end!

Once access has been granted, log into Sawtooth or Lemhi using either [inl/hpc_ondemand.md]
Interactive Shell services, or directly by following our [SSH Primer](inl/hpc_remote.md).

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

## Run an Example

!style! halign=left
HPC Pre-Built MOOSE comes with several examples you can run to make sure everything is sound, as
well as moving some of the example inputs into a safe location you can play with.
!style-end!

There are examples for each physics solver available by name, in the following directory:

```bash
ls $MOOSE_DIR/moose/share/combined
```

!alert! note
Not everything you find in this directory is a physics library. We are working on an elegant way to
ask `moose` for all available solvers.
!alert-end!

For now, lets copy the reactor module workshop into a safe location for editing:

```bash
mkdir -p ~/projects/examples
cd ~/projects/examples
moose --copy-inputs reactor_workshop
```

!alert! note
Take note of the information being displayed in the output. `moose` is alerting to the directory
structure it created. Which can sometimes not represent the exact wordage you provided as arguments.
!alert-end!

With the reactor module's examples and inputs copied, move into the reactor workshop directory and
instruct `moose` to run the tests:

```bash
cd combined/reactor_workshop
moose --run -j 6
```

Testing will commence and take a few moments to finish. There may be several skipped tests for one
reason or another. This is normal. However no test should fail.

Next, we will run a single test manually, to demonstrate how you will ultimately be using `moose`.
Navigate to the following directory, and run the following input file:

```bash
cd ~/projects/examples/combined/reactor_workshop/tests/reactor_examples/abtr/
moose -i abtr.i --mesh-only
```

You will see some information scroll by, and ultimately end back up at your prompt. If you perform a
directory listing you should also see that an exodus file was generated in the process
(`abtr_in.e`):

```bash
ls
abtr_griffin_snippet.i  abtr.i  abtr_in.e
```

You can follow the instructions in the next section to use Paraview, to view this result.

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

Paraview should open. From here, you can select `File`, `Open`, and navigate to

```pre
~/projects/examples/combined/reactor_workshop/tests/reactor_examples/abtr/
```

You should see the same `abtr_in.e` file. Double click on it, and Paraview should open it!

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
scp <your hpc user id>@hpclogin:~/projects/examples/combined/reactor_workshop/tests/reactor_examples/abtr/abtr_in.e .
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
