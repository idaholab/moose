# Mac OS High Sierra or Older

!alert info title=Consider updating to Mojave
Due to the unfortunate difficulty Apple has placed on its users wishing to develop on platforms older than Mojave, it is strongly encouraged users upgrade to Mojave.
!alert-end!

!include getting_started/minimum_requirements.md

## Prerequisites

The following, is required to be installed before you can begin using MOOSE.

- +Xcode Command Line Tools (CLT)+. To install CLT on your machine, open a terminal and run:

  ```bash
  xcode-select --install
  ```

  If you do not have both Xcode and CLT installed, you will be presented with a dialog box allowing you to install CLT. There are two choices, 'Get Xcode' and 'Install'. Choose 'Install' to install CLT. We will need Xcode as well, but choosing 'Get Xcode' will send you to the App Store and will ultimately fail if you are not running the lastest version of Mac OS.

- +Xcode+. Log into [Apple's Developer](https://developer.apple.com/downloads/more) website (iTunes account required) and obtain the appropriate version of Xcode pertaining to your version of Mac OS.

- +Download and install [XQuartz](https://www.xquartz.org/)+

- +Install one of our redistributable packages pertaining to your Mac OS version:+

  - High Sierra 10.13: [!package!name arch=osx10.13]
  - Sierra 10.12: [!package!name arch=osx10.12]

!include getting_started/installation/mac_bash_profile.md

!include getting_started/installation/rod_packages.md

!include getting_started/installation/install_moose.md
