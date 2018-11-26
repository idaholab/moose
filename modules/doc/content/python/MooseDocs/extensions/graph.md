# Graph Extension

MooseDocs uses [plotly] for creating interactive graphs. Graphs are invoked
with the `!plot` command, the available sub-commands are detailed in the following
sections. [plotly-ext-config] lists the configuration options for the extension.

!devel settings module=MooseDocs.extensions.graph
                object=GraphExtension
                id=plotly-ext-config
                caption=Available configuration options for the `PlotlyExtension` object.

The syntax for the support chart types is defined in a manner that allows for all [plotly]
settings to be available for defining the data as well as the chart layout. For a complete list
of available options refer to the [reference manual](https://plot.ly/javascript/reference).


## Scatter and Line Plots

[Scatter and line](https://plot.ly/javascript/line-and-scatter/) plots are create with the
`scatter` sub-command. As shown in the [plotly] documentation the plot command accepts
a single JSON data structure that contains the data and settings for each line. For simplicity
and to remain consistent with the library this same data structure is used within MooseDocs,
as shown in [plotly-data-example].


!devel! example id=plotly-data-example caption=Example scatter plot using [plotly] data structure.
!plot scatter data=[{'x':[1,2,3,4], 'y':[2,4,6,8], 'name':'2x'},
                    {'x':[1,2,3,4], 'y':[1,4,9,16], 'name':'x*x', 'marker':{'size':14}}]
!devel-end!

In most circumstances it is desired to load scatter data from a file, this is done using the
"filename" setting. When used the "x" and "y" data should contain the column name rather than
the actual data, as shown in [plotly-file-example]. All other aspects of the data field remain the
same.

!devel! example id=plotly-file-example caption=Example scatter plot using file-based data.
!plot scatter filename=test_files/white_elephant_jan_2016.csv
              data=[{'x':'time', 'y':'air_temp_set_1', 'name':'Air Temp.'}]
              layout={'xaxis':{'title':'Time (days)'},
                      'yaxis':{'title':'Temperature (C)'},
                      'title':'January 2016 White Elephant Weather Station'}
!devel-end!

A complete list of settings available to the `plot scatter` command are included in
[plot-scatter-settings].

!devel settings module=MooseDocs.extensions.graph
                object=GraphScatter
                id=plot-scatter-settings
                caption=Available settings for the `plot scatter` command.

[plotly]: https://plot.ly/

