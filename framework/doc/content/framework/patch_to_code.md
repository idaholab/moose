# How a Patch becomes Code

!alert note
This is a follow on to [Contributing](framework/contributing.md).

MOOSE uses an extensive set of automated processes and peer review to take in code from Contributors.

## 1. It begins with a Pull Request

Contributors submit code to MOOSE by pushing their modifications to their [Fork](https://help.github.com/articles/fork-a-repo) of the MOOSE repository.  Then, a Contributor will submit a Pull Request (PR) to pull their modifications from their Fork into the `next` branch in the main MOOSE repository.

## 2. Testing At Every Turn

Once a PR is opened on GitHub, it will automatically signal [CIVET](http://civet.inl.gov), which creates multiple "jobs" on [http://civet.inl.gov](http://civet.inl.gov). Client processes running on our build machines will pull down the code contribution, check it for basic formatting, make sure that it references an issue (see [Contributing](framework/contributing.md), and runs a few other pre-checks). If you fail this check, please click on the job to explore details about what went wrong. As a first time developer, you will likely run into a few minor problems. Make changes to your branch and continue to push it up to the server to try again. +Note:+ You do not need to close your PR if you fail to pass checks. Just make adjustments and push again.

Once you have passed the pre-check, you might find that CIVET is waiting to actually build your code. This happens if you are contributing to MOOSE for the first time and we are unfamiliar with you. This is to protect our systems from abuse or malice. Rest assured, a developer will take a look at your contribution soon and will activate the CIVET targets. CIVET will run your code on several platforms under a variety of configurations to make sure that new bugs have not been introduced as a result of your contribution. Check back later for results on the testing of your PR.

## 3. Peer Review

At this point, if the code passes the tests MOOSE developers will be assigned to review the code.  Comments from MOOSE developers will be placed on the PR raising any potential issues.

## 4. Pull Request Cleanup

If there are any issues with the code as submitted it is the job of the submitter to respond to the comments and take corrective action.  That corrective action usually comes in two forms:

### Small Fixes

If only small fixes are required then the submitter should `amend` their patch set to fix the small issues and `force push` the patches back into their branch in their fork.  The basic sequence for doing so is:

```bash
# In your local clone of your Fork
git co branch_name # Checkout the branch you pushed to your Fork that the PR is using
# make changes
git add filename.C  # Use "git add" to mark changes as needing to be committed
git commit --amend  # Amend the "top" commit to include these small changes
git push --force origin branchname # Force your changes back into your branch in your Fork on GitHub
```

The actual commands that you can use for this step can vary... the above is just to give you an idea of what's involved.  If your PR is very long-lived it may be a good idea to rebase your patches on top of any recent activity in the real MOOSE repo *before* the `push`:

```bash
git fetch upstream
git rebase upstream/devel
```

You may have to fix conflicts during that stage.

### Bigger Changes

If there are more features that need to be added to your code before it's accepted by the MOOSE team you can also achieve that by simply adding more patches to your branch.  Always remember to reference the ticket number you are working on in the commit message for each patch that is going into MOOSE.

## 5. Merging

At the point where a MOOSE developer believes your code is fit for inclusion in MOOSE they will "merge" your code into the `next` branch in the main MOOSE repository.  That act of merging automatically kicks off a set of processes:

1.  [CIVET](http://civet.inl.gov) is signaled and more testing is done across multiple platforms.
2.  If #1 passes then the code change is automatically merged into the `master` branch in the main MOOSE repo (that is our "stable" branch).
3.  The automated merge to `master` cues CIVET to spawn jobs that automatically build:

- [Doxygen Documentation](http://www.mooseframework.com/docs/doxygen/moose/classes.html)
- [Browseable Input File Syntax](http://mooseframework.com/docs/syntax/moose/)
- [Test Timing Metrics](http://mooseframework.com/docs/timing/)
- [Test Coverage](http://mooseframework.com/docs/coverage/framework/)

## 6. Inactive Pull Request Policy

Pull Requests that remain inactive with unaddressed comments or failed tests for a period of +30 days+ may be closed by any member of the MOOSE team at their discretion. These Pull Requests may be reopened by the original developer or an interested party at any time if activity resumes. These unmerged PRs can be found by using the filter ```is:unmerged.```


## 7. Software Quality Policy

MOOSE follows stringent guidelines for software quality. The testing system is designed
in a manner to shield day-to-day development from any associated burdens. However, we do require
that all new code is documented in a specific manner to meet the guidelines, please refer to
[framework/documenting.md] for additional information.
