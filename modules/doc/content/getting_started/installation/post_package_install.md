!alert! note
Internal INL users may obtain the latest redistributable packages from the following location:

[https://rod.inl.gov/moose/](https://rod.inl.gov/moose/)
!alert-end!

!alert warning title=Close Open Terminals
If you have any opened terminals at this point, you must close and re-open them to use the MOOSE
environment. The following instructions will ultimately fail if you do not.


With your terminal windows now closed... open one. -By closing and opening a terminal window, you
re-source the environment profiles the installer modified (or created).

You should now have access to MOOSE's module environment. Modules allow users to simplify the
managing of their running environment. If you and your system are already using modules, you will
find our modules prepended to your already available list.

While the moose-environment package allows for many different modules to be available, the modules
all of us use (and is thouroughly tested against) are the following:

On Linux machines:

```bash
  module load moose-dev-gcc
```

On Macintosh machines:

```bash
  module load moose-dev-clang
```

Load one of these modules pertinent to your operating system now, and continue on to Obtaining and Building MOOSE below.
