# Virtual Machine

!include docs/content/getting_started/minimum_requirements.md

---
## Pre-Reqs
* Depending on your operating system, download an appropriate VM Ware player from [VMWare](https://www.vmware.com/).
* After you install the VMWare product, it will ask you to create a VM. Click cancel, or close VMWare at this point.

## Download the Virtual Machine

* Download !moosepackage arch=virtual return=link!
* When the download is complete, extract the contents anywhere you would like.
* Double-clicking on this item should launch the VM. If you see a 'vmwarevm' extension on the item, you will need to go into this directory instead and double click on 'moose_3-29-2016.vmx' to launch the VM.

## Virtual Machine tid-bits
* Depending on your machine's hardware, it may complain about insufficient allocated memory. If this happens, you will have to click on the settings button, then the Processors & Memory options and adjust the memory slider appropriately. It is highly recommended to allocate 8GB or more to this VM.

* As the VM launches for the first time, you will be asked if this image was either moved or copied. Click on 'Copied'. You will automatically log into the VM with the moose user account. This account has privileged rights via sudo. The credentials for this account are:

* User ID: moose
* Password: moose

!!! Info
    This image does not have any server daemons installed (SSH, Apache etc).

!include docs/content/getting_started/installation/clone_moose.md
!include docs/content/getting_started/installation/build_libmesh.md
!include docs/content/getting_started/installation/conclusion.md
