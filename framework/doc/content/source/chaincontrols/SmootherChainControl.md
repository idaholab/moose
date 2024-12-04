# SmootherChainControl

This [ChainControl](syntax/ChainControls/index.md) smooths an input signal using a moving average.
The $n$ most recent values are used to compute the average $\bar{y}$ of the input
values $y$:

!equation
\bar{y}_i = \frac{1}{n} \Sigma\limits_{k=i-n+1}^i y_i

where $i$ represents the time index.

The resulting value is named `<control>:value`, where `<control>` is the
user-given name of the `SmootherChainControl`.
The number of points to average, $n$, is provided with [!param](/ChainControls/SmootherChainControl/n_points).

!syntax parameters /ChainControls/SmootherChainControl

!syntax inputs /ChainControls/SmootherChainControl

!syntax children /ChainControls/SmootherChainControl
