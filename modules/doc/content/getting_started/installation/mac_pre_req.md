## Prerequisites

- Xcode Command Line Tools (CLT). To install CLT on your machine, open a terminal and run:

    `xcode-select --install`

    If you do not have CLT installed, you will be presented with a dialog box allowing you to install CLT.


- +Additionally, for OSX Mojave (10.14), you also need to install the header package:+

    `open /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg`


- Download and install [XQuartz](https://www.xquartz.org/)


- Install one of our redistributable packages pertaining to your OS X version:

  - Mojave 10.14: [!package!name arch=osx10.14]
  - High Sierra 10.13: [!package!name arch=osx10.13]
  - Sierra 10.12: [!package!name arch=osx10.12]

## Modify your Bash Profile

During the installation of one of the above moose-environment packages, you have two opportunities to allow the installer to modify your bash profile. This will allow the moose-environment module system to be made available by default with every new terminal window opened.

- The first being available after clicking 'Customize' during the install. While in this pane, check the option "MOOSE Environment", and then continue by clicking 'Install'.

- The second, is after the installer has completed you will be presented with a pop-up window alerting you to either allow the installer to make this change, or to do nothing. If you choose 'Cancel' at this point, know that you are now responsible for altering your bash profile yourself. In order to make the module system available, you must instruct your bash profile(s) to source the following file:

```bash
    source /opt/moose/environments/moose_profile
    module load moose-dev-clang
```
