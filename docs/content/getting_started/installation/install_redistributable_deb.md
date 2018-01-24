* Install the redistributable using your package manager, and follow the on-screen post installation instructions.
    * `dpkg -i moose-environment-*`
    * The on-screen instructions will ask you to append the following to your ~/.bashrc:

```bash
# Uncomment to enable pretty prompt:
# export MOOSE_PROMPT=true

# Uncomment to enable autojump:
# export MOOSE_JUMP=true

# Source MOOSE profile
if [ -f /opt/moose/environments/moose_profile ]; then
  . /opt/moose/environments/moose_profile
fi
```

!!! Important
    If you have any opened terminals at this point, you will need to close them, and re-open them in order for the MOOSE environment to take affect. The following instructions will ultimately fail if you do not.

!!! Info
    If you ever need to uninstall the moose-environment package, you can do so by running: `sudo dpkg -r moose-environment`

