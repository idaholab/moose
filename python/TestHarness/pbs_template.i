  [./<JOB_NAME>]
    test_name = <TEST_NAME>
    type = PBSJob
    chunks = 1
    place = free
    mpi_procs = <MIN_PARALLEL>
    moose_application = <EXECUTABLE>
    input_file = <INPUT>
    <PBS_STDOUT>
    <PBS_STDERR>
    <PBS_PROJECT>
    walltime = <WALLTIME>
    no_copy = <NO_COPY>
    no_copy_pattern = 'pbs_\d+.cluster'
    copy_files = gold
    combine_streams = True
    cli_args = <CLI_ARGS>
  [../]
