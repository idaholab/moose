# HPC OnDemand

[Open OnDemand](https://openondemand.org) is a web based HPC access portal. OnDemand allows users to access HPC files, open shells, access Linux desktops, use other interactive apps, and build job submissions to submit to a cluster, all from the web browser.  From the OnDemand web portal, users who have been given permission, are able to use level 1 code access.


## Access

Idaho National Lab's Open OnDemand instance can be accessed externally via [hpcondemand.inl.gov](https://hpcondemand.inl.gov/) with your INL HPC username and pin + token or internally at [ondemand.hpc.inl.gov](https://ondemand.hpc.inl.gov) with your INL HPC username and password.

Once authenticated in the OnDemand system, the top navigation bar has a menu item "NCRC". Clicking on this will show a dropdown menu that is populated with the NCRC apps that you have been granted level 1 access to. The following documentation is applicable for each level 1 NCRC application.

[HPC OnDemand](https://hpcondemand.inl.gov/pun/sys/dashboard), is a service provided by the INL, which allows a user direct access to the resources contained within the HPC enclave via their web browser.

Once your request has been accepted, and you have been given the necessary credentials provided by the HPC team, head on over to [HPC OnDemand](https://hpcondemand.inl.gov/pun/sys/dashboard).

## Dashboard

The Dashboard is your homepage when using HPC OnDemand. It will be the first page you see, after you log in.

## Home Directory

You can view your home directory by clicking File, Home Directory. This will launch a 'File Explorer' web browser tab.

File Explorer is your access to the files contained within your home directory. From here, you can perform just about any file operation normally achieved as if browsing your files using a native file explorer. File Explorer will also allow you to download and upload files to and from your machine.

## Jobs

The Jobs menu allows you to create simple bash scripts to be executed on a selected HPC Cluster.

!alert note
This is not to be confused with Portable Batch System (PBS) jobs.

## Clusters

Clicking any item in this menu will launch an interactive shell terminal for that HPC machine. From here, you will be able to do anything you normally would with a native shell.

- #### Interactive Shell

  One of the more exciting features of HPC OnDemand, is having a terminal-like window using a web browser:

  !media large_media/hpc/hpcondemand_terminal.png style=filter:drop-shadow(0 0 0.25rem black);

  With this prompt, you can launch jobs, build MOOSE, obtain a Civet-like testing environment (see below) and more.


# Command Line Access

If you are more comfortable using the command line than the on demand interface, you may follow the instructions [here](ncrc/ncrc_binary.md)
