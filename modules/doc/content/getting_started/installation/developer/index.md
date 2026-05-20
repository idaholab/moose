# Developer Installation

!alert note title=Feedback welcome
These instructions were recently updated. If anything is unclear or you have suggestions for improvement, please share your feedback in this [GitHub discussion](https://github.com/idaholab/moose/discussions/32966).

The following instructions describe how to install the development environment for building and compiling the MOOSE framework, modules, and MOOSE based applications. If you are not modifying MOOSE source, we suggest that you first start with [installation/user/index.md] to install a pre-built version of MOOSE.

MOOSE requires a complex set of dependencies (see [#required_dependencies]) to be built. While these dependencies can be installed by the developer, we provide a variety of development environments that reduce the need for these dependencies to be installed.

First, you need to clone MOOSE or the MOOSE-based application that you wish to develop. For the purposes of these instructions, we will assume that you are trying to build MOOSE. However, if you were given separate instructions for obtaining a MOOSE-based application, follow those instead. If not, follow the instructions in [#cloning_moose].

After obtaining a git checkout of the MOOSE or MOOSE-based application you are developing, follow the instructions based on your operating system in [#linux], [#mac_os], or [#windows].

## Cloning MOOSE id=cloning_moose

MOOSE is hosted on [GitHub](https://github.com/idaholab/moose) and should be cloned from there using [git](https://git-scm.com/).

We recommend creating a `projects` directory to contain your cloned applications. Thus, the first two commands in the commands that follow will involve acting within the `projects` directory.

If using Windows, first follow the installation instructions for installing WSL first in [#windows]. The following commands can then be ran within the WSL Ubuntu window so that the repository is cloned within the WSL space.

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

!include installation/developer/includes/per_os.md

!include installation/includes/required_dependencies.md

!include installation/includes/optional_dependencies.md
