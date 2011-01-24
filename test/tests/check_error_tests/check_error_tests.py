import tools

def badKernel_test():
  tools.executeAppExpectError(__file__,'bad_kernel_test.i',"A _Foo_ is not a registered Kernel")
