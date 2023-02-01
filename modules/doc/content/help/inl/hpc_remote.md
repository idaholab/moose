# Remote Access Primer

!alert note title=Optional Instructions below
The following sections are for advanced users, comfortable and familiar with their computer and
their terminal. These instructions are entirely avoidable by using [inl/hpc_ondemand.md] web based
services.

The following instructions are for those wishing to use their native local machine's
terminal to access [!ac](INL) [!ac](HPC) resources while operating from a machine outside of
[!ac](INL). As well as for those wishing not to use [inl/hpc_ondemand.md].

### SSH Config id=ssh-config

Edit the `~/.ssh/config` file on your local machine to include the commands for proxy calls to
internal HPC sites.

```bash
ServerAliveInterval 240

Host *
  ControlMaster auto
  ControlPath ~/.ssh/master-%r@%h:%p

## HPC Entry Point
Host hpclogin hpclogin.inl.gov
  Hostname hpclogin.inl.gov
  DynamicForward 5555

## Forwarding
Host sawtooth1 sawtooth2 lemhi1 lemhi2 rod hoodoo1 viz1
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
ProxyCommand ssh -q -x hpclogin.inl.gov -W %h:%p
```
!alert-end!


### SSH Tunnel

Create a tunnel into the HPC environment and leave it running while you require access to HPC
resources when [inl/hpc_ondemand.md] is not available or not desired for use. If you close this
window, you will lose your connection to these resources.

```bash
ssh <your hpc user id>@hpclogin
```

!alert note title=Both the RSA PIN and Token are required.
Connecting in this method requires an RSA PIN + Token. You should not need a fully-qualified
domain name in your command above because of the "Host" setting in your SSH config file.

### SOCKS Proxy id=socks-proxy

To access common HPC resources (NCRC Application Documentation, Discourse, etc) within a web
browser, traffic can either be routed through a SOCKS proxy or you can authenticate. This can be
achieved by using a PAC (Proxy-Auto Configuration) file:

!listing moose/scripts/hpc_proxy.pac

Add the proxy configuration to your browser via the following url:

```
https://raw.githubusercontent.com/idaholab/moose/master/scripts/hpc_proxy.pac
```

Documentation is available for using [Firefox](https://support.mozilla.org/en-US/kb/connection-settings-firefox). Add the URL above within the "Automatic proxy configuration URL" box. We do not recommend utilizing Google Chrome with this functionality because it requires setting a system-wide proxy configuration.
