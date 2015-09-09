##
# The default function to execute is in a file named 'Control' and
# is named 'function'
def function(*args):
    print 'Hello from Python'
    return args[0] # returns the supplied value

##
# Function that modifies controlled value with time
def time_function(coef, t, *args):
    return coef*t + coef

##
# Function that modifies controlled value with time
def pp_function(coef, t, pp):
    if pp > 0.2:
      return 0.25
    else:
      return coef
