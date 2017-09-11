from QueueManager import QueueManager
import os, re
from TestHarness import util

## This Class is responsible for maintaining an interface to the PBS scheduling syntax
class RunPBS(QueueManager):
    @staticmethod
    def validParams():
        params = QueueManager.validParams()
        params.addParam('queue_template', os.path.join(os.path.abspath(os.path.dirname(__file__)), 'pbs_template'), "Location of the PBS template")
        return params

    def __init__(self, harness, params):
        QueueManager.__init__(self, harness, params)
        self.params = params

    def getLaunchCommand(self, job_container):
        """ Return appropriate PBS command depending on current QueueManager state """
        test_unique = self.getUnique(job_container)

        if self.checkStatusState():
            job = self.getData(test_unique, job_id=True)
            return 'qstat -xf %s' % (job['job_id'])
        else:
            job = self.getData(test_unique, template_data=True)
            return 'qsub -h %s' % (job['template_data']['queue_script'])

    def launchJob(self, job_container):
        """ Perform necessary file copy and script creation. Execute job """
        test_unique = self.getUnique(job_container)

        # create and populate our template
        template = self.populateTemplateParams(job_container, {})
        self.putData(test_unique, template_data=template)

        # Prepare the worker directory
        self.copyFiles(job_container)

        # Create the batch script
        self.prepareQueueScript(template)

        # Run command
        output = util.runCommand(self.getLaunchCommand(job_container), cwd=template['working_dir'])
        self.handleLaunch(job_container, output)

    def checkJob(self, job_container):
        """ Perform QSTAT and discover job status """
        output = util.runCommand(self.getLaunchCommand(job_container))
        self.handleQueueStatus(job_container, output)

    def postLaunch(self, jobs):
        """ Release jobs that were placed on hold """
        print 'releasing jobs...'
        # TODO, instead of one job at a time method, create a buffer of IDs to release at a time
        for job_container in jobs:
            test_unique = self.getUnique(job_container)
            json_session = self.getData(test_unique, job_id=True)
            util.runCommand('qrls %s' % (json_session['job_id']))

    def populateTemplateParams(self, job_container, template):
        """ Populate QSUB template with relevent information from the tester params """
        tester = job_container.getTester()

        # Populate template with available params
        for param in self.params.keys():
            template[param] = self.params[param]

        # Discover prereq launch job IDs
        prereqs = tester.getPrereqs()
        if prereqs:
            prereq_job_ids = []
            for prereq_test_name in prereqs:
                pre_unique = os.path.join(tester.getTestDir(), prereq_test_name)
                tmp_json = self.getData(pre_unique, job_id=True)
                prereq_job_ids.append(tmp_json['job_id'])

            template['prereq'] = '#PBS -W depend=afterany:%s' % (':'.join(prereq_job_ids))
            template['prereq_ids'] = prereq_job_ids
        else:
            template['prereq'] = ''

        # Convert MAX_TIME to hours:minutes for walltime use
        hours = int(int(tester.specs['max_time']) / 3600)
        minutes = int(int(tester.specs['max_time']) / 60) % 60
        template['walltime'] = '{0:02d}'.format(hours) + ':' + '{0:02d}'.format(minutes) + ':00'

        # # # Add PBS project directive
        if self.options.queue_project:
            template['pbs_project'] = '#PBS -P %s' % (self.options.queue_project)

        # Set node placement
        template['place'] = 'free'

        # Combined stdout and stderr into one stream
        template['combine_streams'] = '#PBS -j oe'

        # Set CPU request count
        template['mpi_procs'] = tester.getProcs(self.options)

        # Set a path friendly job name
        template['job_name'] = ''.join(txt for txt in tester.specs['test_name'] if txt.isalnum() or txt in ['_', '-'])

        # Set working directory
        template['working_dir'] = self.getWorkingDir(job_container)

        # Join the actual command we will execute inside the qsub script
        template['command'] = tester.getCommand(self.options)

        # #### create a set for copy and nocopy so its easier to work with
        no_copy_files = set([])
        copy_files = set([])

        if tester.specs.isValid('no_copy_files'):
            no_copy_files.update(tester.specs['no_copy_files'])
        if tester.specs.isValid('copy_files'):
            copy_files.update(tester.specs['copy_files'])
        if tester.specs.isValid('gold_dir'):
            copy_files.update([tester.specs['gold_dir']])

        # convert the copy and nocopy sets to flat lists
        template['copy_files'] = list(copy_files)
        template['no_copy'] = list(no_copy_files)

        # The location of the resulting QSUB script
        template['queue_script'] = os.path.join(self.getWorkingDir(job_container), self.options.session_file + '-' + template['job_name'] + '.sh')

        return template

    def prepareQueueScript(self, template):
        """ Create the QSUB launch script """
        # Get a list of prereq tests this test may have
        f = open(self.params['queue_template'], 'r')
        content = f.read()
        f.close()

        f = open(template['queue_script'], 'w')

        # Do all of the replacements for the valid parameters
        for key in template.keys():
            if key.upper() in content:
                content = content.replace('<' + key.upper() + '>', str(template[key]))

        # Make sure we strip out any string substitution parameters that were not supplied
        for key in template.keys():
            if key.upper() not in content:
                content = content.replace('<' + key.upper() + '>', '')

        f.write(content)
        f.close()

    # Process the tester output and run processResults
    def testOutput(self, job_container):
        """
        Adjust the Tester for a proper working directory (PBS creates sub-directores),
        and allow derived Tester to perform processResults.

        Return resulting output.
        """
        tester = job_container.getTester()
        test_unique = self.getUnique(job_container)

        job = self.getData(test_unique, std_out=True, template_data=True, exit_code=True)
        template = job['template_data']

        with open(job['std_out'], 'r') as output_file:
            outfile = output_file.read()

        # Alter test_dir to reflect working_dir creation
        original_testdir = tester.getTestDir()
        tester.specs['test_dir'] = template['working_dir']

        if tester.hasRedirectedOutput(self.options):
            outfile += util.getOutputFromFiles(tester, self.options)

        # Allow the tester to verify its own output and set the status
        output = tester.processResults(tester.specs['moose_dir'], self.options, outfile)

        # reset the original test_dir
        tester.specs['test_dir'] = original_testdir

        # Set the testers output with modifications made above
        job_container.setOutput(output)

        return output

    def handleLaunch(self, job_container, output):
        """ Handle statuses supplied by qsub output """
        tester = job_container.getTester()
        test_unique = self.getUnique(job_container)
        pattern = re.compile(r'^(\d+)\.[\W\w]+$')

        if pattern.search(output):
            job_id = pattern.search(output).group(1)

            # Update the queue_data based on results
            job = self.getData(test_unique, template_data=True)
            template = job['template_data']

            self.putData(test_unique, job_id=job_id, std_out=os.path.join(template['working_dir'], template['job_name'] + '.o%s' %(job_id)))

            # Set the tester status
            tester.setStatus('%s LAUNCHED' % (str(job_id)), tester.bucket_pending)

        elif 'command not found' in output:
            tester.setStatus('QSUB NOT FOUND', tester.bucket_fail)

        else:
            tester.setStatus('QSUB INVALID RESULTS: %s' % (output), tester.bucket_fail)

        return output

    def handleQueueStatus(self, job_container, output):
        """
        Handle statuses supplied by qstat output.

        Resulting output returned from this method will either be
        output generated by a _finished_ Tester, or qstat command
        output (when verbose is active).
        """
        tester = job_container.getTester()
        test_unique = self.getUnique(job_container)

        output_value = re.search(r'job_state = (\w)', output)
        exit_code = re.search(r'Exit_status = (-?\d+)', output)
        if exit_code:
            exit_code = int(exit_code.group(1))

            # If we have an exit code from PBS, go ahead and update the queue data
            self.putData(test_unique, exit_code=exit_code)

        # Set the initial reason for a test NOT to be finished
        reason = ''

        if output_value:
            # Job is finished
            if output_value.group(1) == 'F':
                job = self.getData(test_unique, std_out=True)

                # Check that stdout file was created
                if os.path.exists(job['std_out']) and exit_code is not None:
                    # This job has finished with out any apparent errors, so lets allow
                    # the tester to processResults
                    output = self.testOutput(job_container)

                elif exit_code == None:
                    # This job was canceled or deleted (qdel)
                    reason = 'FAILED (PBS ERROR)'
                    tester.setStatus(reason, tester.bucket_fail)
                else:
                    # The job is done but there is no stdout file to read. Can happen when
                    # PBS had issues of some kind.
                    reason = 'NO STDOUT FILE'
                    tester.setStatus(reason, tester.bucket_fail)

            # Job is currently running
            elif output_value.group(1) == 'R':
                reason = 'RUNNING'

            # Job is exiting
            elif output_value.group(1) == 'E':
                reason = 'EXITING'

            # Job is currently queued
            elif output_value.group(1) == 'Q':
                reason = 'QUEUED'

            # Job is waiting for other jobs
            elif output_value.group(1) == 'H':
                reason = 'HOLDING'

            # Unknown statuses should be treated as failures
            else:
                reason = 'UNKNOWN PBS STATUS'
                tester.setStatus(reason, tester.bucket_fail)

            # Update the reason why this test is still pending (and adjust the test to have
            # the finished state of queued)
            if tester.isPending():
                tester.setStatus(reason, tester.bucket_queued)

        else:
            # Job status not available
            reason = 'INVALID QSTAT RESULTS'
            tester.setStatus(reason, tester.bucket_fail)

        return output
