# Remote Access to INL-HPC

The following instructions are designed to users with access to [!ac](INL) [!ac](HPC) computing
resources and operating from a machine outside of [!ac](INL).

## SSH Config id=ssh-config

Edit the `~/.ssh/config` file on your local machine to include the commands for proxy calls to
internal INL sites.

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
Host sawtooth1 sawtooth2 falcon1 falcon2 lemhi1 lemhi2 rod moosebuild.inl.gov hpcgitlab.hpc.inl.gov hpcsc.inl.gov
  ProxyJump hpclogin
```

!alert note title=What if my local and remote users names differ?
If your remote username is different from your local username you will need to add `User` directives
in the appropriate places. See the instructions [here](hpc_remote_different_user.md).

!alert! note title=What if the `ProxyJump` directive fails?
If your SSH client version is from before 2017. The `ProxyJump` command will not be
recognized. Please upgrade your client, if possible. Otherwise, replace
the ProxyJump directive with the following line:

```bash
ProxyCommand ssh -q -x hpclogin.inl.gov -W %h:%p`
```
!alert-end!

## SSH Tunnel

Create a tunnel into the HPC environment and leave it running while you require access to HPC
resources (GitLab, MOOSE Build, etc). If you close this window, you will loose your connection to
these resources.

```bash
ssh <your hpc user id>@hpclogin
```

!alert note title=Both the RSA PIN and Token are required.
Connecting in this method requires an RSA PIN + Token. You should not need a fully-qualified
domain name in your command above because of the "Host" setting in your SSH config file.

## SOCKS Proxy id=socks-proxy

Adjust the SOCKS proxy settings for your +least+ favorite web browser to reflect the following
settings:

```bash
localhost:5555
```

Choose your +least+ favorite, because once you adjust this setting, you will have to undo this
setting once you disconnect for that browser to work again. There are other ways to switch back and
forth by creating profiles. You may want to consult the Internet on how to set this up for your
browser or OS. FireFox, for example, allows for custom profiles:
[Firefox: Profile Manager](https://support.mozilla.org/en-US/kb/profile-manager-create-remove-switch-firefox-profiles)

Finally, you may want to enter a few hostnames that you do +not+ want to go resolve through your
proxy.  These would typically be external INL systems that are resolvable on the Internet
(e.g. +gitlab.software.inl.gov+, +www.inl.gov+). This will allow you to both internal and external
INL sites when connected through the SOCKS proxy.

## Log in to HPC Gitlab

Go to the following link: [https://hpcgitlab.hpc.inl.gov](https://hpcgitlab.hpc.inl.gov)
Log in using your HPC id and password, +not+ your RSA token or PIN.

## SSH Keys

Create your SSH public/private key and install it on GitLab. Instructions for doing so can be found
on GitLab itself at:
[https://hpcgitlab.hpc.inl.gov/help/ssh/README](https://hpcgitlab.hpc.inl.gov/help/ssh/README)

## Request Access

With now being able to connect to [https://hpcgitlab.hpc.inl.gov](https://hpcgitlab.hpc.inl.gov), and
having generated an SSH public/private key pair, please inform a project owner that you require
access to their project.

Once you receive an email stating you have been added as a member of said project, you should then be
able to create a Fork of that repository (using the
[https://hpcgitlab.hpc.inl.gov](https://hpcgitlab.hpc.inl.gov) web site)

To clone the repository you just forked:

```bash
git clone git@hpcgitlab.hpc.inl.gov:<your user id>/<project>.git
```

## View build status on MOOSEBuild

Using the same browser you modified your socks proxy settings, you should be able to navigate to
[https://moosebuild.inl.gov](https://moosebuild.inl.gov).

## Visit the internal HPC webpage

[https://hpcweb.hpc.inl.gov](https://hpcweb.hpc.inl.gov)
