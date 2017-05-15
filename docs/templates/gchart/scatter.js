google.charts.load('current', {'packages':['scatter']});
google.charts.setOnLoadCallback(drawChart);

function drawChart () {
  var data = new google.visualization.DataTable();
  {%- for col in column_names %}
  data.addColumn('number', '{{col}}');
  {%- endfor %}
  data.addRows({{ data_frame[columns].values.tolist() }});
  var options = {
    lineWidth: 1,
    width: {{chart_width}},
    height: {{chart_height}},
    chart: {
      title: '{{title}}',
      subtitle: '{{subtitle}}'
    },
    hAxis: {title: '{{haxis_title}}'},
    vAxis: {title: '{{vaxis_title}}'},
  };
  var chart = new google.charts.Scatter(document.getElementById("{{chart_id}}"));
  chart.draw(data, google.charts.Scatter.convertOptions(options));
}
