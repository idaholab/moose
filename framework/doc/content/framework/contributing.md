# Contributing

MOOSE is a collaborative effort and we always welcome contributions!  When contributing to MOOSE you need to keep in mind that hundreds of people are depending on this code to do their jobs every day.  Because of this we have specific policies, procedures, and automated processes to maintain high code quality while allowing many changes to the code daily.

!alert! tip title=Start a Discussion
Prior to investing your time it is recommended that you have a discussion with the community about
your contribution. This can help determine the best approach for your work and even avoid spending
time on something that is already possible. MOOSE has broad capabilities and an active community,
engaging in the community will certainly be beneficial.

!style halign=center
[+Start a Discussion+](https://github.com/idaholab/moose/discussions)

!alert-end!

!alert note title=Authorship in MOOSE-development papers
MOOSE being a collaborative effort, the authorship of MOOSE is shared within the community. Please see the [authorship guidelines](framework_development/authorship_guidelines.md optional=True)
for more information.

## Code Standards

When modifying or adding to MOOSE you need to follow the strict [MOOSE Code Standard](sqa/framework_scs.md).  These guidelines ensure a common look and feel to all of the code in MOOSE allowing developers to seamlessly move between sections of code and giving users a consistent interface.

## Referencing Issues

Every modification to MOOSE +must+ reference an issue number. This means that every [Pull Request (PR)](https://help.github.com/articles/about-pull-requests/) that flows into MOOSE must have contain at least one commit that references an issue relevant to what you are working on (e.g. `refs #<number>` (where <number> is an issue number on the [MOOSE GitHub issue](https://github.com/idaholab/moose/issues) page, such as #1234). If your PR completely addresses an issue, you can automatically close it by prepending "closes" or "fixes" to the issue reference (e.g., `closes #1234`). Issue numbers are automatically checked by our testing system.

## Work In A Fork

The first step in modifying MOOSE is to create your own [fork](https://help.github.com/articles/fork-a-repo/) where you can commit your set of changes.

### 1. Fork MOOSE

- Navigate to [https://github.com/idaholab/moose](https://github.com/idaholab/moose)

- Click the "Fork" button in the upper right: [Fork Button](https://github.com/idaholab/moose#fork-destination-box).

- Clone your fork to your local machine (replace "username" with your GitHub username).

!alert note title=SSH Recommended
We recommend that you use SSH URLs instead of HTTPS. Generally, you will have fewer problems with
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

### 4. Add Documentation

MOOSE follows stringent guidelines for software quality. The testing system is designed
in a manner to shield day-to-day development from any associated burdens. However, we do require
that all new code is documented in a specific manner to meet the guidelines, please refer to
[framework/documenting.md] for additional information.

### 5. Consider Adding a MOOSE Newsletter Entry

The [MOOSE Newsletter](newsletter/index.md optional=True) is a monthly digest of MOOSE bugfixes, changes, updates,
and new features. We encourage all developers who create MOOSE PRs to add a notice of this change to
the newsletter as part of the PR creation and review process. This helps advertise your change to
other developers and users who want to keep up-to-date, as each news update is also posted to our
[GitHub Discussions board](https://github.com/idaholab/moose/discussions)!

To contribute to the newsletter, edit the file corresponding to the current month located in the
newsletter directory. For example, if a PR will be merged in during the month of December 2024,
the PR developer would make this change at the following location:

```
MOOSE_DIR/modules/doc/content/newsletter/2024/2024_12.md
```

In this file, there are sections related to MOOSE framework changes, MOOSE module changes, PETSc and
libMesh updates, as well as minor (i.e., requiring a one-line description, rather than a paragraph)
bugfixes and updates. Each newsletter is made fully public early in the proceeding month; so, for
example, the newsletter for December 2024 would be posted in January 2025.

### 6. Push Modifications Back to GitHub

Push your branch back into your fork on GitHub:

```bash
git push origin branch_name
```

## 7. Create a Pull Request

GitHub utilizes Pull Requests (PRs) to allow you to submit changes stored in your Fork back to the
main MOOSE repository.  If you are generally interested in how PRs work you can look at the
[official GitHub documentation](https://help.github.com/articles/using-pull-requests).  MOOSE
utilizes the "Fork & Pull" collaborative development model.

The main thing to remember when issuing a PR for MOOSE is that all PRs should be specified to go to
the `next` branch.

## Pull Request Assignee

In most cases a PR will be assigned to MOOSE developer. The assignee is designated to help ensure
that the request is reviewed and merged (or canceled); they are not necessarily the reviewer. The aim
is to make sure every PR is being addressed and monitored. They should also serve as a point of
contact for the PR creator.

!alert note
MOOSE developers should expect to be assigned to their own PRs, as they should know who would be best to review the content.

## What Now?

The next phase is covered in [How a Patch Becomes Code](framework/patch_to_code.md)... that will
take you through the process of a PR ultimately making it's way into the `master` branch in MOOSE...

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
