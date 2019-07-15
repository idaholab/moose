## Prerequisites

The following, is required to be installed before you can begin using MOOSE.

- +Xcode Command Line Tools (CLT)+. To install CLT on your machine, open a terminal and run:

  ```bash
  xcode-select --install
  ```

  If you do not have both Xcode and CLT installed, you will be presented with a dialog box allowing you to install CLT. There are two choices, 'Get Xcode' and 'Install'. Choose 'Install' to install CLT. We will need Xcode as well... but it is easier to obtain CLT *before* Xcode.

  If by chance, `xcode-select --install` states you already have Xcode installed, your best bet is to continue on with the instructions.

- +Xcode+. Using the App Store, search for and install Xcode. Once installed, you must open and run Xcode to finish the installation.

- +Caveats+

  - Mac OS Mojave, requires that an additional headers package be installed. Open a terminal, and run the following command:

    ```bash
    open /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg
    ```

    If the above command fails, then you do not have CLT installed, and you must now log into [Apple's Developer](https://developer.apple.com/downloads/more) website (iTunes account required) and obtain the appropriate version of CLT pertaining to your version of Xcode. There is no other way to obtain CLT once you have Xcode installed first. In the past, Xcode allowed for the installation of additional components (like CLT) via it's menu system. But no longer.

- +Download and install [XQuartz](https://www.xquartz.org/)+

- +Mojave 10.14+: [!package!name arch=osx10.14]
