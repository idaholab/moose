# Getting Started with MOOSE on HPC Machines

This document assumes your HPC Administrator has completed the [HPC Cluster](cluster.md) instructions, and you have received instructions on how to enable your environment for MOOSE-based development.

## Environment

Simply put, your HPC Administrator will provide you with instructions on how to initialize the environment they have constructed for you, for MOOSE-based development. However, you should have to do something. It is not magic, and not automatic. Please do not skip this step. If you are not sure what you should be doing to initialize your environment, please ask your HPC Administrators for help.

Most if not all errors caused during build and/or runtime are usually related to a faulty or un-initialized environment.

!include getting_started/installation/clone_moose.md

!include getting_started/installation/build_libmesh.md

!include getting_started/installation/test_moose.md

!include getting_started/installation/create_an_app.md

!include getting_started/installation/update_moose.md

!include getting_started/installation/post_moose_install.md

!include getting_started/installation/installation_troubleshooting.md
