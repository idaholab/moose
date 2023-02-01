# Remote Access HPC Connectivity (different username)

!alert warning
Users should only follow these instructions if/when they are operating on a machine located outside
INL's internal network.

### SSH Config file (different remote username)

The following example is identical to the example provided on the main [Remote HPC Connectivity](help/inl/hpc_remote.md) page, except it contains `User` directives for cases where your remote username differs from your local username.

```bash
ServerAliveInterval 240

Host *
  User doejohn
  ControlMaster auto
  ControlPath ~/.ssh/master-%r@%h:%p

## HPC Entry Point
Host hpclogin
  Hostname hpclogin.inl.gov
  DynamicForward 5555

## Forwarding
Host sawtooth1 sawtooth2 lemhi1 lemhi2 rod hoodoo1 viz1
  User doejohn
  ProxyJump hpclogin
```

[Back to main page](help/inl/hpc_remote.md)
