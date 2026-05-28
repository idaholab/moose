# Developer Installation

!alert note title=Feedback welcome
These instructions were recently updated. If anything is unclear or you have suggestions for improvement, please share your feedback in this [GitHub discussion](https://github.com/idaholab/moose/discussions/32966).

The following instructions describe how to compile MOOSE or MOOSE-based applications from source. If you are not modifying MOOSE source, we suggest that you first start with [installation/user/index.md] to install a pre-built version of MOOSE.

MOOSE requires a complex set of dependencies (see [#required_dependencies]) to be built. While these dependencies can be installed by the developer, we provide a variety of development environments that reduce the need for these dependencies to be installed. These development environments are not required to build an application but greatly simplify the process of doing so.

First, you need to clone MOOSE or the MOOSE-based application that you wish to build and possibly develop. For the purposes of these instructions, we will assume that you are trying to build MOOSE. However, if you were given separate instructions for obtaining a MOOSE-based application, follow those instead. If not, follow the instructions in [#cloning_moose].

After obtaining a git checkout of the MOOSE or MOOSE-based application you are developing, follow the instructions based on your operating system and/or environment in [#linux], [#inl_hpc], [#mac_os], or [#windows].

## Cloning MOOSE id=cloning_moose

MOOSE is hosted on [GitHub](https://github.com/idaholab/moose) and should be cloned from there using [git](https://git-scm.com/).

We recommend creating a `projects` directory to contain your cloned applications. Thus, the first two commands in the commands that follow will involve acting within the `projects` directory.

!alert note title=Cloning on Windows
If using Windows, first follow the installation instructions for installing WSL first in [#windows]. The following commands can then be ran within the WSL Ubuntu window so that the repository is cloned within the WSL space.

!alert! note title=Cloning on INL HPC
On INL HPC, internet access is not available on compute nodes. That is, you can only perform git operations (including the operation that follows) like `git clone`, `git push`, `git pull`, and `git fetch` on a login node.

It is also suggested to clone and build applications within the scratch space instead. On INL HPC machines, your scratch directory is found at `/scratch/USERNAME` where `USERNAME` is your HPC username. File system performance on scratch is significantly faster than performance within your home directory. However, files stored within scratch may be deleted when they are more than 90 days old. Thus, be sure to copy content from scratch that you do not want to lose.
!alert-end!

To clone MOOSE, run the following commands:

```bash
mkdir -p ~/projects
cd ~/projects
git clone --origin upstream https://github.com/idaholab/moose.git
cd moose
git checkout devel
```

For the instructions that follow, when we reference the "MOOSE repository", we are considering the repository you just cloned which is found at `~/projects/moose`.

If you plan on contributing code changes to MOOSE, you will need to fork the [GitHub](idaholab/moose) repository and contribute changes via pull requests and your fork. For contributing code changes, you should base your pull request branches on the `devel` branch.

## Installation

!include installation/developer/includes/per_os.md

## Dependencies

!include installation/includes/required_dependencies.md

!include installation/includes/optional_dependencies.md
