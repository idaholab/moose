google.charts.load('current', {'packages':['corechart']});
google.charts.setOnLoadCallback(drawChart);

function drawChart () {
  var data = new google.visualization.DataTable();
  var gold = new google.visualization.DataTable();
  {%- for col in column_names %}
  data.addColumn('number', '{{col}}');
  gold.addColumn('number', '{{col}}');
  {%- endfor %}
  {% if not data_frame.empty %}
  data.addRows({{ data_frame[columns].values.tolist() }});
  {% endif %}
  {% if not gold_data_frame.empty %}
  gold.addRows({{ gold_data_frame[columns].values.tolist() }});
  {% endif %}
  {% include "scatter_options.html" %}
  var chart = new google.visualization.ScatterChart(document.getElementById("{{chart_id}}"));
  var diff_data = chart.computeDiff(data, gold)
  chart.draw(diff_data, options);
}
