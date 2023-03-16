[ParameterStudy]
  input = sub.i
  parameters = 'BCs/left/value BCs/right/value'
  quantities_of_interest = 'average/value'

  sampling_type = csv
  csv_samples_file = 'gold/monte_carlo_csv_study_samples_0001.csv'

  output_type = 'csv'
[]
