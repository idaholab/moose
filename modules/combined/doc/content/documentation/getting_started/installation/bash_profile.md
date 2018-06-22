Follow the on-screen instructions about modifying your bash profile, which should be asking you to add the following to the end of your bash_profle:

```bash
if [ -f /opt/moose/environments/moose_profile ]; then
    . /opt/moose/environments/moose_profile
fi
```

Once that is complete, close any opened terminals and re-open them.
