# HPC OnDemand

[HPC OnDemand](https://hpcondemand.inl.gov/pun/sys/dashboard), is a service provided by the INL, which allows a user direct access to the resources contained within the HPC enclave via their web browser. In order to utilize this service, you must first [request an account](https://modsimcode.inl.gov/SitePages/Home.aspx).

Once your request has been accepted, and you have the necessary credentials provided by the HPC team, head on over to [HPC OnDemand](https://hpcondemand.inl.gov/pun/sys/dashboard).


## Shell Prompt

One of the more exciting features of HPC OnDemand, is having a terminal-like window using a web browser:

!media large_media/hpc/hpcondemand_terminal.png style=filter:drop-shadow(0 0 0.25rem black);

With this prompt, you can access the PBS job scheduler, obtain a Civet-like testing environment, and basically anything else a traditional terminal window would otherwise allow.

You can obtain a shell prompt, by clicking on 'Clusters' and selecting any of the desired HPC machines. Each machine is reachable by the other via SSH.

## Civet-like Environment

Often times your code works on your machine, but fails Civet... drat!
