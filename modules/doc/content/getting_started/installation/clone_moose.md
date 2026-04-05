## Cloning MOOSE

!style! halign=left
MOOSE is hosted on [GitHub](https://github.com/idaholab/moose) and should be cloned directly from
there using [git](https://git-scm.com/). We recommend creating a directory named "projects" to put
all of your MOOSE related work.
!style-end!

To clone MOOSE, run the following commands in a terminal:

```bash
mkdir ~/projects
cd ~/projects
git clone https://github.com/idaholab/moose.git
cd moose
git checkout master
```

!alert note
The `master` branch of `idaholab/moose` is the stable branch that will only be updated after all tests are
passing. This protects you from the day-to-day changes in the MOOSE repository. MOOSE-based applications
should typically use this branch.

For the development of MOOSE (contributing changes to MOOSE), you should use the
`devel` branch of `idaholab/moose` as the base for a change.
