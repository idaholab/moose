# ScaleOldChainControl

This [ChainControl](syntax/ChainControls/index.md) allows the user to scale
the previous time value for a [chain control data](/ChainControlData.md)
by another chain control data.
The resulting value is named `<control>:value`, where `<control>` is the
user-given name of the `ScaleOldChainControl`.
The chain control data to scale may be the
same chain control data created by this `ChainControl`, or it may be another.

This `ChainControl` is useful for applying corrections to simulation quantities.
For example, suppose the objective is to tune a heat transfer coefficient such
that the resulting cooling power, which we calculate using a [Postprocessor](Postprocessors/index.md) `simulated_power`,
matches an experimentally measured power, given in a `Postprocessor` `experiment_power`.
One provides an initial guess for the heat transfer coefficient [!param](/ChainControls/ScaleOldChainControl/initial_value).
The heat transfer coefficient in the boundary condition is controlled using a
[SetValueChainControl.md]. The `Postprocessor` `simulated_power` is computed
using a numerical integral over the boundary, and then a [GetPostprocessorChainControl.md]
is used to copy each of `simulated_power` and `experiment_power`. A
[ParsedChainControl.md] is used to compute a scaling factor as `experiment_power / simulated_power`.
This value is then used in [!param](/ChainControls/ScaleOldChainControl/scale_factor).
To keep the scaled value in a physical range (for instance, the heat transfer
coefficient should be a positive value), one may use a [LimitChainControl.md].
This is important, for example, because often there is noise in experimental data,
and the response time of the controlled quantity may vary. This limited
heat transfer coefficient chain control data is the one that should be used with
the `SetValueChainControl`, so it should also be the one used in
[!param](/ChainControls/ScaleOldChainControl/control_data). If there were no
limitation step, then this parameter would be set to `<control>:value`.

!syntax parameters /ChainControls/ScaleOldChainControl

!syntax inputs /ChainControls/ScaleOldChainControl

!syntax children /ChainControls/ScaleOldChainControl
