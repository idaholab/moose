MOOSE is a collaborative effort and we always welcome contributions!  When contributing to MOOSE you need to keep in mind that hundreds of people are depending on this code to do their jobs every day.  Because of that we have specific policies, procedures and automated processes in place to maintain high code quality while allowing many changes to flow into the code daily.

**If you are somewhat new to Git or GitHub we have worked up [a set of slides](https://mooseframework.org/static/media/uploads/docs/moose_github.pdf) to walk you through the processes of modifying MOOSE and submitting patches.**

Otherwise a brief overview can be found below:

## Code Standards

When modifying or adding to MOOSE you need to follow the strict [MOOSE Code Standard](http://mooseframework.org/wiki/CodeStandards/).  These guidelines ensure a common look and feel to all of the code in MOOSE allowing developers to seamlessly move between sections of code and giving users a consistent interface.

## Referencing Issues

Every modification to MOOSE _must_ reference an issue number.  This means that every commit that flows into MOOSE must have a #NNNN (where NNNN are the numbers for an issue such as #1234) present in the commit message.  Further, every Pull Request also needs to reference an issue number in both its description.  Issue numbers are automatically checked for by our testing system.
## Work In A Fork

The first step in modifying MOOSE is to create your own fork where you can commit your set of changes:

### 1. Fork MOOSE
1. Navigate to [https://github.com/idaholab/moose](https://github.com/idaholab/moose)
2. Click the "Fork" button in the upper right:
![Fork Button](http://mooseframework.org/static/media/uploads/images/fork_button.png)
3. Clone your fork to your local machine (replace "username" with your GitHub username):
```bash
git clone https://github.com/username/moose.git
```
If this returns "fatal: Unable to find remote helper for 'https'" you might be behind a firewall.  Try:
```bash
git clone git@github.com:username/moose.git
```

### 2. Add the `upstream` Remote:
Add the real MOOSE repository as a remote named "upstream":
```bash
cd moose
git remote add upstream https://github.com/idaholab/moose.git
```
or, if you had a problem with https, 
```bash
cd moose
git remote add upstream git@github.com:idaholab/moose.git
```

### 3. Make Modifications
* Make your modifications and commit them to a branch (be sure to reference an Issue # in your commit messages.)
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

* Before contributing your changes you should rebase them on top of the current set of patches in the "devel" branch in the real MOOSE repo:
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

The main thing to remember when issuing a PR for MOOSE is that all PRs should be specified to go to the `devel` branch.

## What Now??

The next phase is covered in [How a Patch Becomes Code](http://mooseframework.org/wiki/PatchToCode/)... that will take you through the process of a PR ultimately making it's way into the `master` branch in MOOSE...

