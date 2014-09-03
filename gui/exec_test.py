#!/usr/bin/python
import sys

import io
import tests

if __name__ == '__main__':

  test = sys.argv[1]
  print test

  func = eval(sys.argv[1])
  result, msg = func()

  print result, msg
