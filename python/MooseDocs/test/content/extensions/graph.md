# Graph Extension


!plot scatter data=[{'x':[1,2,3,4], 'y':[2,4,6,8], 'name':'2x'},
                    {'x':[1,2,3,4], 'y':[1,4,9,16], 'name':'x*x', 'marker':{'size':14}}]

!plot scatter filename=test_files/white_elephant_jan_2016.csv
              id=white-elephant
              caption=Weather data for January 2016 at the 'White Elephant' weather station.
              data=[{'x':'time', 'y':'air_temp_set_1', 'name':'Air Temp.'}]
              layout={'xaxis':{'title':'Time (days)'},
                      'yaxis':{'title':'Temperature (C)'},
                      'title':'January 2016 White Elephant Weather Station'}

The data in [white-elephant] is from a weather station in Idaho.

Graph: [!ref](white-elephant)

Idaho Flag: [!ref](media.md#idaho-flag)

Table: [!ref](table.md#float-table)

Algorithm: [!ref](algorithm.md#testalgo)
