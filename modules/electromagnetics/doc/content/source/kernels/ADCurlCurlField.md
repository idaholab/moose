# ADCurlCurlField

!syntax description /Kernels/ADCurlCurlField

## Overview

This object is the same as [CurlCurlField](CurlCurlField.md) in terms of physics/residual, but the Jacobian is calculated using automatic differentiation.

## Example Input File Syntax

!listing ad_vector_kernels.i block=Kernels/curl_curl

!syntax parameters /Kernels/ADCurlCurlField

!syntax inputs /Kernels/ADCurlCurlField

!syntax children /Kernels/ADCurlCurlField
