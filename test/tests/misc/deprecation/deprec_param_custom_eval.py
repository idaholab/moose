
def custom_evaluation(output):
  if output.count("The parameter 'D' is deprecated") == 1 and "The parameter 'E' is deprecated" in output:
    return True
  return False
