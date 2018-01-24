# MooseDocs Google Chart Extension
The "gchart" extension adds limited support for [Google Charts](https://developers.google.com/chart/)
via the `!chart` markdown command. The following sections detail the various charts currently
available.

!!!warning "Extension Under Development"
    This extension is still under development and currently only implements some basic functionality.
    Additionally, the syntax for this extension is likely to change.

!extension GoogleChartExtension

## Line Charts
[Google Line](https://google-developers.appspot.com/chart/interactive/docs/gallery/linechart#fullhtml)
charts provide the simple method to chart data where the first column represents the independent
x-axis data and any additional columns are associated with dependent y-axis data. To create a line
chart the `line` sub-command (e.g., `!chart line`). The complete set
of options for line charts are given in \ref{moose-line-chart}.

!chart line id=line-chart caption=An example line chart. columns=time, precip_accum_set_1, air_temp_set_1 csv=python/peacock/tests/input/white_elephant_jan_2016.csv title=Precipitation and Air Temperature at White Elephant SNOTEL Station subtitle=January 2016 column_names=Time (days), Total Precip. (in), Air Temp. (F)

!extension-settings moose-line-chart

## Scatter Charts
[Google Scatter](https://google-developers.appspot.com/chart/interactive/docs/gallery/scatterchart#fullhtml)
provide additional functionality to what is available with line plots.

!chart scatter id=scatter-chart caption=An example scatter chart. columns=time, precip_accum_set_1, air_temp_set_1 vaxis_title=Precipitation (in), Temp. (F) haxis_title=Time (days) csv=python/peacock/tests/input/white_elephant_jan_2016.csv

!extension-settings moose-scatter-chart

## Diff Scatter Charts
[Google Diff Charts](https://developers.google.com/chart/interactive/docs/gallery/diffchart)
provides the ability to display the difference between two data sets, the 'diffscatter' command
builds a scatter chart for two data sets.

!chart diffscatter id=scatter-diff-chart caption=Difference in duration between the first ten eruptions on consecutive days for Old Faithful geyser in Yellowstone National Part, USA. columns=number,duration csv=python/test_files/old_faithful.csv haxis_title=Eruption No. vaxis_title=Time (min.) haxis_ticks=[0,2,4,6,8,10]

!extension-settings moose-diff-scatter-chart
