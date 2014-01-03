# Load the required modules
import os, subprocess, re, time, errno, signal

## @class DistccDaemon
# Class for managing the distccd daemons
class DistccDaemon(object):

  ## Class constructor
  # @param master The master Machine object (i.e. this machine)
  # @param kwargs Option key/value pairings
  #
  # Optional Arguments:
  #  dedicated = True | {False} - run in dedicated mode
  def __init__(self, master, **kwargs):

    # Initilize variables
    self._master = master
    self._filename = os.path.join(os.getenv('HOME'), '.dmakerc_cron')
    self._pidfile = os.path.join(os.getenv('HOME'), '.dmake_pid')

    # Setup the cron jobs
    self._manageCron(kwargs.pop('dedicated', False))


  ## Start the daemons (public)
  # @param machines A list of Machine objects to start
  # @param kwargs Optional inputs (see below)
  #
  #  Optional Arguments:
  #    allow = <list of address> - additional list of addresses to allow
  def start(self, machines, **kwargs):

    # Kill any existing processes
    self.kill()

    # List of allowable commands
    allow_commands = ['gcc', 'g++', 'gfortran', 'clang', 'clang++', 'mpicc', 'mpicxx', 'cc']

    # Create the distcc .cmd_list file
    cmd_list = open(os.getenv('HOME') + '/.cmd_list', 'w')
    for cmd in allow_commands:
      if cmd != None:
        cmd_list.write(cmd + '\n')
    cmd_list.close()

    # Set the environment variable for the allowed commands
    os.environ['DISTCC_CMDLIST'] = os.getenv('HOME') + '/.cmd_list'

    # Build the distccd command
    distccd_cmd = ['distccd', '--daemon', '-P', self._pidfile]
    for machine in machines:
      distccd_cmd.extend(['--allow', machine.address])

    # Append the optional allow list
    allow = kwargs.pop('allow', None)
    if allow != None:
      for item in allow:
        distccd_cmd.extend(['--allow', item])

    # Set the DISTCC backoff period to something large, so if server fails it will not try again
    os.environ['DISTCC_BACKOFF_PERIOD'] = '6000'

    # Run the daemon
    sub = subprocess.Popen(distccd_cmd)
    sub.wait()


  ## Stops all running distccd processes (public)
  # @see start
  def kill(self):
    if os.path.exists(self._pidfile):
      f = open(self._pidfile)
      pid = int(f.read())
      os.kill(pid, signal.SIGTERM)   # Tell the parent distccd process to terminate

      # Make sure the process is terminated
      try:
        tries = 10
        while tries > 0:
          time.sleep(0.2)
          os.kill(pid, 0)     # Get process status

        print "Unable to terminate distccd process through SIGTERM, using SIGKILL"
        os.kill(pid, signal.SIGKILL)

      except OSError as error:
        if error.errno == errno.ESRCH:
          pass
        else:
          print "Unknown error while obtaining distccd status"

  ## A function to facilitate cronjobs with dedicated mode (private)
  # @param dedicated True if running in dedicated mode
  def _manageCron(self, dedicated):

    # Extract existing crontab file
    cron_proc = subprocess.Popen(['crontab', '-l'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    current_cron = cron_proc.communicate()[0]

    # Modify crontab to run dmake in dedicated mode, refreshing the list every 30 minutes
    if dedicated:
      if current_cron.find('dmake') == -1:
        self._master.description = self._master.description +  ' (Dedicated Mode)'
        current_cron += '*/30 * * * * if [ -f ~/.bash_profile ]; then source ~/.bash_profile >/dev/null 2>&1; elif [ -f ~/.bashrc ]; then source ~/.bashrc >/dev/null 2>&1; fi && dmake --dedicated --daemon --description "' + self._master.description + '" >/dev/null 2>&1\n'
        self._updateCron(current_cron)
    else:

      # Remove dmake from the users cron, if detected
      if current_cron.find('dmake --dedicated') != -1:
        cron_line = re.findall(r'.*dmake --dedicated.*\n', current_cron)[0]
        current_cron = current_cron.replace(cron_line, '')
        self._updateCron(current_cron)


  ## Update the cron file (private)
  def _updateCron(self, cron_lines):
    cron_file = open(self._filename, 'w')
    cron_file.write(cron_lines)
    subprocess.Popen(['crontab', self._filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    cron_file.close()
