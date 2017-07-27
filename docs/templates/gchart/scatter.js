google.charts.load('current', {'packages':['scatter']});
google.charts.setOnLoadCallback(drawChart);

function drawChart () {
  var data = new google.visualization.DataTable();
  {%- for col in column_names %}
  data.addColumn('number', '{{col}}');
  {%- endfor %}
  {% if not data_frame.empty %}
  data.addRows({{ data_frame[columns].values.tolist() }});
  {% endif %}
  {% include "scatter_options.html" %}
  var chart = new google.charts.Scatter(document.getElementById("{{chart_id}}"));
  chart.draw(data, google.charts.Scatter.convertOptions(options));
}
