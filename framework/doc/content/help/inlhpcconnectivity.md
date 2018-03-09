# HPC Connectivity

!alert warning
Users should only follow these instructions if/when they are operating on a machine located out side
INL's internal network.


### SSH Config

Edit your ~/.ssh/config file and add the following content:

```bash
Host hpcgitlab.inl.gov
 User <your hpc user id>
 ProxyCommand nc -x localhost:5555 %h %p
Host hpcsc.inl.gov
 User <your hpc user id>
 ProxyCommand nc -x localhost:5555 %h %p
Host moosebuild.inl.gov
 User <your hpc user id>
 ProxyCommand nc -x localhost:5555 %h %p
```

### SSH Tunnel

Create a tunnel into the HPC environment and leave it running while you require access to HPC
resouces (GitLab, MOOSE Build, etc). If you close this window, you will loose your connection to
these resources.

```bash
ssh -D 5555 <your hpc user id>@hpclogin.inl.gov
```

!alert note
Connecting in this method requires an RSA PIN + Token.

### Socks Proxy

Adjust the socks proxy settings for your +least+ favorite web browser to reflect the following
settings:

```bash
localhost:5555
```

We say +least+ favorite, because once you adjust this setting that web browser will be unable to
browse anything when your SSH tunnel is inactive. Requiring you to change it constantly if its your
favorite browser.

!alert note
If you do not know how to do that, look up how to change socks proxy settings for your web browser
using a search engine.

### Log in to HPC Gitlab

Go to the following link: [https://hpcgitlab.inl.gov](https://hpcgitlab.inl.gov)
Log in using your HPC id and password, +not+ your RSA token or PIN.+

### SSH Keys

Create your SSH public/private key and install it on GitLab. Instructions for doing so can be found
on GitLab itself at:
[https://hpcgitlab.inl.gov/help/ssh/README](https://hpcgitlab.inl.gov/help/ssh/README)

### Request Access

With now being able to connect to [https://hpcgitlab.inl.gov](https://hpcgitlab.inl.gov), and having
generated an SSH public/private key pair, please inform a project owner that you require access to
their project.

Once you receive an email stating you have been added as a member of said project, you should then be
able to create a Fork of that repository (using the
[https://hpcgitlab.inl.gov](https://hpcgitlab.inl.gov) web site)

To clone the repository you just forked:
```bash
git clone git@hpcgitlab.inl.gov:<your user id>/<project>.git
```

### View build status on MOOSEBuild

Using the same browser you modified your socks proxy settings, you should be able to navigate to
[https://moosebuild.inl.gov](https://moosebuild.inl.gov).
