## Build Issues id=buildissues

Build issues are normally caused by an invalid environment, or perhaps an update to your repository occurred, and you now have a mismatch between MOOSE and your application, or a combination of one or the other with libMesh's submodule.

- Verify you have a functional [Modules](help/troubleshooting.md#modules) environment.

- Verify the MOOSE repository is up to date, with the correct vetted version of libMesh:

  !alert warning
  Before performing the following commands, be sure you have committed your work. Because... we are about to delete stuff!

  ```bash
  cd moose
  git checkout master
  git clean -xfd

  <output snipped>

  git fetch upstream
  git pull
  git submodule update --init
  ```

- Verify you either have no moose directory set, or it is set correctly.

  ```bash
  [~] > echo $MOOSE_DIR

  [~] >
  ```

  The above should return nothing, or, it should point to the correct moose repository.

  !alert note
  Most users, do not use or set MOOSE_DIR. If the above command returns something, and you are not sure why, just unset it:

  ```bash
  unset MOOSE_DIR
  ```

- Once completed, you should attempt to [rebuild libMesh](help/troubleshooting.md#libmesh) again.
