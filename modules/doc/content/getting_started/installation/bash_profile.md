## Modify your Bash Profile

For bash users (the default shell for most users), if you wish to have the moose-dev-gcc module loaded automatically with each new terminal session, copy and paste the following (which will append the necessary module load command to the end of your bash profile):

- GUI Users:

  ```bash
  echo "module load moose-dev-gcc" >> ~/.bashrc
  ```

- Console Users:

  ```bash
  echo "module load moose-dev-gcc" >> ~/.bash_profile
  ```

  !alert! note title=Both profiles?
  Some distributions are configured to source both of these files regardless of the method you use to log into your machine (meaning you may only need to modify one). Your milage may vary.
  !alert-end!

!alert! note title=Multiple Users
Each user of the machine wishing to use the moose-environment must also perform the above bash profile modification.
!alert-end!
