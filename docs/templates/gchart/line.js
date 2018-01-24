google.charts.load('current', {'packages':['line']});
google.charts.setOnLoadCallback(drawChart);

function drawChart () {
  var data = new google.visualization.DataTable();
  {%- for col in column_names %}
  data.addColumn('number', '{{col}}');
  {%- endfor %}
  {% if not data_frame.empty %}
  data.addRows({{ data_frame[columns].values.tolist() }});
  {% endif %}
  var options = {
    width: {{chart_width}},
    height: {{chart_height}},
    chart: {
      title: '{{title}}',
      subtitle: '{{subtitle}}'
    }
  };
  var chart = new google.charts.Line(document.getElementById("{{chart_id}}"));
  chart.draw(data, google.charts.Line.convertOptions(options));
}
