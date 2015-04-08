import os, re

##
# A function for coloring text
def colorText(string, color, **kwargs):

  # Get the properties
  html = kwargs.pop('html', False)
  code = kwargs.pop('code', True)
  colored = kwargs.pop('colored', True)

  # ANSI color codes for colored terminal output
  color_codes = {'RESET':'\033[0m','BOLD':'\033[1m','RED':'\033[31m','GREEN':'\033[35m','CYAN':'\033[34m','YELLOW':'\033[33m','MAGENTA':'\033[32m'}
  if code:
    color_codes['GREEN'] = '\033[32m'
    color_codes['CYAN']  = '\033[36m'
    color_codes['MAGENTA'] = '\033[35m'

  if colored and not (os.environ.has_key('BITTEN_NOCOLOR') and os.environ['BITTEN_NOCOLOR'] == 'true'):
    if html:
      string = string.replace('<r>', color_codes['BOLD']+color_codes['RED'])
      string = string.replace('<c>', color_codes['BOLD']+color_codes['CYAN'])
      string = string.replace('<g>', color_codes['BOLD']+color_codes['GREEN'])
      string = string.replace('<y>', color_codes['BOLD']+color_codes['YELLOW'])
      string = string.replace('<b>', color_codes['BOLD'])
      string = re.sub(r'</[rcgyb]>', color_codes['RESET'], string)
    else:
      string = color_codes[color] + string + color_codes['RESET']
  elif html:
    string = re.sub(r'</?[rcgyb]>', '', string)    # stringip all "html" tags

  return string


##
# A function for converting string to boolean
def str2bool(string):
  string = string.lower()
  if string is 'true' or string is '1':
    return True
  else:
    return False
