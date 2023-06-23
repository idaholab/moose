# Relap5-3D Binary Access with INL-HPC

Familiarize yourself with [inl/hpc_ondemand.md], and return here with an interactive shell for the
machine you wish to run your application on. NCRC Application binaries are only available on
Sawtooth and Lemhi.

## Load Relap5-3D Environment

Logged into either Sawtooth or Lemhi, load the Relap5-3D environment:

```bash
module load use.moose moose-apps relap5
```

At this point you should be able to run `relap5`. For this example, Relap5-3D's
`--help` can be displayed by running the following command:

```bash
relap5 --help
```
