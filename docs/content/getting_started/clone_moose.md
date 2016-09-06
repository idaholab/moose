---
## Clone MOOSE
* MOOSE is hosted on GitHub and can be cloned directly from there using Git. We recommend creating a directory named projects to put all of your MOOSE related work in which leads to the following commands (from your home directory):

```bash
mkdir ~/projects
cd ~/projects
git clone https://github.com/idaholab/moose.git
cd ~/projects/moose
git checkout master
```

!!! note
    The master branch of MOOSE is the stable branch that will only be updated after all tests are passing. This protects you from the day-to-day churn in the MOOSE repository
