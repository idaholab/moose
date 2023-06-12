
def custom_evaluation(output):
  if output.count("Deprecated Object: OldNamedKernel") == 1 and output.count("Deprecated Object: DeprecatedKernel") == 1:
    return True
  return False
