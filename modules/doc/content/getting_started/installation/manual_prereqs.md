
- A sane, modern toolchain environment (make, Automake, Autoconf, CMake). Examples for several
  popular operating systems:

  - +Macintosh+: Xcode (`xcode-select --install`)

  - +Windows+: Windows Subsystem for Linux (WSL)

    - In addition, a chosen Linux flavor below

  - +Ubuntu+: (`apt install build-essential`)

  - +CentOS/Rocky+: (`dnf groupinstall 'Development Tools'`)

  - +OpenSUSE+: (`zypper install --type pattern devel_basis`)

Different operating systems leverage different means of obtaining a developer's environment (these
were just a few). We assume the reader is intricately familiar with their platform of choice, and
has established an environment for development.
