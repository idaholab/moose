## Build Issues id=buildissues

Build issues are normally caused by an invalid environment, or perhaps an update to your repository occurred, and you now have a mismatch between MOOSE and your application, or a combination of one or the other with libMesh's submodule.

- Verify you have a functional environment see [Modules](help/troubleshooting.md#modules) above.

- Verify the MOOSE repository is up to date, with the correct vetted version of libMesh:

  !alert warning
  Before performing the following commands, be sure you have committed your work. We are about to delete stuff!

  ```bash
  cd moose
  git checkout master
  git clean -xfd

  <output snipped>

  git fetch upstream
  git pull
  git submodule update --init
  ```

- Once completed, you should attempt to [rebuild libMesh](help/troubleshooting.md#libmesh).
