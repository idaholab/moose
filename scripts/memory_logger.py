#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from tempfile import TemporaryFile, SpooledTemporaryFile
import os, sys, re, socket, time, pickle, csv, uuid, subprocess, argparse, decimal, select, platform, signal


class Debugger:
    """
  The Debugger class is the entry point to our stack tracing capabilities.
  It determins which debugger to inherit based on parsed arguments and
  platform specs.
    """
    def __init__(self, arguments):
        if arguments.debugger == 'lldb':
            self.debugger = lldbAPI(arguments)
        else:
            self.debugger = DebugInterpreter(arguments)

    def getProcess(self, pid):
        return self.debugger.getProcess(pid)

    def getStackTrace(self, getProcess_tuple):
        return self.debugger.getStackTrace(getProcess_tuple)


class lldbAPI:
    def __init__(self, arguments):
        self.debugger = lldb.SBDebugger.Create()
        self.debugger.SetAsync(True)

    def __del__(self):
        lldb.SBDebugger.Destroy(self.debugger)

    def getProcess(self, pid):
        # Create and attach to the pid and return our debugger as a tuple
        target = self.debugger.CreateTargetWithFileAndArch(None, None)
        return target, pid

    def getStackTrace(self, process_tuple):
        target, pid = process_tuple
        lldb_results = []

        # reuse the process object if available
        if target.process.id is not 0:
            process = target.Attach(lldb.SBAttachInfo(target.process.id), lldb.SBError())
        else:
            process = target.Attach(lldb.SBAttachInfo(int(pid)), lldb.SBError())

        # test if we succeeded at attaching to PID process
        if process:
            # grab thread information
            lldb_results.append(process.GetThreadAtIndex(0).__str__())
            # iterate through all frames and collect back trace information
            for i in xrange(process.GetThreadAtIndex(0).GetNumFrames()):
                lldb_results.append(process.GetThreadAtIndex(0).GetFrameAtIndex(i).__str__())

            # Unfortunately we must detach each time we perform a stack
            # trace. This severely limits our sample rate. It _appears_ to
            # to be a bug in LLDB's Python API. Otherwise we would be able to:
            #
            # process.Stop()
            # ..collect back trace..
            # process.Continue()
            #
            # instead we have to:
            process.Detach()
            return '\n'.join(lldb_results)
        else:
            return ''

class DebugInterpreter:
    """
    Currently, interfacing with LLDB via subprocess is impossible. This is due to lldb not printing
    to stdout, or stderr when displaying the prompt to the user (informing the user, the debugger
    is ready to receive input). However, this class may someday be able to, which is why
    the self.debugger variable is present.
    """
    def __init__(self, arguments):
        self.last_position = 0
        self.debugger = arguments.debugger

    def _parseStackTrace(self, gibberish):
        not_gibberish = re.findall(r'\(' + self.debugger + '\) (#.*)\(' + self.debugger + '\)', gibberish, re.DOTALL)
        if len(not_gibberish) != 0:
            return not_gibberish[0]
        else:
            # Return a blank line, as to not pollute the log. Gibberish here
            # usually indicates a bunch of warnings or information about
            # loading symbols
            return ''

    def _waitForResponse(self, dbg_stdout):
        # Allow a maximum of 5 seconds to obtain a debugger prompt position.
        # Otherwise we can hang indefinitely
        end_queue = time.time() + float(5)
        while time.time() < end_queue:
            dbg_stdout.seek(self.last_position)
            for line in dbg_stdout:
                if line == '(' + self.debugger + ') ':
                    self.last_position = dbg_stdout.tell()
                    return True
            time.sleep(0.01)
        return False

    def getProcess(self, pid):
        # Create a temporary file the debugger can write stdout/err to
        dbg_stdout = SpooledTemporaryFile()
        # Create and attach to running proccess
        process = subprocess.Popen([which(self.debugger)], stdin=subprocess.PIPE, stdout=dbg_stdout, stderr=dbg_stdout)
        for command in [ 'attach ' + pid + '\n' ]:
            if self._waitForResponse(dbg_stdout):
                try:
                    process.stdin.write(command)
                except:
                    return (False, self.debugger, 'quit unexpectedly')
            else:
                return (False, 'could not attach to process in allotted time')
        return (process, dbg_stdout)

    def getStackTrace(self, process_tuple):
        process, dbg_stdout = process_tuple
        # Store our current file position so we can return to it and read
        # the eventual entire stack trace output
        batch_position = dbg_stdout.tell()
        # Loop through commands necessary to create a back trace
        for command in ['ctrl-c', 'bt\n', 'c\n']:
            if command == 'ctrl-c':
                process.send_signal(signal.SIGINT)
            else:
                if self._waitForResponse(dbg_stdout):
                    process.stdin.write(command)
                else:
                    dbg_stdout.seek(batch_position)
                    return self.detachProcess(process_tuple)
        # Return to previous file position so that we can return the entire
        # stack trace
        dbg_stdout.seek(batch_position)
        return self._parseStackTrace(dbg_stdout.read())

    def detachProcess(self, process):
        process, dbg_stdout = process
        # Offset the position due to ctrl-c not generating a newline event
        tmp_position = (dbg_stdout.tell() - 1)
        for command in ['ctrl-c', 'quit\n', 'y\n']:
            if command == 'ctrl-c':
                process.send_signal(signal.SIGINT)
            else:
                # When these two variables are not equal, its a safe assumption the
                # debugger is ready to receive input
                if tmp_position != dbg_stdout.tell():
                    tmp_position = dbg_stdout.tell()
                    try:
                        process.stdin.write(command)
                    except:
                        # Because we are trying to detach and quit the debugger just pass
                        pass
        # Always return True for a detach call. What would we do if it failed anyway?
        # Why am I even leaving a comment about this?
        return True

class Server:
    def __init__(self, arguments):
        self.arguments = arguments
        self.arguments.cwd = os.getcwd()
        # Test to see if we are starting as a server
        if self.arguments.pbs == True:
            if os.getenv('PBS_NODEFILE') != None:
                # Initialize an agent, strictly for holding our stdout logs. Give it the UUID of 'server'
                self.agent = Agent(self.arguments, 'server')
                if self.arguments.recover:
                    self.logfile = WriteCSV(self.arguments.outfile[0], False)
                else:
                    self.logfile = WriteCSV(self.arguments.outfile[0], True)
                self.client_connections = []
                self.startServer()
            else:
                print 'I could not find your PBS_NODEFILE. Is PBS loaded?'
                sys.exit(1)
        # If we are not a server, start the single client
        else:
            self.startClient()

    def startServer(self):
        # Setup the TCP socket
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.bind((socket.gethostname(), 0))
        self.server_socket.listen(5)
        (self.host, self.port) = self.server_socket.getsockname()

        # We will store all connections (sockets objects) made to the server in a list
        self.client_connections.append(self.server_socket)

        # Launch the actual binary we want to track
        self._launchJob()

        # Now launch all pbs agents
        self._launchClients()

        # This is a try so we can handle a keyboard ctrl-c
        try:

            # Continue to listen and accept active connections from agents
            # until all agents report a STOP command.
            AGENTS_ACTIVE = True
            while AGENTS_ACTIVE:
                read_sockets, write_sockets, error_sockets = select.select(self.client_connections,[],[])
                for sock in read_sockets:
                    if sock == self.server_socket:
                        # Accept an incomming connection
                        self.client_connections.append(self.server_socket.accept()[0])
                    else:
                        # Deal with the data being sent to the server by its agents
                        self.handleAgent()

                        # Check to see if _all_ agents are telling the server to stop
                        agent_count = len(self.agent.agent_data.keys())
                        current_count = 0
                        for agent in self.agent.agent_data.keys():
                            if self.agent.agent_data[agent]['STOP']:
                                current_count += 1

                        # if All Agents have reported a STOP command, begin to exit
                        if current_count == agent_count:
                            AGENTS_ACTIVE = False
                            # Gotta get out of the for loop somehow...
                            break

                    # Sleep a bit before reading additional data
                    time.sleep(self.arguments.repeat_rate[-1])

            # Close the server socket
            self.server_socket.close()

            # Close the logfile as the server is about to exit
            self.logfile.close()

        # Cancel server operations if ctrl-c was pressed
        except KeyboardInterrupt:
            print 'Canceled by user. Wrote log:', self.arguments.outfile[0]
            sys.exit(0)

        # Normal exiting procedures
        print '\n\nAll agents have stopped. Log file saved to:', self.arguments.outfile[0]
        sys.exit(0)

    def startClient(self):
        Client(self.arguments)

    def _launchClients(self):
        # Read the environment PBS_NODEFILE
        self._PBS_NODEFILE = open(os.getenv('PBS_NODEFILE'), 'r')
        nodes = set(self._PBS_NODEFILE.read().split())

        # Print some useful information about our setup
        print 'Memory Logger running on Host:', self.host, 'Port:', self.port, \
          '\nNodes:', ', '.join(nodes), \
          '\nSample rate (including stdout):', self.arguments.repeat_rate[-1], 's (use --repeat-rate to adjust)', \
          '\nRemote agents delaying', self.arguments.pbs_delay[-1], 'second/s before tracking. (use --pbs-delay to adjust)\n'

        # Build our command list based on the PBS_NODEFILE
        command = []
        for node in nodes:
            command.append([  'ssh', node,
                         'bash --login -c "source /etc/profile && ' \
                         + 'sleep ' + str(self.arguments.pbs_delay[-1]) + ' && ' \
                         + os.path.abspath(__file__) \
                         + ' --call-back-host ' \
                         + self.host + ' ' + str(self.port) \
                         + '"'])

        # remote into each node and execute another copy of memory_logger.py
        # with a call back argument to recieve further instructions
        for pbs_node in command:
            subprocess.Popen(pbs_node, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # Launch the binary we intend to track
    def _launchJob(self):
        subprocess.Popen(self.arguments.run[-1].split(), stdout=self.agent.log, stderr=self.agent.log)

    # A connection has been made from client to server
    # Capture that data, and determin what to do with it
    def handleAgent(self):
        # Loop through all client connections, and receive data if any
        for agent_socket in self.client_connections:

            # Completely ignore the server_socket object
            if agent_socket == self.server_socket:
                continue

            # Assign an AgentConnector for the task of handling data between client and server
            reporting_agent = AgentConnector(self.arguments, agent_socket)

            # OK... get data from a client and begin
            new_data = reporting_agent.readData()
            if new_data != None:
                # There should be only one dictionary key (were reading data from just one client at a time)
                agent_uuid = new_data.keys()[0]

                # Update our dictionary of an agents data
                self.agent.agent_data[agent_uuid] = new_data[agent_uuid]

                # Modify incoming Agents timestamp to match Server's time (because every node is a little bit off)
                if self.arguments.recover:
                    self.agent.agent_data[agent_uuid]['TIMESTAMP'] = GetTime().now - self.agent.delta
                else:
                    self.agent.agent_data[agent_uuid]['TIMESTAMP'] = GetTime().now

                # update total usage for all known reporting agents
                total_usage = 0
                for one_agent in self.agent.agent_data.keys():
                    total_usage += self.agent.agent_data[one_agent]['MEMORY']
                self.agent.agent_data[agent_uuid]['TOTAL'] = int(total_usage)

                # Get any stdout thats happened thus far and apply it to what ever agent just sent us data
                self.agent.agent_data[agent_uuid]['STDOUT'] = self.agent._getStdout()

                # Write to our logfile
                self.logfile.write(self.agent.agent_data[agent_uuid])

                # Check for any agents sending a stop command. If we find one,
                # set some zeroing values, and close that agent's socket.
                if self.agent.agent_data[agent_uuid]['STOP']:
                    self.agent.agent_data[agent_uuid]['MEMORY'] = 0
                    agent_socket.close()
                    if agent_socket != self.server_socket:
                        self.client_connections.remove(agent_socket)

                    # Go ahead and set our server agent to STOP as well.
                    # The server will continue recording samples from agents
                    self.agent.agent_data['server']['STOP'] = True

                # If an Agent has made a request for instructions, handle it here
                update_client = False
                if new_data[agent_uuid]['REQUEST'] != None:
                    for request in new_data[agent_uuid]['REQUEST'].iteritems():
                        if new_data[agent_uuid]['REQUEST'][request[0]] == '':
                            update_client = True
                            # We only support sending any arguments supplied to ther server, back to the agent
                            for request_type in dir(self.arguments):
                                if request[0] == str(request_type):
                                    self.agent.agent_data[agent_uuid]['REQUEST'][request[0]] = getattr(self.arguments, request[0])

                    # If an Agent needed additional instructions, go ahead and re-send those instructions
                    if update_client:
                        reporting_agent.sendData(self.agent.agent_data[agent_uuid])

class Client:
    def __init__(self, arguments):
        self.arguments = arguments

        # Initialize an Agent with a UUID based on our hostname
        self.my_agent = Agent(arguments, str(uuid.uuid3(uuid.NAMESPACE_DNS, socket.gethostname())))

        # Initialize an AgentConnector
        self.remote_server = AgentConnector(self.arguments)

        # If client will talk to a server (PBS)
        if self.arguments.call_back_host:
            # We know by initializing an agent, agent_data contains the necessary message asking for further instructions
            self.my_agent.agent_data[self.my_agent.my_uuid] = self.remote_server.sendData(self.my_agent.agent_data)

            # Apply new instructions received from server (this basically updates our arguments)
            for request in self.my_agent.agent_data[self.my_agent.my_uuid]['REQUEST'].iteritems():
                for request_type in dir(self.arguments):
                    if request[0] == str(request_type):
                        setattr(self.arguments, request[0], request[1])

            # Requests have been satisfied, set to None
            self.my_agent.agent_data[self.my_agent.my_uuid]['REQUEST'] = None

            # Change to the same directory as the server was when initiated (needed for PBS stuff)
            os.chdir(self.arguments.cwd)

        # Client will not be talking to a server, save data to a file instead
        else:
            # Deal with --recover
            if self.arguments.recover:
                # Do not overwrite the file
                self.logfile = WriteCSV(self.arguments.outfile[0], False)
            else:
                # Overwrite the file
                self.logfile = WriteCSV(self.arguments.outfile[0], True)

        # Lets begin!
        self.startProcess()

    # This function handles the starting and stoping of the sampler process.
    # We loop until an agent returns a stop command.
    def startProcess(self):
        AGENTS_ACTIVE = True

        # If we know we are the only client, go ahead and start the process we want to track.
        if self.arguments.call_back_host == None:
            subprocess.Popen(self.arguments.run[-1].split(), stdout=self.my_agent.log, stderr=self.my_agent.log)
            # Delay just a bit to keep from recording a possible zero memory usage as the binary starts up
            time.sleep(self.arguments.sample_delay[0])


        # This is a try so we can handle a keyboard ctrl-c
        try:

            # Continue to process data until an Agent reports a STOP command
            while AGENTS_ACTIVE:
                # Take a sample
                current_data = self.my_agent.takeSample()

                # Handle the data supplied by the Agent.
                self._handleData(current_data)

                # If an Agent reported a STOP command, go ahead and begin the shutdown phase
                if current_data[current_data.keys()[0]]['STOP']:
                    AGENTS_ACTIVE = False

                # Sleep just a bit between samples, as to not saturate the machine
                time.sleep(self.arguments.repeat_rate[-1])

            # An agent reported a stop command... so let everyone know where the log was saved, and exit!
            if self.arguments.call_back_host == None:
                print 'Binary has exited and a log file has been written. You can now attempt to view this file by running' \
                  '\nthe memory_logger with either the --plot or --read arguments:\n\n', sys.argv[0], '--plot', self.arguments.outfile[0], \
                  '\n\nSee --help for additional viewing options.'
        # Cancel server operations if ctrl-c was pressed
        except KeyboardInterrupt:
            self.logfile.close()
            print 'Canceled by user. Wrote log:', self.arguments.outfile[0]
            sys.exit(0)

        # Everything went smooth.
        sys.exit(0)

    # Figure out what to do with the sampled data
    def _handleData(self, data):
        # Sending the sampled data to a server
        if self.arguments.call_back_host:
            self.remote_server.sendData(data)
        # Saving the sampled data to a file
        else:
            # Compute the TOTAL memory usage to be how much our one agent reported
            # Because were the only client doing any work
            data[self.my_agent.my_uuid]['TOTAL'] = data[self.my_agent.my_uuid]['MEMORY']
            self.logfile.write(data[self.my_agent.my_uuid])

            # If the agent has been told to stop, close the database file
            if self.my_agent.agent_data[self.my_agent.my_uuid]['STOP'] == True:
                self.logfile.close()

class AgentConnector:
    """
  Functions used to communicate to and from Client and Server.
  Both Client and Server classes use this object.

  readData()
  sendData('message', socket_connection=None)

  if sendData's socket_connection is None, it will create a new connection to the server
  based on supplied arguments
  """
    def __init__(self, arguments, connection=None):
        self.arguments = arguments
        self.connection = connection
        self.CREATED_CONNECTION = False

        # If the connection is None, meaning this object was instanced by a client,
        # we must create a connection to the server first
        if self.connection == None and self.arguments.call_back_host != None:
            self.CREATED_CONNECTION = True
            self.connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.connection.connect((self.arguments.call_back_host[0], int(self.arguments.call_back_host[1])))

    # read all data sent by an agent
    def readData(self):
        # Get how much data there is to receive
        # The first eight bytes is our data length
        data_width = int(self.connection.recv(8))
        tmp_received = ''

        # We need to receive precisely the ammount of data the
        # client is trying to send us.
        while len(tmp_received) < data_width:
            if data_width - len(tmp_received) > 1024:
                tmp_received += self.connection.recv(1024)
            else:
                tmp_received += self.connection.recv(data_width - (len(tmp_received)))

        # unpickle the received message
        return self._unpickleMessage(tmp_received)

    # send data to an agent
    def sendData(self, message):
        # pickle the data up, and send the message
        self.connection.sendall(self._pickleMessage(message))

        # If we had to create the socket (connection was none), and this client/agent is requesting
        # instructions, go ahead and read the data that _better be there_ sent to us by the server.
        if self.CREATED_CONNECTION and message[message.keys()[0]]['REQUEST'] != None:
            return self.readData()

    # The following two functions pickle up the data for easy socket transport
    def _pickleMessage(self, message):
        t = TemporaryFile()
        pickle.dump(message, t)
        t.seek(0)
        str_msg = t.read()
        str_len = len(str_msg)
        message = "%-8d" % (str_len,) + str_msg
        return message

    def _unpickleMessage(self, message):
        t = TemporaryFile()
        t.write(message)
        t.seek(0)
        try:
            return pickle.load(t)
        except KeyError:
            print 'Socket data was not pickled data: ', message
        except:
            raise

class WriteCSV:
    def __init__(self, logfile, overwrite):
        if overwrite:
            self.file_object = open(logfile, 'w', 1)
        else:
            self.file_object = open(logfile, 'a', 1)
        csv.field_size_limit(sys.maxsize)
        self.log_file = csv.writer(self.file_object, delimiter=',', quotechar='|', escapechar='\\', quoting=csv.QUOTE_MINIMAL)

    # Close the logfile
    def close(self):
        self.file_object.close()

    # Write a CSV row
    def write(self, data):
        formatted_string = self._formatString(data)
        self.log_file.writerow(formatted_string)

    # Format the CSV output
    def _formatString(self, data):
        # We will be saving this data in CSV format. Before we do, lets format it a bit here
        format_order = ['TIMESTAMP', 'TOTAL', 'STDOUT', 'STACK', 'HOSTNAME', 'MEMORY']
        formatted_text = []
        for item in format_order:
            # We have to handle python's way of formatting floats to strings specially
            if item == 'TIMESTAMP':
                formatted_text.append('%.6f' % data[item])
            else:
                formatted_text.append(data[item])
        return formatted_text

class Agent:
    """
  Each agent object contains its own sampled log data. The Agent class is responsible for
  collecting and storing data. machine_id is used to identify the agent.

  machine_id is supplied by the client class. This allows for multiple agents if desired
  """
    def __init__(self, arguments, machine_id):
        self.arguments = arguments
        self.my_uuid = machine_id
        self.track_process = ''
        self.process = None

        # This log object is for stdout purposes
        self.log = TemporaryFile()
        self.log_position = 0

        # Discover if --recover is being used. If so, we need to obtain the
        # timestamp of the last entry in the outfile log... a little bulky
        # to do... and not a very good place to do it.
        if self.arguments.recover:
            if os.path.exists(self.arguments.outfile[-1]):
                memory_list = []
                history_file = open(self.arguments.outfile[-1], 'r')
                csv.field_size_limit(sys.maxsize)
                reader = csv.reader(history_file, delimiter=',', quotechar='|', escapechar='\\', quoting=csv.QUOTE_MINIMAL)

                # Get last item in list. Unfortunately, no way to do this until
                # we have read the entire file...? Lucky for us, most memory log
                # files are in the single digit megabytes
                for row in reader:
                    memory_list.append(row)
                history_file.close()
                last_entry = float(memory_list[-1][0]) + self.arguments.repeat_rate[-1]
                self.delta = (GetTime().now - last_entry)
            else:
                print 'Recovery options detected, but I could not find your previous memory log file.'
                sys.exit(1)
        else:
            self.delta = 0

        # Create the dictionary to which all sampled data will be stored
        # NOTE: REQUEST dictionary items are instructions (arguments) we will
        # ask the server to provide (if we are running with --pbs)
        # Simply add them here. We _can not_ make the arguments match the
        # server exactly, this would cause every agent launched to perform
        # like a server... bad stuff

        # Example: We added repeat_rate (see dictionary below). Now every
        # agent would update their repeat_rate according to what the user
        # supplied as an argument (--repeat_rate 0.02)
        self.agent_data = { self.my_uuid :
                            { 'HOSTNAME'  : socket.gethostname(),
                              'STDOUT'    : '',
                              'STACK'     : '',
                              'MEMORY'    : 0,
                              'TIMESTAMP' : GetTime().now - self.delta,
                              'REQUEST'   : { 'run'          : '',
                                              'pstack'       : '',
                                              'repeat_rate'  : '',
                                              'cwd'          : '',
                                              'debugger'     : ''},
                              'STOP'      : False,
                              'TOTAL'     : 0,
                              'DEBUG_LOG' : ''
                            }
                          }

        # we need to create a place holder for our debugger because when
        # memory_logger is run via --pbs, this Agent will not know what
        # kind of debugger to use until it has made contact with the server
        self.stack_trace = None

    # NOTE: This is the only function that should be called in this class
    def takeSample(self):
        if self.arguments.pstack:
            if self.stack_trace is None:
                self.stack_trace = Debugger(self.arguments)
            self.agent_data[self.my_uuid]['STACK'] = self._getStack()

        # Always do the following
        self.agent_data[self.my_uuid]['MEMORY'] = self._getMemory()
        self.agent_data[self.my_uuid]['STDOUT'] = self._getStdout()
        if self.arguments.recover:
            self.agent_data[self.my_uuid]['TIMESTAMP'] = GetTime().now - self.delta
        else:
            self.agent_data[self.my_uuid]['TIMESTAMP'] = GetTime().now

        # Return the data to whom ever asked for it
        return self.agent_data

    def _getStdout(self):
        self.log.seek(self.log_position)
        output = self.log.read()
        self.log_position = self.log.tell()
        sys.stdout.write(output)
        return output

    def _getMemory(self):
        tmp_pids = self._getPIDs()
        memory_usage = 0
        if tmp_pids != {}:
            for single_pid in tmp_pids.iteritems():
                memory_usage += int(single_pid[1][0])
            if memory_usage == 0:
                # Memory usage hit zero? Then assume the binary being tracked has exited. So lets begin doing the same.
                self.agent_data[self.my_uuid]['DEBUG_LOG'] = 'I found the total memory usage of all my processes hit 0. Stopping'
                self.agent_data[self.my_uuid]['STOP'] = True
                return 0
            return int(memory_usage)
        # No binay even detected? Lets assume it exited, so we should begin doing the same.
        self.agent_data[self.my_uuid]['STOP'] = True
        self.agent_data[self.my_uuid]['DEBUG_LOG'] = 'I found no processes running. Stopping'
        return 0

    def _getStack(self):
        # Create a process object if none already exists. Reuse the old one if it does.
        if self.process is None:
            tmp_pids = self._getPIDs()
            # Check if we actually found any running processes
            if tmp_pids != {}:
                # Obtain a single process id, any process id will do. This will be the process we attach to and perform stack traces
                one_pid = tmp_pids.keys()[0]
                self.process = self.stack_trace.getProcess(str(one_pid))
                return self.stack_trace.getStackTrace(self.process)
            else:
                return ''
        else:
            return self.stack_trace.getStackTrace(self.process)

    def _getPIDs(self):
        pid_list = {}

        # Determin the binary to sample and store it. Doing the findCommand is a little expensive.
        if self.track_process == '':
            self.track_process = self._findCommand(''.join(self.arguments.run))

        # If we are tracking a binary
        if self.arguments.run:
            command = [which('ps'), '-e', '-o', 'pid,rss,user,args']
            tmp_proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            all_pids = tmp_proc.communicate()[0].split('\n')
            # Figure out what we are allowed to track (strip away mpiexec, processes not owned by us, etc)
            for single_pid in all_pids:
                if single_pid.find(self.track_process) != -1 and \
                   single_pid.find(__file__) == -1 and \
                   single_pid.find('mpirun') == -1 and \
                   single_pid.find(os.getenv('USER')) != -1 and \
                   single_pid.find('mpiexec') == -1:
                    pid_list[int(single_pid.split()[0])] = []
                    pid_list[int(single_pid.split()[0])].extend([single_pid.split()[1], single_pid.split()[3]])
        return pid_list

    # Determine the command we are going to track
    # A few things are happening here; first we strip off any MPI commands
    # we then loop through the remaining items until we find a matching path
    # exp:  mpiexec -n 12 ../../../moose_test-opt -i simple_diffusion.i -r 6
    # would first strip off mpiexec, check for the presence of -n in our
    # current directory, then 12, then ../../../moose_test-opt   <- found. It would
    # stop and return the base name (moose_test-opt).
    def _findCommand(self, command):
        if command.find('mpiexec') == 0 or command.find('mpirun') == 0:
            for binary in command.split():
                if os.path.exists(binary):
                    return os.path.split(binary)[1]
        elif os.path.exists(command.split()[0]):
            return os.path.split(command.split()[0])[1]

class GetTime:
    """A simple formatted time object.
  """
    def __init__(self, posix_time=None):
        import datetime
        if posix_time == None:
            self.posix_time = datetime.datetime.now()
        else:
            self.posix_time = datetime.datetime.fromtimestamp(posix_time)
        self.now = float(datetime.datetime.now().strftime('%s.%f'))
        self.microsecond = self.posix_time.microsecond
        self.second = self.posix_time.second
        self.minute = self.posix_time.strftime('%M')
        self.hour = self.posix_time.strftime('%H')
        self.day = self.posix_time.strftime('%d')
        self.month = self.posix_time.strftime('%m')
        self.year = self.posix_time.year
        self.dayname = self.posix_time.strftime('%a')
        self.monthname = self.posix_time.strftime('%b')

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
                csv.field_size_limit(sys.maxsize)
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
        self.stdout_msgs = {}
        self.pstack_msgs = {}
        self.multiples = 1
        self.memory_label = 'Memory in Bytes'

        # Try and calculate memory sizes, so we can move annotations around a bit more accurately
        largest_memory = []
        for plot_name, value_list in plot_dictionary.iteritems():
            for records in value_list:
                largest_memory.append(int(records[1]))
        largest_memory.sort()

        # Determine the scale of the graph
        suffixes = ["Terabytes", "Gigabytes", "Megabytes", "Kilobytes", "Bytes"]
        multiplier = 1 << 40;
        index = 0
        while largest_memory[-1] < multiplier and multiplier > 1:
            multiplier = multiplier >> 10
            index = index + 1
        self.multiples = multiplier
        self.memory_label = "Memory in " + suffixes[index-1]

        # Loop through each log file
        for plot_name, value_list in plot_dictionary.iteritems():
            plot_list.append(fig.add_subplot(111))
            tmp_memory = []
            tmp_time = []

            tmp_stdout_x = []
            tmp_stdout_y = []

            tmp_pstack_x = []
            tmp_pstack_y = []

            stdout_msg = []
            pstack_msg = []

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
                    stdout_msg.append(records[2])

                if len(records[3]) > 0 and self.arguments.pstack:
                    tmp_pstack_x.append(tmp_time[-1])
                    tmp_pstack_y.append(tmp_memory[-1])
                    pstack_msg.append(records[3])

            # Do the actual plotting:
            f, = plot_list[-1].plot(tmp_time, tmp_memory)
            tmp_plot.append(f)
            tmp_legend.append(plot_name)
            plot_list[-1].grid(True)
            plot_list[-1].set_ylabel(self.memory_label)
            plot_list[-1].set_xlabel('Time in Seconds')

            # Enable dork mode
            if self.arguments.darkmode:
                fig.set_facecolor('0.1')
                plot_list[-1].set_axis_bgcolor('0.1')
                plot_list[-1].spines['bottom'].set_color('white')
                plot_list[-1].spines['top'].set_color('white')
                plot_list[-1].spines['right'].set_color('white')
                plot_list[-1].spines['left'].set_color('white')
                plot_list[-1].tick_params(axis='x', colors='white')
                plot_list[-1].tick_params(axis='y', colors='white')
                plot_list[-1].xaxis.label.set_color('white')
                plot_list[-1].yaxis.label.set_color('white')
                plot_list[-1].grid(color='0.6')

            # Plot annotations
            if self.arguments.stdout:
                stdout_line, = plot_list[-1].plot(tmp_stdout_x, tmp_stdout_y, 'x', picker=10, color=f.get_color(), markeredgecolor='0.08', markeredgewidth=0.1)
                next_index = str(len(plot_list))
                stdout_line.set_gid('stdout' + next_index)
                self.stdout_msgs[next_index] = stdout_msg
                self.buildAnnotation(plot_list[-1], tmp_stdout_x, tmp_stdout_y, stdout_msg, f.get_color())

            if self.arguments.pstack:
                pstack_line, = plot_list[-1].plot(tmp_pstack_x, tmp_pstack_y, 'o', picker=10, color=f.get_color(), markeredgecolor='0.08', markeredgewidth=0.1)
                next_index = str(len(plot_list))
                pstack_line.set_gid('pstack' + next_index)
                self.pstack_msgs[next_index] = pstack_msg

        # Make points clickable
        fig.canvas.mpl_connect('pick_event', self)

        # Create legend
        legend = plt.legend(tmp_plot, tmp_legend, loc = self.arguments.legend)
        legend.get_frame().set_alpha(0.7)

        # More dork mode settings
        if self.arguments.darkmode:
            legend.get_frame().set_facecolor('0.2')
            for text in legend.get_texts():
                text.set_color('0.8')

        plt.show()

    def __call__(self, event):
        color_codes = {'RESET':'\033[0m', 'r':'\033[31m','g':'\033[32m','c':'\033[36m','y':'\033[33m', 'b':'\033[34m', 'm':'\033[35m', 'k':'\033[0m', 'w':'\033[0m' }
        line = event.artist
        ind = event.ind

        name = line.get_gid()[:-1]
        index = line.get_gid()[-1]

        if self.arguments.stdout and name == 'stdout':
            if self.arguments.no_color != False:
                print color_codes[line.get_color()]
            print "stdout -----------------------------------------------------\n"
            for id in ind:
                print self.stdout_msgs[index][id]
            if self.arguments.no_color != False:
                print color_codes['RESET']

        if self.arguments.pstack and name == 'pstack':
            if self.arguments.no_color != False:
                print color_codes[line.get_color()]
            print "pstack -----------------------------------------------------\n"
            for id in ind:
                print self.pstack_msgs[index][id]
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

# A simple which function to return path to program
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
    print 'I could not find the following binary:', program
    sys.exit(1)

def verifyArgs(args):
    possible_positions = [ 'center',
                           'center left',
                           'center right',
                           'upper center',
                           'lower center',
                           'best',
                           'right',
                           'left',
                           'upper right',
                           'lower right',
                           'upper left',
                           'lower left']
    if args.legend not in possible_positions:
        print 'Invalid legend position requested. Possible values are:\n\t', '\n\t'.join([x for x in possible_positions])
        sys.exit(1)

    option_count = 0
    if args.read:
        option_count += 1
    if args.run:
        option_count += 1
    if args.plot:
        option_count += 1
    if option_count != 1 and args.pbs != True:
        if args.call_back_host == None:
            print 'You must use one of the following: run, read, or plot'
            sys.exit(1)
    args.cwd = os.getcwd()

    # Work with --recover (a MOOSE application specific option)
    args.recover = False
    if args.run:
        if args.run[0].find('--recover') != -1:
            args.recover = True
        if args.run[0].find('~') != -1:
            print "You must use absolute paths. Python does not understand the '~' path discriptor.\nYou can use environment vairables (eg: $HOME) so long as they are absolute paths."
            sys.exit(1)

    if args.outfile == None and args.run:
        # Attempt to build the output file based on input file
        if re.findall(r'-i (\w+)', args.run[0]) != []:
            args.outfile = [os.getcwd() + '/' + re.findall(r'-i (\w+)', args.run[0])[0] + '_memory.log']
        else:
            args.outfile = [os.getcwd() + '/' + args.run[0].replace('..', '').replace('/', '').replace(' ', '_') + '.log']

    if args.pstack and (args.read is None and args.plot is None):
        if args.debugger is not None:
            if args.debugger == 'lldb':
                if platform.platform().find('Darwin') != -1:
                    try:
                        import lldb
                    except ImportError:
                        lldbImportError()
                        sys.exit(1)
                else:
                    results = which('lldb')
            elif args.debugger == 'gdb':
                results = which('gdb')
        else:
            print 'Invalid debugger selected. You must choose between gdb and lldb using the --debugger argument'
            sys.exit(1)
    return args

def parseArguments(args=None):
    parser = argparse.ArgumentParser(description='Track and Display memory usage')

    rungroup = parser.add_argument_group('Tracking', 'The following options control how the memory logger tracks memory usage')
    rungroup.add_argument('--run', nargs=1, metavar='command', help='Run specified command using absolute paths. You must encapsulate the command in quotes.')
    rungroup.add_argument('--pbs', dest='pbs', metavar='', action='store_const', const=True, default=False, help='Instruct memory logger to tally all launches on all nodes\n ')
    rungroup.add_argument('--pbs-delay', dest='pbs_delay', metavar='float', nargs=1, type=float, default=[1.0], help='For larger jobs, you may need to increase the delay as to when the memory_logger will launch the tracking agents\n ')
    rungroup.add_argument('--sample-delay', dest='sample_delay', metavar='float', nargs=1, type=float, default=[0.25], help='The time to delay before taking the first sample (when not using pbs)')
    rungroup.add_argument('--repeat-rate', nargs=1,  metavar='float', type=float, default=[0.25], help='Indicate the sleep delay in float seconds to check memory usage (default 0.25 seconds)\n ')
    rungroup.add_argument('--outfile', nargs=1, metavar='file', help='Save log to specified file. (Defaults based on run command)\n ')

    readgroup = parser.add_argument_group('Read / Display', 'Options to manipulate or read log files created by the memory_logger')
    readgroup.add_argument('--read', nargs=1, metavar='file', help='Read a specified memory log file to stdout\n ')
    readgroup.add_argument('--separate', dest='separate', action='store_const', const=True, default=False, help='Display individual node memory usage (read mode only)\n ')
    readgroup.add_argument('--plot', nargs="+", metavar='file', help='Display a graphical representation of memory usage (Requires Matplotlib). Specify a single file or a list of files to plot\n ')
    readgroup.add_argument('--legend', metavar='"lower left"', default='lower left', help='Place legend in one of the following locations (default --legend "lower left") "center", "center left", "center right", "upper center", "lower center", "best", "right", "left", "upper right", "lower right", "upper left", "lower left"\n ')

    commongroup = parser.add_argument_group('Common Options', 'The following options can be used when displaying the results')
    commongroup.add_argument('--pstack', dest='pstack', action='store_const', const=True, default=False, help='Display/Record stack trace information (if available)\n ')
    commongroup.add_argument('--stdout', dest='stdout', action='store_const', const=True, default=False, help='Display stdout information\n ')
    commongroup.add_argument('--debugger', dest='debugger', metavar='gdb | lldb', nargs='?', help='Specify the debugger to use. Possible values: gdb or lldb\n ')

    plotgroup = parser.add_argument_group('Plot Options', 'Additional options when using --plot')
    plotgroup.add_argument('--rotate-text', nargs=1, metavar='int', type=int, default=[30], help='Rotate stdout/pstack text by this ammount (default 30)\n ')
    plotgroup.add_argument('--move-text', nargs=2, metavar='int', default=['0', '0'], help='Move text X and Y by this ammount (default 0 0)\n ')
    plotgroup.add_argument('--trim-text', nargs=1, metavar='int', type=int, default=[15], help='Display this many characters in stdout/pstack (default 15)\n ')
    plotgroup.add_argument('--no-color', dest='no_color', metavar='', action='store_const', const=False, help='When printing output to stdout do not use color codes\n ')
    plotgroup.add_argument('--darkmode', dest='darkmode', metavar='', action='store_const', const=True, help='When you want to be cool\n ')

    internalgroup = parser.add_argument_group('Internal PBS Options', 'The following options are used to control how memory_logger as a tracking agent connects back to the caller. These are set automatically when using PBS and can be ignored.')
    internalgroup.add_argument('--call-back-host', nargs=2, help='Server hostname and port that launched memory_logger\n ')

    return verifyArgs(parser.parse_args(args))

def lldbImportError():
    print """
  Unable to import lldb

    The Python lldb API is now supplied by Xcode but not
    automatically set in your PYTHONPATH. Please search
    the internet for how to do this if you wish to use
    --pstack on Mac OS X.

    Note: If you installed Xcode to the default location of
    /Applications, you should only have to perform the following:

  export PYTHONPATH=/Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework/Resources/Python:$PYTHONPATH

    ###!! IMPORTANT !!###
    It may also be necessary to unload the miniconda module.
    If you receive a fatal Python error about PyThreadState
    try using your system's version of Python instead.
  """

if __name__ == '__main__':
    args = parseArguments()
    if args.read:
        ReadLog(args)
        sys.exit(0)
    if args.plot:
        MemoryPlotter(args)
        sys.exit(0)
    Server(args)
