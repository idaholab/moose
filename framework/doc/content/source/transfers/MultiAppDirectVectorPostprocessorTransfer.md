# MultiAppDirectVectorPostprocessorTransfer

!syntax description /Transfers/MultiAppDirectVectorPostprocessorTransfer

## Overview

This MultiAppDirectVectorPostprocessorTransfer provides a method to transfer complete vectors
of data between the main application and the sub-applications. When transferring data from main
application the vector from the main is copied to each of the sub-applications. If the
[!param](/Transfers/MultiAppDirectVectorPostprocessorTransfer/subapp_index) is used then data
is only transferred to the specified sub-application. When transferring data to the main application
the [!param](/Transfers/MultiAppDirectVectorPostprocessorTransfer/subapp_index) must
be supplied.


## Example Input File Syntax

!listing direct_vector_pp/main.i block=Transfers


!syntax parameters /Transfers/MultiAppDirectVectorPostprocessorTransfer

!syntax inputs /Transfers/MultiAppDirectVectorPostprocessorTransfer

!syntax children /Transfers/MultiAppDirectVectorPostprocessorTransfer
