import re

class PerfLogParser:
  ''' This class parses text from the PerfLog output in Libmesh '''
  def parse(self, text):
    self.text = text
    self.start_columns = []
    self.general_info = {}
    self.struct = {}

    self.parseGeneralInformation()
    self.parsePerfTables()


  def parseGeneralInformation(self):
    for string in ("Mesh Information", "EquationSystems"):
      m = re.search(r'(' + string + r'.*?^$)', self.text, re.M | re.S)
      if m:
        for l in m.group(0).splitlines():
          v = l.strip().split('=', 2)
          if len(v) == 1:
            v.append('')
          self.general_info[v[0]] = v[1]


  def getStartColumns(self, line):
    # Avoid any traps with whitespace by parsing from the performance data (digits and decimals)
    self.start_columns = []
    for m in re.finditer(r'[\d\.]+', line):
      self.start_columns.append(m.start(0))

    #Add a bogus start column at the begin and end for convenience in string slicing later
    self.start_columns.insert(0, 0)
    self.start_columns.append(len(line))


  def parsePerfTables(self):
    #Look for the output tables denoted by the word 'Performance' in the heading
    for table in re.finditer(r'^\|([^\n]*?Performance).*?^\| Totals.*?$', self.text, re.M | re.S):
      section = table.group(1).strip()
      if section in self.struct: continue # skip over duplicate tables

      # Grab the Active and Alive times if they exist
      (alive_time, active_time)  = (None, None)
      alive_match = re.search(r'Alive time=([\d\.]+)', table.group(0))
      if (alive_match):
        alive_time = alive_match.group(1)
        active_match = re.search(r'Active time=([\d\.]+)', table.group(0))
      if (active_match):
        active_time = active_match.group(1)

      self.struct[section] = {'alive_time': alive_time, 'active_time': active_time, 'data': {}}

      # Split the tables further on the rows of hyphens
      sections = re.split(r'\|?-+\|?', table.group(0))

      # Start with the second to last section since this will contain the routine runtimes and
      # a list of start columns can be determined then parse the last section next
      curr_subtitle = ''
      headers = []
      for section_number in (-2, -1, 1):
        lines = re.split("\n", sections[section_number])
        for l in lines:
          # chop off the first and last characters of the ASCII table art
          l = l[1:-1]
          # skip lines containing only  whitespace
          if re.match(r'^\s*$', l): continue

          # see if this is a subtitle line
          m = re.match(r'^ (\S+)(.*)', l)
          if m:
            curr_subtitle = m.group(1)
            curr_handle = self.struct[section]['data'][curr_subtitle] = {}
            if (m.group(2).strip() == ''):
              continue # skip lines that have no more data to parse

          data = []
          if not self.start_columns:
            self.getStartColumns(l)

          # parse data by columns not by tokenizing!
          for i in range(1, len(self.start_columns)):
            data.append(l[self.start_columns[i-1]:self.start_columns[i]].strip())

          key = data.pop(0)
          if not key:
            key = curr_subtitle

          # Glue the column data into our data structure but see if there is already
          # data in the structure under this key in case the data spans rows
          inserted = 0
          if key in curr_handle:
            for i in range(len(curr_handle[key])):
              curr_handle[key][i] += ' ' + data[i]
            inserted = 1
          if not inserted:
            curr_handle[key] = data

    #### Uncomment the following lines to view the raw datastructure
    # import pprint
    # pprint.pprint(struct)

  def writeExcel(self, xls_handle, sheet_name):
    xls_handle.writeExcel(sheet_name, self.struct)
    xls_handle.writeExcel(sheet_name, self.general_info)
