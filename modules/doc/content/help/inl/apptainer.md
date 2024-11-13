# Apptainer

!style halign=left
Inevitably, PRs will fail [!ac](CIVET) while they appear to pass on your own
machine. Apptainer can help you reproduce the error, in an interactive fashion using the same
container [!ac](CIVET) used when the error occurred.

## Launch Container

!style halign=left
First, we need the [!ac](URI) detailing what container the job is failing in. By capturing the first
few lines from any [!ac](CIVET) step that failed (red box), the [!ac](URI) will be listed. As an
example:

```language=yaml
//: Running in versioned apptainer container moose-dev
BUILD_ROOT/moose/: Executing moosebuild in environment oras://mooseharbor.hpc.inl.gov/moose-dev/moose-dev-x86_64:4b79189
INFO:    Using cached SIF image
```

The above contains the [!ac](URI) we want:

```
oras://mooseharbor.hpc.inl.gov/moose-dev/moose-dev-x86_64:4b79189
```

With the [!ac](URI) known, launch an interactive shell using
[HPC OnDemand](hpc_ondemand.md#interactive-shell-idinteractive-shell) for any of the [!ac](INL)
[!ac](HPC) cluster machine login nodes, and perform the following:

```bash
[~]> module load apptainer
[~]> apptainer shell oras://mooseharbor.hpc.inl.gov/moose-dev/moose-dev-x86_64:4b79189

[sha256:52660332a1977bb9cbb3e4aaf9b19f810669eba8579b8dda6193eba7b05cf359][~]>
```

!style! style=position:relative;top:-15px;left:5px;font-style:italic;font-size:small;
(see [Apptainer Features](help/inl/apptainer.md#features) section below on how to control the
lengthy prompt)
!style-end!

## Troubleshoot

!style halign=left
You are now operating from the same container which your PR is failing wihtin. By default,
your home directory should be available. And by extension, your `~/cluster_name/projects` directory
containing your project.

!alert! tip
See [Apptainer Features](help/inl/apptainer.md#features) below if you are operating from your
`/scratch` directory. As this will require an additional argument to `apptainer` in order to make
available.
!alert-end!

Next, simply put: Clone your application (if you have not); build it; run it; and test it. Do
whatever it is that [!ac](CIVET) failed to do.

### Troubleshooting Hints

If you are having difficulty reproducing the failure:

- Be absolutely certain your project repo is clean. Also, it might be best to create a new clone of
  your project. A clone identical to how [!ac](CIVET) clones your project during the
  `Fetch and Branch` step.

- Re-visit the failing step occurring in [!ac](CIVET), and scrutinize with the utmost care all
  commands, arguments used, environment variables set, etc. One should even go so far as checking
  the steps that precede the failing step to determine if those results are cause for concern
  (cloning succeeded, but perhaps a submodule was not).

- Perhaps [!ac](CIVET) was instructed to treat warnings as errors (`make -Werror`). Or perhaps
  [!ac](CIVET) is building and using a different method (`METHOD=dbg make`), etc.

- Missing files not included in your PR (a missing `git add`).

If you're still unable to reproduce the error, please keep in mind one golden truth:
+Containers are immutable+. The lack of reproducibility is being caused by *something* that is
different in your environment compared to [!ac](CIVET)'s. Things like network connectivity, shell
flavor (tsch, zsh, bash), CPU microarchitecture (exceedingly rare), and the like.

!alert! note title=Many [!ac](CIVET) steps +do not+ have network access
Beyond the initial cloning of the application, most steps are barred from network access. You
can mimmic these behaviors if need be. See [Apptainer Features](help/inl/apptainer.md#features)
below for details.
!alert-end!

## Apptainer Features id=features

!style halign=left
The following are only a few of the many available features Apptainer has to offer.

#### Paths and Mounts (e.g. your `/scratch` directory)

!style halign=left
You can instruct Apptainer to mount any path you have access to, so that it is available while
inside the container. On [!ac](INL) [!ac](HPC) machines, this is most useful if you enjoy operating
in your `/scratch` directory. You can instruct Apptainer to mount this location by way of the `-B`
argument:

```bash
apptainer shell -B /scratch oras://...

[moose-dev][~]> ls /scratch  # contents of scratch is displayed
```

Do you want to mount something somewhere else - perhaps also with read-only permissions?

```bash
apptainer shell -B /scratch:/somewhere_else:ro oras://...

[moose-dev][~]> ls /somewhere_else  # contents of somewhere_else which contains scratch is displayed
[moose-dev][~]> touch /somewhere_else/<your user id>/testing
touch: cannot touch '/somewhere_else/<your user id>/testing': Read-only file system
```

Multiple PATHs:

```bash
apptainer shell -B /scratch:/somewhere_else:ro,/projects oras://...
```

#### Inherited Environment Variables

!style halign=left
If you wish to have some environment variable made available as the container passes control over to
you, you only need to prefix your variable with the following Apptainer influential variable
`APPTAINERENV_`, as so:

```bash
[~]> export APPTAINERENV_foo=bar
[~]> apptainer shell oras://...

[moose-dev][~]> echo $foo
bar
```

Extremely useful for obvious reasons, you can also use `APPTAINERENV_` to control how the prompt is
displayed:

```bash
[~]> export APPTAINERENV_APPTAINER_NAME=moose-dev
[~]> apptainer shell oras://mooseharbor.hpc.inl.gov/moose-dev/moose-dev-x86_64:4b79189

[moose-dev][~]>
```

!alert! tip title=Custom Prompt
There is an all-encompasing `$CUSTOM_PROMPT` variable that allows you to pass your own prompt:

```bash
[~]> export APPTAINERENV_CUSTOM_PROMPT="\[\033[1;34m\][my-container]\[\033[1;32m\][\t]\[\033[0m\]> "
[~]> apptainer shell oras://mooseharbor.hpc.inl.gov/moose-dev/moose-dev-x86_64:4b79189

[my-container][12:17:43]>
```

<!-- NOTE to editor: sub children elements require less top positioning (-7 vs -15) -->

!style! style=position:relative;top:-7px;left:5px;font-style:italic;font-size:small;
(it is not possible to replicate color codes here in documentation, but they are being honored)
!style-end!
!alert-end!

#### Helpful Arguments

!style halign=left
The following highlights some of the more influential arguments [!ac](CIVET) employs during your PR.


`[exec,shell]` means to use `exec` or `shell` sub-command arguments.

| Argument | Description |
| :- | :- |
| `exec oras://... echo hello world` | Executes `echo hello world` in side the container, then exits |
| `[exec,shell] --containall` | Minimalistic container (empty `/dev`, `/tmp`, `$HOME`) |
| `[exec,shell] --network none` | No network |
