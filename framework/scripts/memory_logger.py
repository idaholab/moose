#!/usr/bin/env python
import argparse, re, os, sys, subprocess, time, decimal, datetime
from tempfile import TemporaryFile

class trackPID():
  def __init__(self, process):
    self.args = process
    self.pid = process.track

  def GetMemory(self):
    try:
      tmp_proc = subprocess.Popen(['/bin/ps', '-p', str(self.args.track), '-o', 'rss='], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      return { 'TOTAL' : int(re.findall(r'(\d+)', str(tmp_proc.communicate()[0]))[0]) }
    except:
      return

class sglPID():
  def __init__(self, process):
    self.args = process
    self.process = subprocess.Popen(''.join(self.args.run).split())
    self.pid = self.process.pid

  def GetMemory(self):
    try:
      tmp_proc = subprocess.Popen(['/bin/ps', '-p', str(self.pid), '-o', 'rss='], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      return { 'TOTAL' : int(re.findall(r'(\d+)', str(tmp_proc.communicate()[0]))[0]) }
    except:
      return

class mpiPID():
  def __init__(self, process):
    self.args = process
    self.process = subprocess.Popen(''.join(self.args.run).split())
    self.pid = self.process.pid

  def _discover_name(self):
    locations = ''.join(self.args.run).split()
    for item in locations:
      if os.path.exists(item):
        return item

  def GetMemory(self):
    command = ['/bin/ps', '-u', str(os.getenv('USER')), '-eo', 'pid,comm']
    pid_dict = {}
    tmp_proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    pid_list = tmp_proc.communicate()[0].split('\n')
    for item in pid_list:
      tmp_suffix = item.split('/').pop()
      tmp_item = re.findall(r'(\d+).*' + str(self._discover_name().split('/').pop()), item)
      if len(tmp_item) > 0:
        tmp_proc = subprocess.Popen(['/bin/ps', '-p', str(tmp_item[0]), '-o', 'rss='], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        pid_dict[tmp_item[0]] = int(re.findall(r'(\d+)', str(tmp_proc.communicate()[0]))[0])
    tmp_mem = 0
    for keyitem in pid_dict:
      tmp_mem += pid_dict[keyitem]
    pid_dict['TOTAL'] = tmp_mem
    return pid_dict

class ReadMemoryLog():
  def __init__(self, args):
    history_file = open(args.read, 'r')
    self.memory_list = history_file.read().split('\n')
    self.memory_list.pop()
    history_file.close()
    self.sorted_list = []
    self.mem_list = []
    self.printHistory()

  def printHistory(self):
    RESET  = '\033[0m'
    BOLD   = '\033[1m'
    RED    = '\033[31m'
    GREEN  = '\033[32m'
    CYAN   = '\033[36m'
    YELLOW = '\033[33m'
    last_memory = 0.0
    for timestamp in self.memory_list:
      to = GetTime(eval(timestamp)[0])
      self.mem_list.append(eval(timestamp)[1])
      self.sorted_list.append([str(to.day) + ' ' + str(to.monthname) + ' ' + str(to.hour) + ':' + str(to.minute) + ':' + str(to.second) + '.' + '{:06.0f}'.format(to.microsecond), eval(timestamp)[1]])
    largest_memory = decimal.Decimal(max(self.mem_list))
    percentage_length = decimal.Decimal(self.getTerminalSize()[0]) - decimal.Decimal(len(str(self.sorted_list[0][0]) + ' using: ' + '{:20,.0f}'.format(self.sorted_list[0][1]) + 'K |'))
    print 'Date Stamp' + ' '*int(24) + 'Memory Usage | Percent of MAX memory used: ( ' + str('{:0,.0f}'.format(largest_memory)) + ' K )'
    for item in self.sorted_list:
      if decimal.Decimal(item[1]) == largest_memory:
        percent = '100'
      elif (decimal.Decimal(item[1]) / largest_memory) ==  0:
        percent = '0'
      else:
        percent = str(decimal.Decimal(item[1]) / largest_memory)[2:4] + '.' + str(decimal.Decimal(item[1]) / largest_memory)[4:6]
      if decimal.Decimal(item[1]) == largest_memory:
        tmp_str = item[0] + '{:20,.0f}'.format(item[1]) + ' K |' + '-'*int(percentage_length * (decimal.Decimal(item[1]) / largest_memory)) + RESET + '-| ' + percent + '%'
      elif item[1] > last_memory:
        tmp_str = item[0] + '{:20,.0f}'.format(item[1]) + ' K |' + RED + '-'*int(percentage_length * (decimal.Decimal(item[1]) / largest_memory)) + RESET + '| ' + percent + '%'
      else:
        tmp_str = item[0] + '{:20,.0f}'.format(item[1]) + ' K |' + GREEN + '-'*int(percentage_length * (decimal.Decimal(item[1]) / largest_memory)) + RESET + '| ' + percent + '%'
      last_memory = item[1]
      print tmp_str
    print 'Date Stamp' + ' '*int(24) + 'Memory Usage | Percent of MAX memory used: ( ' + str('{:0,.0f}'.format(largest_memory)) + ' K )'

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

class GetTime():
  def __init__(self, posix_time=None):
    if posix_time == None:
      self.posix_time = datetime.datetime.now()
    else:
      self.posix_time = datetime.datetime.fromtimestamp(posix_time)
    self.now = float(datetime.datetime.now().strftime('%s.%f'))
    self.microsecond = self.posix_time.microsecond
    self.second = self.posix_time.second
    self.minute = self.posix_time.minute
    self.hour = self.posix_time.hour
    self.day = self.posix_time.day
    self.month = self.posix_time.month
    self.year = self.posix_time.year
    self.dayname = self.posix_time.strftime('%a')
    self.monthname = self.posix_time.strftime('%b')

class ExportMemoryUsage():
  def __init__(self, args):
    history_file = open(args.export, 'r')
    self.memory_list = history_file.read().split('\n')
    self.memory_list.pop()
    history_file.close()
    self.sorted_list = []
    self.mem_list = []
    self.exportFile()

  def exportFile(self):
    output_file = open(args.export + '.comma_delimited', 'w')
    for timestamp in self.memory_list:
      time_object = GetTime(eval(timestamp)[0])
      self.mem_list.append(eval(timestamp)[1])
      self.sorted_list.append([str(time_object.year) + '-' + str(time_object.month) + '-' + str(time_object.day) + ' ' + str(time_object.hour) + ':' + str(time_object.minute) + ':' + str(time_object.second) + '.' + str(time_object.microsecond), eval(timestamp)[1]])
    for item in self.sorted_list:
      output_file.write(str(item[0]) + ',' + str(item[1]) + '\n')
    output_file.close()
    print 'Comma delimited file saved to: ' + os.getcwd() + '/' + str(args.export) + '.comma_delimited'

def writeMemoryUsage(process):
  file_object = open(process.args.outfile, 'w')
  last_memory = 0
  def _usage(last_memory):
    time_object = GetTime()
    current_usage = [time_object.now, int(process.GetMemory()['TOTAL'])]
    if int(current_usage[1]) != int(last_memory):
      last_memory = int(current_usage[1])
      file_object.write(str(current_usage) + '\n')
      file_object.flush()
    return int(last_memory)
  def _zero():
    try:
      time_object = GetTime()
      current_usage = [time_object.now, 0]
      file_object.write(str(current_usage) + '\n')
      file_object.close()
    except:
      pass
  try:
    while True:
      if not os.kill(int(process.pid), 0):
        if process.args.track is None:
          if process.process.poll() != 0:
            last_memory = _usage(last_memory)
          else:
            _zero()
            print '\nApplication terminated. Wrote file:', process.args.outfile
            return
        else:
          last_memory = _usage(last_memory)
      else:
        _zero()
        print '\nApplication terminated. Wrote file:', process.args.outfile
        return
      time.sleep(float(process.args.repeat_rate))
  except KeyboardInterrupt:
    _zero()
    print '\nCanceled by user. Wrote file:', process.args.outfile
    return
  except:
    _zero()
    print '\nApplication terminated. Wrote file:', process.args.outfile
    return

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
  if option_count != 1:
    print 'You must use one of the following: track, read, or run'
    sys.exit(1)
  return args

def parseARGs(args=None):
  parser = argparse.ArgumentParser(description='Track memory usage')
  parser.add_argument('--track', nargs="?", type=int, help='Track a single specific PID already running\n ')
  parser.add_argument('--read', nargs="?", help='Read a specified memory log file\n ')
  parser.add_argument('--export', nargs="?", help='Export specified log file to a comma delimited format\n ')
  parser.add_argument('--run', nargs="+", help='Run specified command. You must encapsulate the command in quotes\n ')
  parser.add_argument('--mpi', dest='mpi', action='store_const', const=True, default=False, help='Instruct memory logger to tally all launches\n ')
  parser.add_argument('--outfile', nargs="?", default=os.getcwd() + '/usage.log' ,help='Save log to specified file. (Defaults to usage.log)\n ')
  parser.add_argument('--repeat-rate', nargs="?", type=float, default=0.25, help='Indicate the sleep delay in float seconds to check memory usage (default 0.25 seconds)\n ')
  return _verifyARGs(parser.parse_args(args))

if __name__ == '__main__':
  args = parseARGs()
  if args.read:
    ReadMemoryLog(args)
    sys.exit(0)
  if args.run:
    if args.mpi:
      results = mpiPID(args)
    else:
      results = sglPID(args)
  if args.track:
    results = trackPID(args)
  if args.export:
    ExportMemoryUsage(args)
    sys.exit(0)

  writeMemoryUsage(results)
