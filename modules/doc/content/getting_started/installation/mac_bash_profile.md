
## Modify your Bash Profile

During the installation of one of the above moose-environment packages, you have two opportunities to allow the installer to modify your bash profile. This will allow the moose-environment module system and compiler stack to be made available by default with every new terminal window opened.

- The first is available after clicking 'Customize' during the install. While in this pane, check the option "MOOSE Environment", and then continue by clicking 'Install'.

- The second is after the installer has completed, where you will be presented with a pop-up window alerting you to either allow the installer to make this change, or to do nothing. If you choose 'Cancel' at this point, know that you are now responsible for altering your bash profile yourself. In order to make the module system available, you must instruct your bash profile(s) to source the following file:

  ```bash
  source /opt/moose/environments/moose_profile
  ```

  Once sourced, you can then load an appropriate compiler stack for MOOSE-based development:

  ```bash
  module load moose-dev-clang
  ```

!alert note title= Finalize Installation
Once the installer has completed, and/or you chose to modify your bash profile yourself, you must close any opened terminal windows, and re-open them to use the MOOSE environment.
