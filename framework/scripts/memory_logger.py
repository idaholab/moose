#!/usr/bin/env python
import sys, os, subprocess, socket, pickle, argparse, time, decimal, csv
from tempfile import TemporaryFile

class MemoryPlotter:
  def __init__(self, arguments):
    self.arguments = arguments
    self.buildGraph()

  def buildPlots(self):
    plot_dictionary = {}
    for log in self.arguments.plot:
      memory_list = []
      if os.path.exists(log):
        log_file = open(log, 'r')
        reader = csv.reader(log_file, delimiter=',', quotechar='|', escapechar='\\', quoting=csv.QUOTE_MINIMAL)
        for row in reader:
          memory_list.append(row)
        log_file.close()
        plot_dictionary[log.split('/')[-1:][0]] = memory_list
      else:
        print 'log not found:', log
        sys.exit(1)
    return plot_dictionary

  def buildGraph(self):
    try:
      import matplotlib.pyplot as plt
    except ImportError:
      print 'Error importing matplotlib. Matplotlib not available on this system?'
      sys.exit(1)
    plot_dictionary = self.buildPlots()
    fig = plt.figure()
    plot_list = []
    tmp_plot = []
    tmp_legend = []
    self.stdout_msg = []
    self.pstack_msg = []
    self.multiples = 1
    self.memory_label = 'Memory in Bytes'

    # Try and calculate memory sizes, so we can move annotations around a bit more accurately
    largest_memory = []
    for plot_name, value_list in plot_dictionary.iteritems():
      for records in value_list:
        largest_memory.append(int(records[1]))
    largest_memory.sort()
    # TODO: Better way to round and determin the following? It looks stupid.
    if largest_memory[-1] < 1000000000:
      self.multiples = 1000000
      self.memory_label = 'Memory in Gigabytes'
    if largest_memory[-1] < 1000000:
      self.multiples = 1000
      self.memory_label = 'Memory in Megabytes'
    if largest_memory[-1] < 10000:
      self.multiples = 10
      self.memory_label = 'Memory in Kilobytes'
    if largest_memory[-1] < 1000:
      self.multiples = 1
      self.memory_label = 'Memory in Bytes'

    # Loop through each log file
    for plot_name, value_list in plot_dictionary.iteritems():
      plot_list.append(fig.add_subplot(111))
      tmp_memory = []
      tmp_time = []

      tmp_stdout_x = []
      tmp_stdout_y = []

      tmp_pstack_x = []
      tmp_pstack_y = []

      # Get the start time, and make this 0
      try:
        tmp_zero = decimal.Decimal(value_list[0][0])
      except:
        print 'Could not parse log file:', plot_name, 'is this a valid memory_logger file?'
        sys.exit(1)

      # Populate the graph
      for records in value_list:
        tmp_memory.append(decimal.Decimal(records[1]) / self.multiples)
        tmp_time.append(str(decimal.Decimal(records[0]) - tmp_zero))

        if len(records[2]) > 0 and self.arguments.stdout:
          tmp_stdout_x.append(tmp_time[-1])
          tmp_stdout_y.append(tmp_memory[-1])
          self.stdout_msg.append(records[2])

        if len(records[3]) > 0 and self.arguments.pstack:
          tmp_pstack_x.append(tmp_time[-1])
          tmp_pstack_y.append(tmp_memory[-1])
          self.pstack_msg.append(records[3])

      # Do the actual plotting:
      f, = plot_list[-1].plot(tmp_time, tmp_memory)
      tmp_plot.append(f)
      tmp_legend.append(plot_name)
      plot_list[-1].grid(True)
      plot_list[-1].set_ylabel(self.memory_label)
      plot_list[-1].set_xlabel('Time in Seconds')

      # Plot annotations
      if self.arguments.stdout:
        stdout_line, = plot_list[-1].plot(tmp_stdout_x, tmp_stdout_y, 'x', picker=10, color=f.get_color())
        stdout_line.set_gid('stdout')
        self.buildAnnotation(plot_list[-1], tmp_stdout_x, tmp_stdout_y, self.stdout_msg, f.get_color())

      if self.arguments.pstack:
        pstack_line, = plot_list[-1].plot(tmp_pstack_x, tmp_pstack_y, 'o', picker=10, color=f.get_color())
        pstack_line.set_gid('pstack')

    # Make points clickable
    fig.canvas.mpl_connect('pick_event', self)

    # Create legend
    plt.legend(tmp_plot, tmp_legend, loc = 2)

    plt.show()

  def __call__(self, event):
    color_codes = {'RESET':'\033[0m', 'r':'\033[31m','g':'\033[32m','c':'\033[36m','y':'\033[33m', 'b':'\033[34m', 'm':'\033[35m', 'k':'\033[0m', 'w':'\033[0m' }
    line = event.artist
    ind = event.ind
    if self.arguments.stdout and line.get_gid() == 'stdout':
      if self.arguments.no_color != False:
        print color_codes[line.get_color()]
      print "stdout -----------------------------------------------------\n"
      for id in ind:
        print self.stdout_msg[id]
      if self.arguments.no_color != False:
        print color_codes['RESET']

    if self.arguments.pstack and line.get_gid() == 'pstack':
      if self.arguments.no_color != False:
        print color_codes[line.get_color()]
      print "pstack -----------------------------------------------------\n"
      for id in ind:
        print self.pstack_msg[id]
      if self.arguments.no_color != False:
        print color_codes['RESET']

  def buildAnnotation(self,fig,x,y,msg,c):
    for i in range(len(x)):
      fig.annotate(str(msg[i].split('\n')[0][:self.arguments.trim_text[-1]]),
                   xy=(x[i], y[i]),
                   rotation=self.arguments.rotate_text[-1],
                   xytext=(decimal.Decimal(x[i]) + decimal.Decimal(self.arguments.move_text[0]), decimal.Decimal(y[i]) + decimal.Decimal(self.arguments.move_text[1])),
                   color=c, horizontalalignment='center', verticalalignment='bottom',
                   arrowprops=dict(arrowstyle="->",
                                   connectionstyle="arc3,rad=0.5",
                                   color=c
                                 )
                 )

class ExportMemoryUsage:
  """Converts a log file to a comma delimited format (for Matlab)
"""
  def __init__(self, arguments):
    self.arguments = arguments
    if os.path.exists(self.arguments.export[-1]):
      history_file = open(self.arguments.export[-1], 'r')
    else:
      print 'Input file not found:', self.arguments.export[-1]
      sys.exit(1)

    reader = csv.reader(history_file, delimiter=',', quotechar='|', escapechar='\\', quoting=csv.QUOTE_MINIMAL)
    self.memory_list = []
    for row in reader:
      self.memory_list.append(row)
    history_file.close()

    self.sorted_list = []
    self.mem_list = []
    self.exportFile()

  def exportFile(self):
    output_file = open(self.arguments.export[-1] + '.comma_delimited', 'w')
    for timestamp in self.memory_list:
      time_object = GetTime(float(timestamp[0]))
      self.mem_list.append(timestamp[1])
      self.sorted_list.append([str(time_object.year) + '-' + str(time_object.month) + '-' + str(time_object.day) + ' ' + str(time_object.hour) + ':' + str(time_object.minute) + ':' + str(time_object.second) + '.' + str(time_object.microsecond), timestamp[1]])
    for item in self.sorted_list:
      output_file.write(str(item[0]) + ',' + str(item[1]) + '\n')
    output_file.close()
    print 'Comma delimited file saved to: ' + os.getcwd() + '/' + str(self.arguments.export[-1]) + '.comma_delimited'

class WriteLog:
  """The logfile object
"""
  def __init__(self, log_file):
    self.file_object = open(log_file, 'w')
    self.log_file = csv.writer(self.file_object, delimiter=',', quotechar='|', escapechar='\\', quoting=csv.QUOTE_MINIMAL)

  def write_file(self, data):
    self.log_file.writerow(data)

  def close(self):
    self.file_object.close()

class SamplerUtils:
  """This class contains the logging facilities as well as
the methods to retrieve memory usage and stack traces.
"""
  def __init__(self, arguments):
    self.arguments = arguments
    self.log = TemporaryFile()
    self.last_position = 0

  def __del__(self):
    try:
      self.log.close()
    except:
      pass

  def _discover_name(self):
    if self.arguments.run:
      for item in self.arguments.run[-1].split():
        if os.path.exists(item):
          return item.split('/').pop()
    elif self.arguments.call_back_host:
      return self.arguments.agent_run

  # Gets the current stdout/err
  def read_log(self):
    self.log.seek(self.last_position)
    output = self.log.read()
    self.last_position = self.log.tell()
    sys.stdout.write(output)
    return output

  # return a dictionary of PIDs and their memory_usage
  def get_pids(self):
    pid_list = {}
    if self.arguments.track == None:
      command = ['ps', '-e', '-o', 'pid,rss,user,comm']
      tmp_proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      all_pids = tmp_proc.communicate()[0].split('\n')
      search_for = self._discover_name()
      for single_pid in all_pids:
        if single_pid.find(search_for) > -1 and single_pid.find(os.getenv('USER')) > -1:
          pid_list[single_pid.split()[0]] = []
          pid_list[single_pid.split()[0]].append(single_pid.split()[1])
    else:
      command = ['ps', str(self.arguments.track[-1]), '-e', '-o', 'pid,rss,user,comm']
      tmp_proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      all_pids = tmp_proc.communicate()[0].split('\n')
      for single_pid in all_pids:
        if single_pid.find(self.arguments.track[-1]) > -1:
          pid_list[single_pid.split()[0]] = []
          pid_list[single_pid.split()[0]].append(single_pid.split()[1])
    if pid_list == {}:
      return None
    return pid_list

  # Get total memory usage
  def read_ps(self):
    memory_usage = 0
    pid_list = self.get_pids()
    if pid_list == None:
      return None
    for single_pid in pid_list:
      for single_usage in pid_list[single_pid]:
        memory_usage += int(single_usage)
    if memory_usage == 0:
      return None
    return int(memory_usage)

  # Get Stack Trace information from first discovered PID
  def read_pstack(self):
    if self.arguments.pstack:
      pid_list = self.get_pids()
      if pid_list == None:
        return ''
      for single_pid in pid_list:
        # This is very expensive on some systems (~1 second). Hence we only do this once.
        tmp_proc = subprocess.Popen(['pstack', str(single_pid)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return tmp_proc.communicate()[0]
    else:
      return ''

class Report:
  """This class handles all the actual data.
"""
  def __init__(self, arguments):
    self.args = arguments
    self.changes = False
    self.sampler = SamplerUtils(self.args)
    self.writer = WriteLog(self.args.outfile[-1])
    if self.args.node_list == None:
      self.args.node_list = [ self.args.my_hostname ]
    self.current_results = { 'HOST' : {},
                             'REPORTING_HOST'  : '',
                             'TOTAL'           : 0,
                             'REQ'             : False,
                             'REQ_DATA'        : {},
                             'RUN'             : True
                             }
    for node in self.args.node_list:
      self.current_results['HOST'][node] = { 'PSTACK' : '',
                                             'LOG'    : '',
                                             'TOTAL'  : 0
                                             }
    self.last_results = self.current_results.copy()

    self.initialize = { 'REPORTING_HOST' : self.args.my_hostname,
                        'REQ'            : True,
                        'REQ_DATA'       : { 'agent_run'   : None,
                                             'repeat_rate' : None,
                                             'pstack'      : None,
                                             'pbs_delay'   : None
                                             }
                        }

  def __del__(self):
    self.writer.close()

  def getReport(self):
    if self.args.pbs:
      # If we are the server, only get stdout
      self.current_results['HOST'][self.current_results['REPORTING_HOST']]['LOG']  = self.sampler.read_log()
    else:
      self.current_results['HOST'][self.args.my_hostname]['TOTAL']  = self.sampler.read_ps()
      self.current_results['HOST'][self.args.my_hostname]['PSTACK'] = self.sampler.read_pstack()
      self.current_results['HOST'][self.args.my_hostname]['LOG']  = self.sampler.read_log()
      self.current_results['REPORTING_HOST'] = self.args.my_hostname
    self.updateReport()

  def updateReport(self):
    total_mem = 0
    for key in self.args.node_list:
      if self.current_results['HOST'][key]['TOTAL'] != None:
        total_mem += int(self.current_results['HOST'][key]['TOTAL'])
      else:
        if self.args.pbs != True:
          print 'Informing server all processes have stopped.'
          self.current_results['RUN'] = False
          self.current_results['HOST'][key]['TOTAL'] = 0
    self.current_results['TOTAL'] = total_mem
    if self.last_results != self.current_results:
      self.changes = True
      for key in self.last_results.keys():
        if key != 'HOST':
          self.last_results[key] = self.current_results[key]
      for key in self.last_results['HOST'].keys():
        self.last_results['HOST'][key] = self.current_results['HOST'][key]
    if self.args.pbs:
      self.writeResults()

  def writeResults(self):
    self.changes = False
    data = [ GetTime().now,
             self.last_results['TOTAL'],
             self.last_results['HOST'][self.current_results['REPORTING_HOST']]['LOG'],
             self.last_results['HOST'][self.current_results['REPORTING_HOST']]['PSTACK'],
             self.last_results['REPORTING_HOST'],
             self.last_results['HOST'][self.current_results['REPORTING_HOST']]['TOTAL']
             ]
    self.writer.write_file(data)

class ReadLog:
  """Read a memory_logger log file, and display the results to stdout in an easy to read form.
"""
  def __init__(self, arguments):
    self.arguments = arguments
    history_file = open(self.arguments.read[-1], 'r')
    reader = csv.reader(history_file, delimiter=',', quotechar='|', escapechar='\\', quoting=csv.QUOTE_MINIMAL)
    self.memory_list = []
    for row in reader:
      self.memory_list.append(row)
    history_file.close()
    self.sorted_list = []
    self.mem_list = []
    self.use_nodes = False
    self.printHistory()

  def printHistory(self):
    RESET  = '\033[0m'
    BOLD   = '\033[1m'
    BLACK  = '\033[30m'
    RED    = '\033[31m'
    GREEN  = '\033[32m'
    CYAN   = '\033[36m'
    YELLOW = '\033[33m'
    last_memory = 0.0
    (terminal_width, terminal_height) = self.getTerminalSize()
    for timestamp in self.memory_list:
      to = GetTime(float(timestamp[0]))
      total_memory = int(timestamp[1])
      log = timestamp[2].split('\n')
      pstack = timestamp[3].split('\n')
      node_name = str(timestamp[4])
      node_memory = int(timestamp[5])

      self.mem_list.append(total_memory)
      self.sorted_list.append([str(to.day) + ' ' + str(to.monthname) + ' ' + str(to.hour) + ':' + str(to.minute) + ':' + '{:02.0f}'.format(to.second) + '.' + '{:06.0f}'.format(to.microsecond), total_memory, log, pstack, node_name, node_memory])

    largest_memory = decimal.Decimal(max(self.mem_list))
    if len(set([x[4] for x in self.sorted_list])) > 1:
      self.use_nodes = True

    print 'Date Stamp' + ' '*int(17) + 'Memory Usage | Percent of MAX memory used: ( ' + str('{:0,.0f}'.format(largest_memory)) + ' K )'
    for item in self.sorted_list:
      tmp_str = ''
      if decimal.Decimal(item[1]) == largest_memory:
        tmp_str = self.formatText(largest_memory, item[0], item[1], item[5], item[2], item[3], item[4], RESET, terminal_width)
      elif item[1] > last_memory:
        tmp_str = self.formatText(largest_memory, item[0], item[1], item[5], item[2], item[3], item[4], RED, terminal_width)
      elif item[1] == last_memory:
        tmp_str = self.formatText(largest_memory, item[0], item[1], item[5], item[2], item[3], item[4], CYAN, terminal_width)
      else:
        tmp_str = self.formatText(largest_memory, item[0], item[1], item[5], item[2], item[3], item[4], GREEN, terminal_width)
      last_memory = item[1]
      sys.stdout.write(tmp_str)
    print 'Date Stamp' + ' '*int(17) + 'Memory Usage | Percent of MAX memory used: ( ' + str('{:0,.0f}'.format(largest_memory)) + ' K )'


  def formatText(self, largest_memory, date, total_memory, node_memory, log, pstack, reporting_host, color_code, terminal_width):
    RESET  = '\033[0m'
    if decimal.Decimal(total_memory) == largest_memory:
      percent = '100'
    elif (decimal.Decimal(total_memory) / largest_memory) ==  0:
      percent = '0'
    else:
      percent = str(decimal.Decimal(total_memory) / largest_memory)[2:4] + '.' + str(decimal.Decimal(total_memory) / largest_memory)[4:6]

    header = len(date) + 18
    footer = len(percent) + 6
    additional_correction = 0
    max_length = decimal.Decimal(terminal_width - header) / largest_memory
    total_position = total_memory * decimal.Decimal(max_length)
    node_position = node_memory * decimal.Decimal(max_length)
    tmp_log = ''
    if self.arguments.stdout:
      for single_log in log:
        if single_log != '':
          tmp_log += ' '*(header - len(' stdout |')) + '  stdout | ' + single_log + '\n'
    if self.arguments.pstack:
      for single_pstack in pstack:
        if single_pstack != '':
          tmp_log += ' '*(header - len(' pstack |')) + '  pstack | ' + single_pstack + '\n'

    if self.arguments.separate and self.use_nodes != False:
      message = '< ' + RESET + reporting_host + ' - ' + '{:10,.0f}'.format(node_memory) + ' K' + color_code + ' >'
      additional_correction = len(RESET) + len(color_code)
    elif self.use_nodes:
      message = '< >'
    else:
      node_position = 0
      message = ''
    return date + '{:15,.0f}'.format(total_memory) + ' K | ' + color_code + '-'*int(node_position) + message + '-'*(int(total_position) - (int(node_position) + ((len(message) - additional_correction) + footer))) + RESET + '| ' + percent + '%\n' + tmp_log



  def getTerminalSize(self):
    """Quicky to get terminal window size"""
    env = os.environ
    def ioctl_GWINSZ(fd):
      try:
        import fcntl, termios, struct, os
        cr = struct.unpack('hh', fcntl.ioctl(fd, termios.TIOCGWINSZ, '1234'))
      except:
        return None
      return cr
    cr = ioctl_GWINSZ(0) or ioctl_GWINSZ(1) or ioctl_GWINSZ(2)
    if not cr:
      try:
        fd = os.open(os.ctermid(), os.O_RDONLY)
        cr = ioctl_GWINSZ(fd)
        os.close(fd)
      except:
        pass
    if not cr:
      try:
        cr = (env['LINES'], env['COLUMNS'])
      except:
        cr = (25, 80)
    return int(cr[1]), int(cr[0])

class ParseResults:
  """A class to read the socket file object into the running dictionary
"""
  def __init__(self, arguments, stdout_log, launched_job, node_list):
    self.stdout_log = stdout_log
    self.arguments = arguments
    self.server_stop = False
    self.r = Reporter(self.arguments, launched_job, node_list)

  def parse(self, client_data):
    try:
      if client_data != None:
        t = TemporaryFile()
        t.write(client_data)
        t.seek(0)
        reported_data = pickle.load(t)
        t.close()
        self.parseData(reported_data)
    except ValueError:
      print 'Error unpacking dictionary. Not a dictionary:', str(reported_data)
      self.r.current_report[self.arguments.my_hostname]['RUN'] = False

  def parseData(self, reported_data):
    self.r.current_results[reported_data.keys()[0]] = reported_data[reported_data.keys()[0]]
    self.r.current_results['REPORTING_HOST'] = reported_data.keys()[0]
    self.r.current_results[reported_data.keys()[0]]['TIME'] = GetTime().now
    if self.r.current_results[self.arguments.my_hostname]['RUN'] == False:
      self.server_stop = True
    self.r.getReport()

class GetTime:
  """A simple formatted time object.
"""
  def __init__(self, posix_time=None):
    import datetime
    if posix_time == None:
      self.posix_time = datetime.datetime.now()
    else:
      self.posix_time = datetime.datetime.fromtimestamp(posix_time)
    self.now = str(datetime.datetime.now().strftime('%s.%f'))
    self.microsecond = self.posix_time.microsecond
    self.second = self.posix_time.second
    self.minute = self.posix_time.strftime('%M')
    self.hour = self.posix_time.strftime('%H')
    self.day = self.posix_time.strftime('%d')
    self.month = self.posix_time.strftime('%m')
    self.year = self.posix_time.year
    self.dayname = self.posix_time.strftime('%a')
    self.monthname = self.posix_time.strftime('%b')

class ClientParser:
  """This class contains the logic for recieved data.
"""
  def __init__(self, report):
    self.r = report

  def accept(self, client):
    msg = ''
    data_len = int(client[0].recv(8))
    while len(msg) < data_len:
      msg = msg + str(client[0].recv(1024))
    self.parse_message(self._unpickle_message(msg), client)

  def _unpickle_message(self, message):
    t = TemporaryFile()
    t.write(message)
    t.seek(0)
    try:
      message = pickle.load(t)
      t.close()
      return message
    except KeyError:
      print 'Socket data was not pickled data'
    except:
      raise

  def _pickle_message(self, message):
    t = TemporaryFile()
    pickle.dump(message, t)
    t.seek(0)
    str_msg = t.read()
    str_len = len(str_msg)
    message = "%-8d" % (str_len,) + str_msg
    t.close()
    return message

  def parse_message(self, dictionary_data, client):
    reporting_node = dictionary_data['REPORTING_HOST']
    if dictionary_data['REQ'] == True:
      self.determine_request(dictionary_data, client)
    else:
      self.r.current_results['HOST'][reporting_node] = dictionary_data['HOST'][reporting_node]
      self.r.current_results['REPORTING_HOST'] = reporting_node
      self.r.current_results['RUN'] = dictionary_data['RUN']
      self.r.getReport()

  def determine_request(self, message, client):
    # Right now, we only support the request for arguments
    for request in message['REQ_DATA'].keys():
      for request_type in dir(self.r.args):
        if str(request) == str(request_type):
          message['REQ_DATA'][request] = getattr(self.r.args, request)
    message['REQ'] = False
    client[0].sendall(self._pickle_message(message))

class ServerAgent:
  """This class creates a listing socket and passes any connection attempts to the client parser.
ServerAgent also creates the reporter instance and launches all reporter agents.
"""
  def __init__(self, arguments):
    self.args = arguments
    self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.server.bind((socket.gethostname(), 0))
    self.server.listen(5)
    (self.host, self.port) = self.server.getsockname()
    self.report = Report(self.args)
    self.cp = ClientParser(self.report)
    print 'Memory Logger running on:', self.host, self.port, '\nNodes:', ', '.join(self.args.node_list), '\nSample rate (including stdout):', self.args.repeat_rate[-1], 's\nDelaying', self.args.pbs_delay[-1], 'seconds before agents attempt tracking\n'

  def start_server(self):
    self.launch_agents()
    try:
      while self.report.current_results['RUN']:
        client_connection = self.server.accept()
        self.cp.accept(client_connection)
    except KeyboardInterrupt:
      print 'Canceled by user'
      sys.exit(0)
    if self.args.pbs_delay[-1] < 1:
      print 'Application terminated. If termination was unexpected, try increasing the --pbs-delay value.'
      sys.exit(0)
    print 'Application terminated. Wrote to file:', self.args.outfile[-1]

  def launch_agents(self):
    self.args.agent_run = self.report.sampler._discover_name()
    if self.args.agent_run == None:
      print 'No binary detected. Exiting...'
      sys.exit(1)
    else:
      print '\nInstructing agents to track:', self.args.agent_run

    self.args.process = subprocess.Popen(self.args.run[-1].split(), stdout=self.report.sampler.log, stderr=self.report.sampler.log)
    for node in self.args.node_list:
      if self.args.pstack:
        command = [  'ssh',
                     node,
                     'bash --login -c "source /etc/profile && ' \
                     + 'sleep ' + str(self.args.pbs_delay[-1]) + ' && ' \
                     + os.path.abspath(__file__) \
                     + ' --pstack --call-back-host ' \
                     + self.host + ' ' + str(self.port) \
                     + '"']
      else:
        command = [  'ssh',
                     node,
                     'bash --login -c "source /etc/profile && ' \
                     + 'sleep ' + str(self.args.pbs_delay[-1]) + ' && ' \
                     + os.path.abspath(__file__) \
                     + ' --call-back-host ' \
                     + self.host + ' ' + str(self.port) \
                     + '"']
      subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

class ClientConnection:
  """This class controls the data connection between client and server.
server = (( 'server hostname', int(port) ))
"""
  def __init__(self, server):
    self.server = server
    self.client = None

  def close(self):
    try:
      self.client.close()
    except:
      pass

  def send(self, message):
    try:
      self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.client.settimeout(15)
      self.client.connect(self.server)
      self.client.sendall(self._pickle_message(message))
    except:
      print 'Connection timeout'
      sys.exit(0)

  def receive(self, message):
    msg = ''
    try:
      self.client = socket.socket()
      self.client.settimeout(15)
      self.client.connect(self.server)
      self.client.sendall(self._pickle_message(message))
      data_len = int(self.client.recv(8))
      while len(msg) < data_len:
        msg = msg + str(self.client.recv(1024))
      return self._unpickle_message(msg)
    except:
      print 'Connection timeout'
      sys.exit(0)

  def _unpickle_message(self, message):
    t = TemporaryFile()
    t.write(message)
    t.seek(0)
    try:
      return pickle.load(t)
    except KeyError:
      print 'Socket data was not pickled data'
    except:
      raise

  def _pickle_message(self, message):
    t = TemporaryFile()
    pickle.dump(message, t)
    t.seek(0)
    str_msg = t.read()
    str_len = len(str_msg)
    message = "%-8d" % (str_len,) + str_msg
    t.close()
    return message


##########################################################

def trackMemory(args):
  if args.pbs:
    node_list = []
    node_file = open(os.getenv('PBS_NODEFILE'), 'r')
    tmp_list = set(node_file.read().split())
    for item in tmp_list:
      node_list.append(socket.getfqdn(item))
    node_file.close()
    args.node_list = node_list
    ServerAgent(args).start_server()
  elif args.call_back_host:
    agent = ClientConnection((args.call_back_host[0], int(args.call_back_host[1])))
    args.node_list = None
    report = Report(args)
    for initialize_item in report.initialize.keys():
      report.current_results[initialize_item] = report.initialize[initialize_item]
    print report.current_results
    report.current_results = agent.receive(report.current_results)
    print report.current_results
    agent.close()
    args.repeat_rate = report.current_results['REQ_DATA']['repeat_rate']
    args.pstack = report.current_results['REQ_DATA']['pstack']
    args.agent_run = report.current_results['REQ_DATA']['agent_run']
    while report.current_results['RUN']:
      report.getReport()
      if report.changes:
        agent = ClientConnection((args.call_back_host[0], int(args.call_back_host[1])))
        agent.send(report.current_results)
        agent.close()
      time.sleep(args.repeat_rate[-1])
  else:
    args.node_list = None
    report = Report(args)
    args.process = subprocess.Popen(args.run[-1].split(), stdout=report.sampler.log, stderr=report.sampler.log)
    time.sleep(.5)
    try:
      while report.current_results['RUN']:
        report.getReport()
        if report.changes:
          report.writeResults()
        time.sleep(args.repeat_rate[-1])
    except KeyboardInterrupt:
      print 'Canceled by user'
      sys.exit(0)
    except:
      raise
    print 'Application terminated. Wrote to file:', args.outfile[-1]

def which(program):
  def is_exe(fpath):
    return os.path.exists(fpath) and os.access(fpath, os.X_OK)
  fpath, fname = os.path.split(program)
  if fpath:
    if is_exe(program):
      return program
  else:
    for path in os.environ["PATH"].split(os.pathsep):
      exe_file = os.path.join(path, program)
      if is_exe(exe_file):
        return exe_file
  return None

def _verifyARGs(args):
  option_count = 0
  if args.track:
    option_count += 1
  if args.read:
    option_count += 1
  if args.run:
    option_count += 1
  if args.export:
    option_count += 1
  if args.plot:
    option_count += 1
  if option_count != 1 and args.pbs != True:
    if args.call_back_host == None:
      print 'You must use one of the following: track, read, run, or plot'
      sys.exit(1)
  if args.pstack:
    if which('pstack') == None and args.read == None and args.plot == None:
      print '\npstack binary not found. Add it to your PATH and try again.'
      sys.exit(1)
  if args.run:
    if args.run[0].split(' ')[0] == 'mpiexec' or args.run[0].split(' ')[0] == 'mpirun':
      args.mpi = True
    else:
      args.mpi = False
    if args.mpi is not True and args.pbs:
      print 'Ummm, you are specifying PBS with out using mpiexec/mpirun.'
      sys.exit(0)
  args.my_hostname = socket.gethostname()
  return args

def parseArguments(args=None):
  parser = argparse.ArgumentParser(description='Track and Display memory usage')

  rungroup = parser.add_argument_group('Tracking', 'The following options control how the memory logger tracks memory usage')
  rungroup.add_argument('--run', nargs=1, metavar='command', help='Run specified command. You must encapsulate the command in quotes\n ')
  rungroup.add_argument('--pbs', dest='pbs', metavar='', action='store_const', const=True, default=False, help='Instruct memory logger to tally all launches on all nodes\n ')
  rungroup.add_argument('--pbs-delay', dest='pbs_delay', metavar='float', nargs=1, type=float, default=[1.0], help='For larger jobs, you may need to increase the delay as to when the memory_logger will launch the tracking agents\n ')
  rungroup.add_argument('--track', nargs=1, metavar='int', type=int, help='Track a single specific PID already running\n ')
  rungroup.add_argument('--repeat-rate', nargs=1,  metavar='float', type=float, default=[0.25], help='Indicate the sleep delay in float seconds to check memory usage (default 0.25 seconds)\n ')
  rungroup.add_argument('--outfile', nargs=1, metavar='file', default=[os.getcwd() + '/usage.log'], help='Save log to specified file. (Defaults to usage.log)\n ')

  readgroup = parser.add_argument_group('Read / Display', 'Options to manipulate or read log files created by the memory_logger')
  readgroup.add_argument('--read', nargs=1, metavar='file', help='Read a specified memory log file to stdout\n ')
  readgroup.add_argument('--plot', nargs="+", metavar='file', help='Display a graphical representation of memory usage (Requires Matplotlib). Specify a single file or a list of files to plot\n ')
  readgroup.add_argument('--separate', dest='separate', action='store_const', const=True, default=False, help='Display individual node memory usage\n ')
  readgroup.add_argument('--export', nargs=1, metavar='file', help='Export specified log file to a comma delimited format\n ')

  commongroup = parser.add_argument_group('Common Options', 'The following options can be used when tracking or displaying the results')
  commongroup.add_argument('--pstack', dest='pstack', action='store_const', const=True, default=False, help='Save or Display stack trace information\n ')
  commongroup.add_argument('--stdout', dest='stdout', action='store_const', const=True, default=False, help='Display stdout information (memory logger always saves stdout)\n ')

  plotgroup = parser.add_argument_group('Plot Options', 'Additional options when using --plot')
  plotgroup.add_argument('--rotate-text', nargs=1, metavar='int', type=int, default=[30], help='Rotate stdout/pstack text by this ammount (default 30)\n ')
  plotgroup.add_argument('--move-text', nargs=2, metavar='int', default=['0', '0'], help='Move text X and Y by this ammount (default 0 0)\n ')
  plotgroup.add_argument('--trim-text', nargs=1, metavar='int', type=int, default=[15], help='Display this many characters in stdout/pstack (default 15)\n ')
  plotgroup.add_argument('--no-color', dest='no_color', metavar='', action='store_const', const=False, help='When printing output to stdout do not use color codes\n ')

  internalgroup = parser.add_argument_group('Internal PBS Options', 'The following options are used to control how memory_logger as a tracking agent connects back to the caller. These are set automatically when using PBS and can be ignored.')
  internalgroup.add_argument('--call-back-host', nargs=2, help='Server hostname and port that launched memory_logger\n ')

  return _verifyARGs(parser.parse_args(args))

if __name__ == '__main__':
  args = parseArguments()
  if args.read:
    ReadLog(args)
    sys.exit(0)
  if args.export:
    ExportMemoryUsage(args)
    sys.exit(0)
  if args.plot:
    MemoryPlotter(args)
    sys.exit(0)
  trackMemory(args)
