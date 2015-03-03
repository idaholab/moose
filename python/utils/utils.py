import os, re

##
# A function for coloring text
def colorText(str, color, **kwargs):

  # Get the properties
  html = kwargs.pop('html', False)
  code = kwargs.pop('code', False)
  colored = kwargs.pop('colored', True)

  # ANSI color codes for colored terminal output
  color_codes = {'RESET':'\033[0m','BOLD':'\033[1m','RED':'\033[31m','GREEN':'\033[35m','CYAN':'\033[34m','YELLOW':'\033[33m','MAGENTA':'\033[32m'}
  if code:
    color_codes['GREEN'] = '\033[32m'
    color_codes['CYAN']  = '\033[36m'
    color_codes['MAGENTA'] = '\033[35m'

  if colored and not (os.environ.has_key('BITTEN_NOCOLOR') and os.environ['BITTEN_NOCOLOR'] == 'true'):
    if html:
      str = str.replace('<r>', color_codes['BOLD']+color_codes['RED'])
      str = str.replace('<c>', color_codes['BOLD']+color_codes['CYAN'])
      str = str.replace('<g>', color_codes['BOLD']+color_codes['GREEN'])
      str = str.replace('<y>', color_codes['BOLD']+color_codes['YELLOW'])
      str = str.replace('<b>', color_codes['BOLD'])
      str = re.sub(r'</[rcgyb]>', color_codes['RESET'], str)
    else:
      str = color_codes[color] + str + color_codes['RESET']
  elif html:
    str = re.sub(r'</?[rcgyb]>', '', str)    # strip all "html" tags

  return str
