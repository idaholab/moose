##
# Color the supplied text string.
# @param input_str The string to color
# @param color The color to apply to the input (RED, GREEN, CYAN, YELLOW, MAGENTA, or BOLD)
# @param html If true usex html for more advanced coloring schemes. \verbatim<r>,<g>,<y>,<b>\endverbatim, all colors are bold
# @return the color string
def colorText(input_str, color, html=False):

  # ANSI color codes for colored terminal output
  color_codes = {'RESET':'\033[0m',
                 'BOLD':'\033[1m',
                 'RED':'\033[31m',
                 'GREEN':'\033[32m',
                 'CYAN':'\033[34m',
                 'YELLOW':'\033[33m',
                 'MAGENTA':'\033[35m'}

  # Perform coloring
  if html:
    input_str = input_str.replace('<r>', color_codes['BOLD'] + color_codes['RED'])
    input_str = input_str.replace('<c>', color_codes['BOLD'] + color_codes['CYAN'])
    input_str = input_str.replace('<g>', color_codes['BOLD'] + color_codes['GREEN'])
    input_str = input_str.replace('<y>', color_codes['BOLD'] + color_codes['YELLOW'])
    input_str = input_str.replace('<b>', color_codes['BOLD'])
    input_str = re.sub(r'</[rcgyb]>', color_codes['RESET'], input_str)
  else:
    input_str = color_codes[color] + input_str + color_codes['RESET']

  return input_str
