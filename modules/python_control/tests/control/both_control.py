
def execute(controlled, postprocessor):
  if postprocessor["time"] > 0.4:
    controlled["Kernels/diff/coef"] = 1.0
  if postprocessor["time"] > 0.9:
    controlled["BCs/right/value"] = 20.0

def keep_going(controlled, postprocessor):
  return postprocessor["time"] < 0.6
