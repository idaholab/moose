# TimeWindowedStatisticsPostProcessor

!syntax description /Postprocessors/TimeWindowedStatisticsPostProcessor

## Description

The `TimeWindowedStatisticsPostProcessor` reports a time statistic of another postprocessor,
selected with [!param](/Postprocessors/TimeWindowedStatisticsPostProcessor/postprocessor). For
example, it can be used to monitor whether a quantity in a pseudo-transient or statistically
steady simulation has settled, or to report its time average once it has. Only samples from time
steps with $t \geq$ [!param](/Postprocessors/TimeWindowedStatisticsPostProcessor/start_time) are
used, so an initial transient can be excluded.

One sample $p_i$ is recorded per time step. With $N_w$ the number of samples in the window, which
is the smaller of [!param](/Postprocessors/TimeWindowedStatisticsPostProcessor/window) and the
number of samples recorded so far, the [!param](/Postprocessors/TimeWindowedStatisticsPostProcessor/statistic)
parameter selects one of the following:

- `mean`: the *arithmetic* mean of the last $N_w$ samples,

  \begin{equation}
  \bar{p}_w = \frac{1}{N_w} \sum_{i=n-N_w+1}^{n} p_i
  \end{equation}

- `std`: the sample standard deviation of the last $N_w$ samples,

  \begin{equation}
  s_w = \sqrt{\frac{1}{N_w - 1} \sum_{i=n-N_w+1}^{n} \left( p_i - \bar{p}_w \right)^2}
  \end{equation}

  which is 0 when only one sample has been recorded.

- `time_integrated_mean`: the time-weighted mean of all samples since averaging started,

  \begin{equation}
  \bar{p} = \frac{\sum_i p_i \Delta t_i}{\sum_i \Delta t_i}
  \end{equation}

  where $\Delta t_i$ is the part of the time step of sample $i$ after
  [!param](/Postprocessors/TimeWindowedStatisticsPostProcessor/start_time), so that no time before
  [!param](/Postprocessors/TimeWindowedStatisticsPostProcessor/start_time) is included. At
  $t =$ [!param](/Postprocessors/TimeWindowedStatisticsPostProcessor/start_time), the integration
  interval is empty and the current value of the monitored postprocessor is returned.

- `statistics_time`: the total time $\sum_i \Delta t_i$ over which `time_integrated_mean` is
  taken.

The window is measured in time steps, not in simulation time, and the windowed statistics weigh all
samples equally regardless of the time step size. Before
[!param](/Postprocessors/TimeWindowedStatisticsPostProcessor/start_time) is reached, `mean` and
`time_integrated_mean` return the current value of the monitored postprocessor, while `std` and
`statistics_time` return 0.

!alert note
At most one sample is recorded per time step, so the statistics are not affected if the object
executes several times in a time step. Samples are taken from the value of the monitored
postprocessor at the time of execution, so the default `execute_on = TIMESTEP_END` is the
intended use.

## Example Input Syntax

In this example, the monitored postprocessor `f` is equal to the simulation time, which advances by
1 each time step. Averaging starts at $t = 2.5$ and the window holds the last 3 time steps. One
`TimeWindowedStatisticsPostProcessor` is declared for each statistic: at $t = 6$, the windowed
`mean` is the mean of 4, 5 and 6, the `std` is 1, and the `time_integrated_mean` is the mean of
3 through 6 weighted by the parts of the time steps after $t = 2.5$, which are 0.5 for the sample
at $t = 3$ and 1 for the others.

!listing test/tests/postprocessors/time_windowed_statistics/time_windowed_statistics.i block=Postprocessors

!syntax parameters /Postprocessors/TimeWindowedStatisticsPostProcessor

!syntax inputs /Postprocessors/TimeWindowedStatisticsPostProcessor

!syntax children /Postprocessors/TimeWindowedStatisticsPostProcessor
