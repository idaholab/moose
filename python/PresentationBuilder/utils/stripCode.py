import re
from FactorySystem import ParseGetPot

##
# A function for stripping C++ functions from .C files
# @param code The code to extract from
# @param function The function to extract
def stripCppFunction(code, function):

  strt = code.find(function)
  strt = code[0:strt].rfind('}')+1
  stop = strt
  count = 0

  for c in code[strt:]:
    stop += 1
    if c == '{':
      count += 1
    elif c == '}':
      count -= 1
      if count == 0:
        break

  return code[strt:stop+1].strip()

##
# A function for stripping C++ function prototypes from .h files
# @param code The code to extract from
# @param function The function prototype to extract
def stripCppPrototype(code, function):

  strt = code.find(function)
  strt = max(code[0:strt].rfind('}')+1, code[0:strt].rfind(';')+1)
  stop = strt
  count = 0

  for c in code[strt:]:
    stop += 1
    if c == '{':
      count += 1
    elif c == '}':
      count -= 1
      if count == 0:
        break

    if c == ';' and count == 0:
      break

  return code[strt:stop+1].strip('\n')

##
# A method for striping input file blocks
# @param code The input file
# @param block The block to remove
def stripInput(code, block):

  # Search for top-level block
  regex = '\[' + block + '\].*?\[\]'
  match = re.search(regex, code, re.DOTALL)
  if match:
    return match.group(0)

  # Search for sub-blocks
  strt = 0
  stop = 0
  found = False
  count = 0
  for m in re.finditer(r'(\s*\[\./(.*?)\])|(\[\.\./\])', code):
    if not found and m.groups()[1] == block:
      found = True
      strt = m.start()

    if found and m.groups()[0]:
      count += 1
    elif found and m.groups()[2]:
      count -= 1
      if count == 0:
        stop = m.end()
        break

  return code[strt:stop]



##
# A function for extracting portions of input files and C++ code for presentations
# @param code The code to extract from
# @param ext The file extension of the raw code
# @param strip The function or input file block to extract
def stripCode(code, ext, strip):

  if ext == '.C':
    code = stripCppFunction(code, strip)
  elif ext == '.h':
    code = stripCppPrototype(code, strip)
  elif ext == '.i':
    code = stripInput(code, strip)

  return code
