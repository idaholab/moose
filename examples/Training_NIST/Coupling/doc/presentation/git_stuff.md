# Git, Submodules, and Updates

!---

## Git

- Git is a version control software.

  - Everyone will use Git to update BISON.

  - Contributors to BISON will also use Git to prepare merge requests.

- Git is a very powerful tool. As such, if you want to learn it, download
  the free book [Pro Git](https://git-scm.com/book/en/v2).

  -  Check out the first three chapters at least.

!---

## Terminology

- *repository* (*repo* for short) is where you clone from. We have two:

  1. *idaholab* is the official location for BISON.

  1. *your_user_name* is the location of your "fork" of BISON.

- *fork* is a repo created by you from idaholab/bison.

  - Users do not need to create a fork.

  - Contributors must create a fork.

  - Once a fork is created, it is its own entity separate from idaholab/bison.

- *clone* is how you copy what is in a repo to a directory.

- *origin* is a name for the repo you originally cloned from.

- *upstream* is a name or alias set for idaholab/bison repo.

+NOTE:+ If you originally cloned from idaholab,
then you don't need an upstream. "origin" is idaholab/bison.

!---

## Git Environment

This is a pictorial diagram of our Git environment. In this diagram,
*local machine* represents either a physical, local machine or the
HPC. The two upper circles represent space on
[https://hpcgitlab.hpc.inl.gov](https://hpcgitlab.hpc.inl.gov). The dotted
arrows *upstream* and *origin* represent Git remote names.

!row!
!col! width=17%

$~$

!col-end!

!col! width=66%
!media gitlab_diagram.png style=width:100%;

!col-end!

!col! width=17%

$~$

!col-end!

!row-end!

!---

## Git Repository Overview

- A Git repository has more than just files in it. It has history, too!

- Each "*commit*" adds a page to that history.

  - Use `git log` to look at the identifier, author, date, and description of
    commits.

  - A [commit identifier](https://stackoverflow.com/questions/29106996/what-is-a-git-commit-id)
    is a SHA-1 hash of the contents of that commit and is unique to that
    commit. Example: `521747298a3790fde1710f3aa2d03b55020575aa`

  - Usually, only the first 7 digits is needed to identify a commit.

- History can branch out and merge back together.

  - Your location is the commit you are on.

  - Branches are a convenient way to keep track of commits that link together.

!---

## Git Forks

- Contributors (and some users) of BISON will have forks of BISON.

- Recall that forks become their own repository after creation.

  - A fork does not automatically receive or send information to the
    original repository.

  - You own your fork and can do anything there without fear of ruining the
    official BISON repository (idaholab/bison).

  - You, the owner of the fork, must manually receive or send (contribute)
    from or to the original repository.

  - Contributing code requires review and will be covered later.

- Forks allow for "spiderweb-like" development and collaboration.

  - Forks may be added as remote repositories with their own name, such as
    *upstream*.

  - Forks allow bilateral collaboration without having to go through the
    official repository.

!---

## Useful Git Commands

Git has documentation available from the command line:

```
git help
```

To look up the documentation about a `command` such as "status":

```
git help status
```

To find information about what state your repository is in:

```
git status
```

To see a log of commits, when they occurred, and by whom:

```
git log
```

To quickly see which branch you are on:

```
git branch
```

!---

## Submodules

- Both MOOSE and BISON use submodules to include code from other codes.

- A *submodule* is an identified version of another code or software placed
  in our repository.

  - For example, MOOSE is a submodule in BISON.

  - The exact commit of MOOSE is placed in BISON's repository.

  - Git knows to go to MOOSE's repository, get that exact commit, and place
    the contents in the "moose" folder.

- Our submodules guarantee that the version of software works with the current
  version of BISON.

- Submodules are updated regularly and should be updated when updating BISON.

!---

## Working with Submodules

!style! fontsize=90%
- Only the *first* time you update a submodule do you need to "initialize" it.

```
git submodule update --init
```

- The usual command to update a submodule does not initialize it.

```
git submodule update
```

- When inside a submodule directory, that directory acts as a local clone of
  the submodule's repository.

  - Due to submodule directories being local clones, Git commands are available.

  - Most users will never have to adjust submodules except for updating them
    in the BISON directory.

  - Some contributors may need to checkout branches for submodules for
    development involving both BISON and that submodule.

  - Git is able to track changes to submodules, add remotes of other submodule
    forks, and pull/push changes.

- Check that contribution merge requests do not have erroneous submodule
  updates within them. It is *very rare* that a merge request will have a submodule
  update change within it.

!style-end!

!---

## Preparing to Update BISON

- Git is used to update BISON from idaholab/bison. BISON's documentation
  provides the instructions, but before you update:

  - Use `git status` to make sure your repository has no uncommitted changes.

  - Un-compile BISON to make sure all compilation files are properly cleaned
    with `make cleanall`.

  - Use `git branch` to check to make sure you are on the branch you want updated.

- If you have problems, please reach out to us through the BISON mailing list.

- Be conscientious of your work and make backups if your work is located within BISON's
  directory. Always have a backup of your important work.
