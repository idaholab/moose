# ElementW1pError

The W1_p error between variable $u$ and function $f$ is equal to:

!equation
||u - f||_{W1_p} = \left( \int_{\Omega} (u-f)^p + (\vec{\nabla} u - \vec{\nabla} f)^p d \Omega \right)^{/dfrac{1}{p}}

The gradient difference term is computed as below:

!equation
(\vec{\nabla} u - \vec{\nabla} f)^p = \sum_{\text{dimension i}} (\vec{\nabla} u_i - \vec{\nabla} f_i)^p

!syntax description /Postprocessors/ElementW1pError

!syntax parameters /Postprocessors/ElementW1pError

!syntax inputs /Postprocessors/ElementW1pError

!syntax children /Postprocessors/ElementW1pError
