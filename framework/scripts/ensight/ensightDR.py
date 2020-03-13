#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, sys, re, argparse, subprocess, socket, time

pbs_template = """#!/bin/bash
#PBS -N EnsightSOS-tst
#PBS -l select=<SELECT>:ncpus=<PROCS>:mpiprocs=<PROCS>
#PBS -l place=scatter:excl
#PBS -l walltime=<WALLTIME>
#PBS -q <QUEUE>
<OUTPUT>

module load ensight
sleep 10
<COMMAND>
"""

def launchCEI(options):
    cei = which('ceishell30')
    if cei != None:
        cei_command = [ cei,
                       '-app',
                       '-v',
                       '-child', 'listen://\?port=' + str(options.cei_port) + '\&timeout=-1' ]
        if options.debug:
            print 'CEI Local lauch command:', cei_command
        cei_process = subprocess.Popen(cei_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        job_id = launchNodes(options)
        print 'QSUB Job ID:', job_id
        if job_id != None:
            # Neccessary to wait just a bit before attempting to query the PBS system for job status
            time.sleep(2)
            head_node = getHeadnode(options, job_id)
        else:
            print 'Job did not launch! Onoes!'
            cei_process.kill()
            sys.exit(1)
        if head_node != None:
            print 'Head Node:', head_node
            sos_process = launchSOS(options, head_node)
        else:
            print 'Were queued!!! Onoes!!'
            sys.exit(1)
        ensight_process = launchEnsight(options, head_node, job_id, cei_process, sos_process)
    else:
        print 'ceishell30 executable not found. Module loaded?'
        sys.exit(1)

def launchNodes(options):
    # Create QSUB script
    global pbs_template
    for key, value in vars(options).items():
        pbs_template = pbs_template.replace('<' + key.upper() + '>', str(value))
    if options.save_output:
        pbs_template = pbs_template.replace('<OUTPUT>', '#PBS -j oe')
    else:
        pbs_template = pbs_template.replace('<OUTPUT>', '#PBS -o /dev/null\n#PBS -e /dev/null')
    sos_command = ['mpiexec', 'ceishell30', '-v', '-parent connect://$HOSTNAME\?port=' + str(options.sos_port), '-role', 'SOS_SERVERS']
    pbs_template = pbs_template.replace('<COMMAND>', ' '.join(sos_command))

    qsub_script = open('launch_ensightDR.qsub', 'w')
    qsub_script.write(pbs_template)
    qsub_script.close()

    # Launch QSUB script and return the JOB ID
    qsub_command = [ 'ssh',
                     options.cluster,
                     'bash --login -c "source /etc/profile && ' \
                       + 'qsub ' + os.path.dirname(os.path.abspath(__file__)) + '/launch_ensightDR.qsub' \
                       + '"' ]
    if options.debug:
        print 'QSUB launch command:', qsub_command
        print 'QSUB job script contents:\n', pbs_template

    qsub_process = subprocess.Popen(qsub_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return re.findall(r'^(\d+)', qsub_process.communicate()[0], re.M)[0]

def launchSOS(options, head_node):
    sos_command = [ 'ssh', options.cluster, 'ssh', head_node,
                   '"ceishell30',
                   '-parent', 'connect://' + socket.gethostname() + '\?port=' + str(options.cei_port),
                   '-child', 'listen://\?nconnections=' + str(options.select * options.procs) + '\&timeout=-1\&port=' + str(options.sos_port),
                   '-role', 'SOS"' ]
    if options.debug:
        print 'SOS remote lauch command:', sos_command

    return subprocess.Popen(sos_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

def launchEnsight(options, head_node, job_id, cei_process, sos_process):
    ensight_command = ['ensight100', '-sos', '-ceishell']
    ensight_process = subprocess.Popen(ensight_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # Wait for Ensight to be killed
    ensight_output = ensight_process.communicate()

    # Delete the job if not already. This allows for a clean exit of ceishell30
    qdel_job(options, job_id)
    cei_process.terminate()
    os.remove('launch_ensightDR.qsub')

    if options.debug:
        print '\n##########\nEnsight Output:\n', ensight_output[0], '\n##########\nEnsight Error:', ensight_output[1]

def getHeadnode(options, job_id):
    jobid_command = [ 'ssh',
                      options.cluster,
                      'bash --login -c "source /etc/profile && ' \
                        + 'qstat -f ' + str(job_id) \
                        + '"' ]
    qstat_process = subprocess.Popen(jobid_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return re.findall(r'exec_host\W+(.+?)/', qstat_process.communicate()[0])[0]

def qdel_job(options, job_id):
    qdel_command = ['ssh', options.cluster, 'qdel', str(job_id)]
    if options.debug:
        print 'Killing JOB:', qdel_command
    subprocess.Popen(qdel_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

def verifyArgs(options):
    for key, value in vars(options).items():
        if type(value) == list and len(value) == 1:
            tmp_str = getattr(options, key)
            setattr(options, key, value[0])
    return options

def parseArguments(args=None):
    parser = argparse.ArgumentParser(description='Launch Ensight using DR options')
    options = parser.add_argument_group('Options', 'The following options control how Ensight will be launched')
    options.add_argument('--cluster', nargs=1, metavar='str', default=['flogin2'], help='Specify the cluster to use (default: flogin2)\n ')
    options.add_argument('--select', nargs=1, metavar='int', default=[8], help='Specify how many nodes to utilize (default: 8)\n ')
    options.add_argument('--procs', nargs=1, metavar='int', default=[2], help='Specify how many processors to utilize (default: 2)\n ')
    options.add_argument('--walltime', metavar='str', nargs=1, type=str, default=['24:00:00'], help='Specify the wall time of the job (default: 24:00:00)\n ')
    options.add_argument('--queue', metavar='str', nargs=1, type=str, default=['general'], help='Specify the job queue (default: general)\n ')
    options.add_argument('--save-output', action='store_const', const=True, default=False, help='Save the job output. (default: False)\n ')

    ensight_options = parser.add_argument_group('Ensight Options', 'The following options are specific to CEI and SOS')
    ensight_options.add_argument('--cei-port', nargs=1, metavar='int', type=int, default=[12350], help='Specify the CEI port (default: 12350)\n ')
    ensight_options.add_argument('--sos-port', nargs=1, metavar='int', type=int, default=[34568], help='Specify the SOS port (default: 34568)\n ')

    script_options = parser.add_argument_group('EnsightDR Launcher Options', 'The following options are specific to this launcher script')
    script_options.add_argument('--debug', action='store_const', const=True, default=False, help='Print any extras as we run\n ')

    return parser.parse_args(args)

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

if __name__ == '__main__':
    options = verifyArgs(parseArguments())
    launchCEI(options)
