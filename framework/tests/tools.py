import os, sys, re
from subprocess import *
from optparse import OptionParser

#######################################
#######################################
### Class ExcelWriter
#######################################
#######################################
class ExcelWriter:
  ''' This class writes Excel data from a mostly generic nested data structure'''
  def __init__(self, file_name):
    import xlwt

    self.sheets = []
    self.file_name = file_name
    self.wb = xlwt.Workbook()


  def writeExcel(self, sheet_name, struct):
    # Excel has a 31 character limit on tab names
    sheet_name = sheet_name[:31]

    if sheet_name in self.sheets:
      self.curr_ws = self.wb.get_sheet(self.sheets.index(sheet_name))
      row = self.curr_ws.last_used_row + 5
      col = 0
    else:
      self.curr_ws = self.wb.add_sheet(sheet_name)
      self.sheets.append(sheet_name)
      row, col = (0, 0)        
    self.writeData(row, col, struct)


  def writeData(self, row, col, struct):
    if type(struct) is str:
      self.curr_ws.write(row, col, struct)
    elif type(struct) is list:
      for item in struct:
        (row, col) = self.writeData(row, col+1, item)
    elif type(struct) is dict:
      # Make sure the "Event" key comes out first - everything else can remain unsorted
      for key, value in sorted(struct.iteritems(), cmp=lambda a, b: (1, -1)[a[0] == 'Event']):
        self.curr_ws.write(row, col, key)
        # see if the value is complex (i.e. a nested structure or not)
        if type(value) is str:
          self.writeData(row, col+1, value)
        else:
          (row, garbage) = self.writeData(row+1, col, value)
        row+=1
    return (row, col)


  def close(self):
    if os.path.exists(self.file_name):
      # clobber the file
      os.remove(self.file_name);
    self.wb.save(self.file_name)
#######################################
#######################################

#######################################
#######################################
### Class PerfLog Parser
#######################################
#######################################
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
#######################################
#######################################


#######################################
#######################################
### Class TestHarness
#######################################
#######################################

global_exec_name = ''

import timeit, inspect, StringIO
class TestHarness:
  def __init__(self, argv, exec_name):
    global global_exec_name
    global_exec_name = exec_name

    # parse command line args
    self.options, self.leftovers, self.arg_string = self.getoptions(argv)

    # Emulate the standard Nose RegEx for consistency
    self.test_match = re.compile(r"(?:^|\b|[_-])[Tt]est")

    # See if an Excel dump was requested
    if self.options.xlsFile:
      self.xls_writer = ExcelWriter(self.options.xlsFile)
    else:
      self.xls_writer = None

  def run_tests(self):
    out_string = ''
    results_table = ''
    test_counter = 0
    test_dir = os.getcwd()

    start = timeit.default_timer()
    for dirpath, dirnames, filenames in os.walk(test_dir):
      if (self.test_match.search(dirpath)):
        for file in filenames:
          # See if there were other arguments (test names) passed on the command line
          if (file[-2:] == 'py' and 
              self.test_match.search(file) and
              (len(self.leftovers) == 0 or len([item for item in self.leftovers if re.match(item, file)]) == 1)):
            module_name = file[:-3]

            result, result_string = self.inspectAndTest(dirpath, module_name)
            out_string += result_string
            if (result != 'skipped'):
              test_counter += 1
	    cnt = 70 - len(file + result) - 2
	    s = file + " " + '.'*cnt + " " + result
            print s
            results_table += s + "\n"
    end = timeit.default_timer()

    if self.xls_writer:
      self.xls_writer.close()
    return out_string, results_table, test_counter, str(round(end-start,3))


  def inspectAndTest(self, dirpath, module_name):
    saved_cwd = os.getcwd()  
    sys.path.append(os.path.abspath(dirpath))
    os.chdir(dirpath)

    # dynamically load the module
    module = __import__(module_name)

    result_string = ''
    # inspect the routine and look for test functions
    for routine, address in inspect.getmembers(module, inspect.isroutine):
      if self.test_match.search(routine):
                 
        test_result = 'ok'
        supported_args = set(inspect.getargspec(address)[0])
        try:
          # See if this test function supports benchmarking or parallel requests
          if ((self.arg_string.find('dofs') >= 0 and not 'dofs' in supported_args) or (self.arg_string.find('np') >=0 and not 'np' in supported_args)):
            test_result = 'skipped'
          else:
            test_start = timeit.default_timer()
               
            # Capture stdout to a buffer object for the local function call
            saved_stdout = sys.stdout
            capture = StringIO.StringIO()
            sys.stdout = capture

            eval('address(' + self.arg_string + ')')
                  
            sys.stdout = saved_stdout
            test_end = timeit.default_timer()
            if (self.arg_string == ''):
              test_result = 'OK'
            else:
              test_result = str(round(test_end-test_start,3)) + 's'
              if self.xls_writer:
                parser = PerfLogParser()
                parser.parse(capture.getvalue())
                parser.writeExcel(self.xls_writer, module_name)
            if (self.options.verbose == True):
              result_string += capture.getvalue()
                  
                        
        except AssertionError:
          sys.stdout = saved_stdout
          test_end = timeit.default_timer()

          result_string += capture.getvalue()
          test_result = 'FAILED'

    os.chdir(saved_cwd)
    sys.path.pop()

    return (test_result, result_string)
     

  def getoptions(self, argv): 
    # Callback function
    arg_vector = []
    def buildArgVector(option, opt_str, value, parser):
      arg_vector.append(option.dest + '=' +  str(value)) 

    parser = OptionParser()
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False, help="show more output [default=FALSE]")
    parser.add_option("-x", "--xls", action="store", dest="xlsFile", metavar="FILE", help="write excel format performance data to FILE")
    parser.add_option("-d", "--dofs", action="callback", callback=buildArgVector, type="int", dest="dofs", help="refine each example to meet the minimum requested DOFS")
    parser.add_option("-n", "--np", action="callback", callback=buildArgVector, type="int", dest="np", help="specify the number of MPI processes launched")

    (options, args) = parser.parse_args(argv[1:])
    # Return the 'options' and leftover args returned from parse_args and
    # also the arg_string made from joining the arguments in the arg_vector
    return (options, args, ','.join(arg_vector))


  
################ Testing Functions Go Here outside of the TestHarness #################
def executeCommand(command):
  print 'Executing: ' + command

  p = Popen([command],stdout=PIPE,stderr=STDOUT, close_fds=True, shell=True)
  return p.communicate()[0]


def delOldOutFiles(test_dir, out_files):
  for file in out_files:
    try:
      os.remove(os.path.join(test_dir,file))
    except:
      pass

def executeApp(test_dir, input_file, min_dofs=0, parallel=0):
  saved_cwd = os.getcwd()
  os.chdir(test_dir)
  command = global_exec_name + ' -i ' + input_file
  if (parallel):
    command = 'mpiexec -np ' + str(parallel) + ' ' + command  
  if (min_dofs):
    command = 'time ' + command + ' --dofs ' + str(min_dofs)
  stdout = executeCommand(command)
  print stdout
  os.chdir(saved_cwd)

def executeExodiff(test_dir, out_files):
  for file in out_files:
    command = 'exodiff -F 1e-11 -use_old_floor -t 5.5E-6 ' + os.path.join(test_dir,file) + ' ' + os.path.join(test_dir,'gold',file)
    print command
    stdout = executeCommand(command)
    print stdout
    if stdout.find('different') != -1 or stdout.find('ERROR') != -1 or stdout.find('command not found') != -1:
      assert False

def executeAppAndDiff(test_file, input_file, out_files, min_dofs=0, parallel=0):
  test_dir = os.path.dirname(test_file)
  delOldOutFiles(test_dir, out_files)
  executeApp(test_dir, input_file, min_dofs, parallel)
  if (min_dofs == 0 and parallel == 0):
    executeExodiff(test_dir, out_files)






  
