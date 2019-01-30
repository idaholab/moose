# Plotly Extension


!plot scatter data=[{'x':[1,2,3,4], 'y':[2,4,6,8], 'name':'2x'},
                    {'x':[1,2,3,4], 'y':[1,4,9,16], 'name':'x*x', 'marker':{'size':14}}]

!plot scatter filename=test_files/white_elephant_jan_2016.csv
              data=[{'x':'time', 'y':'air_temp_set_1', 'name':'Air Temp.'}]
              layout={'xaxis':{'title':'Time (days)'},
                      'yaxis':{'title':'Temperature (C)'},
                      'title':'January 2016 White Elephant Weather Station'}
