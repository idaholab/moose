# Contributing

MOOSE is a collaborative effort and we always welcome contributions!  When contributing to MOOSE you need to keep in mind that hundreds of people are depending on this code to do their jobs every day.  Because of this we have specific policies, procedures, and automated processes to maintain high code quality while allowing many changes to the code daily.

**If you are somewhat new to Git or GitHub we have worked up [a set of slides](https://mooseframework.org/static/media/uploads/docs/moose_github.pdf) to walk you through the processes of modifying MOOSE and submitting patches.**

## Code Standards

When modifying or adding to MOOSE you need to follow the strict [MOOSE Code Standard](framework_development/code_standards.md).  These guidelines ensure a common look and feel to all of the code in MOOSE allowing developers to seamlessly move between sections of code and giving users a consistent interface.

## Referencing Issues

Every modification to MOOSE +must+ reference an issue number. This means that every [Pull Request (PR)](https://help.github.com/articles/about-pull-requests/) that flows into MOOSE must have contain at least one commit that references an issue relevant to what you are working on (e.g. `refs #<number>` (where <number> is an issue number on the [MOOSE GitHub issue](https://github.com/idaholab/moose/issues) page, such as #1234). If your PR completely addresses an issue, you can automatically close it by prepending "closes" or "fixes" to the issue reference (e.g., `closes #1234`). Issue numbers are automatically checked by our testing system.

## Work In A Fork

The first step in modifying MOOSE is to create your own [fork](https://help.github.com/articles/fork-a-repo/) where you can commit your set of changes.

### 1. Fork MOOSE

- Navigate to [https://github.com/idaholab/moose](https://github.com/idaholab/moose)

- Click the "Fork" button in the upper right: [Fork Button](https://github.com/idaholab/moose#fork-destination-box).

- Clone your fork to your local machine (replace "username" with your GitHub username).

+Note:+ We recommend that you use SSH URLs instead of HTTPS. Generally you will have fewer problems with
firewalls and authentication this way. It does however require an additional step of setting up keys.
Please follow the instructions provided by Github to setup your [SSH keys](https://help.github.com/articles/connecting-to-github-with-ssh/).

```bash
git clone git@github.com:username/moose.git
```

### 2. Add the `upstream` Remote:

Add the real MOOSE repository as a remote named "upstream":

```bash
cd moose
git remote add upstream git@github.com:idaholab/moose.git
```

### 3. Make Modifications

Create a branch for your work:

```bash
git checkout -b branch_name upstream/devel
```

Make your modifications and commit them to a branch (be sure to reference an issue number in your commit messages).

```bash
git add your_file.h your_file.C
git commit -m "A message about the commit

closes #12345"
```

See [`git add`](http://git-scm.com/docs/git-add) and [`git commit`](http://git-scm.com/docs/git-commit) for more assistance on these commands.

Note: The MOOSE team prefers that you format your commit messages as follows:

```
Short Description or Title of PR (less than 50 characters)
[blank line]
More detail of your PR if needed.
 - Bulleted lists are encouraged
 - Fixes
 - Enhancements

Reference your ticket using the keyword "closes" if appropriate
to automatically close the issue when your PR is merged.
closes #12345
```

Before contributing your changes you should rebase them on top of the current set of patches in the "devel" branch in the real MOOSE repo:

```bash
git fetch upstream
git rebase upstream/devel
```

### 4. Push Modifications Back to GitHub

Push your branch back into your fork on GitHub:

```bash
git push origin branch_name
```

## Create a Pull Request

GitHub utilizes Pull Requests (PRs) to allow you to submit changes stored in your Fork back to the main MOOSE repository.  If you are generally interested in how PRs work you can look at the [official GitHub documentation](https://help.github.com/articles/using-pull-requests).  MOOSE utilizes the "Fork & Pull" collaborative development model.

In addition: [our own slides](https://mooseframework.org/static/media/uploads/docs/moose_github.pdf) are a great way to learn about the process of submitting a PR for the MOOSE project.

The main thing to remember when issuing a PR for MOOSE is that all PRs should be specified to go to the `next` branch.

## What Now?

The next phase is covered in [How a Patch Becomes Code](framework_development/patch_to_code.md)... that will take you through the process of a PR ultimately making it's way into the `master` branch in MOOSE...

## Autotools

If a developer ever needs to modify MOOSE configuration settings, they will need
to edit `$MOOSE_DIR/framework/configure.ac`. For those changes to then go into
effect, they will need to run `autoreconf` to generate the `configure` script
from `configure.ac`. We require a specific version of `autoconf` in order to
avoid generating an unnecessary `git diff`. The `autoconf` version required is
specified in the `AC_PREREQ` macro in `configure.ac`. If you do not have an
adequate version of `autoconf`, then libMesh can build it for you. To have
libMesh build you a set of autotools, do:

```
cd $MOOSE_DIR/libmesh
./bootstrap --build-autotools
```

and then prepend `$MOOSE/libmesh/contrib/autotools/bin` to your `PATH`
environment variable.
