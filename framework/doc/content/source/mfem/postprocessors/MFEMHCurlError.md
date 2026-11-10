# MFEMHCurlError

!if! function=hasCapability('mfem')

## Overview

Postprocessor for calculating the H(curl) error of a vector $H(\mathrm{curl})$ conforming source
variable compared to a vector function $\vec{u}_{ex}$, given with its curl:

!equation
\left\Vert \vec{u}_{ex} - \vec{u}_{h}\right\Vert_{H(\mathrm{curl})} = \left( \left\Vert \vec{u}_{ex} - \vec{u}_{h}\right\Vert_{\mathrm{L2}}^2 + \left\Vert \nabla \times \vec{u}_{ex} - \nabla \times \vec{u}_{h}\right\Vert_{\mathrm{L2}}^2 \right)^{1/2}

where $\vec{u}_{h} \in H(\mathrm{curl})$. The curl $\nabla \times \vec{u}_{ex}$ is given by
[!param](/Postprocessors/MFEMHCurlError/curl), a vector function with three components in 3D. In
2D the curl is a scalar, given as a vector function with only an $x$ component. The quadrature rule
is set by [!param](/Postprocessors/MFEMHCurlError/quadrature_order), as described in
[MFEMNormPostprocessor.md].

## Example Input File Syntax

In this example, the H(curl) error of the electric field `e_field` of a 2D definite Maxwell problem
is computed against its exact solution `exact_e_field`, whose curl `exact_curl_e_field` is a scalar.

!listing test/tests/mfem/postprocessors/curlcurl_amr_errors.i block=Functions/exact_curl_e_field

!listing test/tests/mfem/postprocessors/curlcurl_amr_errors.i block=Postprocessors/hcurl_error

!syntax parameters /Postprocessors/MFEMHCurlError

!syntax inputs /Postprocessors/MFEMHCurlError

!syntax children /Postprocessors/MFEMHCurlError

!if-end!

!else
!include mfem/mfem_warning.md
