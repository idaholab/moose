Follow the on-screen instructions. Those instructions will ask you to close any opened terminal windows, and open a new one.

Next, in a new terminal window, load the moose-dev-gcc module:

```bash
module load moose-dev-gcc
```

For bash users (the default shell for most users), if you wish to have the moose-dev-gcc module loaded automatically with each new terminal session, copy and paste the following (which will append the necessary module load command to the end of your bash_profile):

```bash
echo "module load moose-dev-gcc" >> ~/.bash_profile
```

+If you do not perform the above echo command, you must remember to execute: `module load moose-dev-gcc` each time you wish to perform *any* MOOSE related development.+

!alert! note title=Multiple Users
Each user of the machine wishing to use the moose-environment must also perform the above bash_profile modification.
!alert-end!
