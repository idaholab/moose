# SmootherControl

The `SmootherControl` is a controller that smooths the input signal using a moving average technique.

It receives an [!param](/ControlLogic/SmootherControl/input) value  and stores it into a vector of size [!param](/ControlLogic/SmootherControl/n_points). When the vector reaches its maximum capacity (`n_points`), the oldest value (first element) is removed, and the current input is added to the end of the vector.
Then it calculates the average of the values stored in the vector. The value of the `output` data is computed as:

!equation
output = \frac{1}{s(v)} \Sigma^{s(v)}_{i=1} v_i\\  \\
s(v) \leq N

where $v$ is the vector containing the input values, $s(v)$ is the current size of the vector, and $N$ is the maximum size of the vector, defined by [!param](/ControlLogic/SmootherControl/n_points).

!syntax parameters /ControlLogic/SmootherControl

!syntax inputs /ControlLogic/SmootherControl

!syntax children /ControlLogic/SmootherControl
