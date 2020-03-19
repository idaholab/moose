# HPC Connectivity

!alert warning
Users should only follow these instructions if/when they are operating on a machine located out side
INL's internal network.


### SSH Config

Edit your `~/.ssh/config` file and add the commands to proxy calls to internal INL sites. You may
add additional information such as a "User" to use if your local machine and HPC account have different
IDs. This file just serves as a useful guide:

```bash
ServerAliveInterval 240

Host *
  ControlMaster auto
  ControlPath ~/.ssh/master-%r@%h:%p

## HPC Entry Point
Host hpclogin
  Hostname hpclogin.inl.gov
  DynamicForward 5555

## Forwarding
Host sawtooth1 sawtooth2 falcon1 falcon2 rod moosebuild.inl.gov hpcgitlab.hpc.inl.gov hpcsc.inl.gov
  ProxyCommand ssh -q -x hpclogin.inl.gov -W %h:%p
```

### SSH Tunnel

Create a tunnel into the HPC environment and leave it running while you require access to HPC
resouces (GitLab, MOOSE Build, etc). If you close this window, you will loose your connection to
these resources.

```bash
ssh <your hpc user id>@hpclogin
```

!alert note
Connecting in this method requires an RSA PIN + Token. You shouldn't need a fully-qualified
domain name in your command above because of the "Host" setting in your SSH config file.

### Socks Proxy

Adjust the SOCKS proxy settings for your +least+ favorite web browser to reflect the following
settings:

```bash
localhost:5555
```

We say +least+ favorite, because once you adjust this setting, you will have to undo this setting
once you disconnect for that browser to work again. There are other ways to switch back and forth
by creating profiles. You may want to consult the Internet on how to set this up for your browser or OS.

### Log in to HPC Gitlab

Go to the following link: [https://hpcgitlab.hpc.inl.gov](https://hpcgitlab.hpc.inl.gov)
Log in using your HPC id and password, +not+ your RSA token or PIN.+

### SSH Keys

Create your SSH public/private key and install it on GitLab. Instructions for doing so can be found
on GitLab itself at:
[https://hpcgitlab.hpc.inl.gov/help/ssh/README](https://hpcgitlab.hpc.inl.gov/help/ssh/README)

### Request Access

With now being able to connect to [https://hpcgitlab.hpc.inl.gov](https://hpcgitlab.hpc.inl.gov), and having
generated an SSH public/private key pair, please inform a project owner that you require access to
their project.

Once you receive an email stating you have been added as a member of said project, you should then be
able to create a Fork of that repository (using the
[https://hpcgitlab.hpc.inl.gov](https://hpcgitlab.hpc.inl.gov) web site)

To clone the repository you just forked:

```bash
git clone git@hpcgitlab.hpc.inl.gov:<your user id>/<project>.git
```

### View build status on MOOSEBuild

Using the same browser you modified your socks proxy settings, you should be able to navigate to
[https://moosebuild.inl.gov](https://moosebuild.inl.gov).
