  [./<JOB_NAME>]
    type = PBSJob
    chunks = 1
    place = free
    mpi_procs = <MIN_PARALLEL>
    moose_application = <EXECUTABLE>
    input_file = <INPUT>
    walltime = <WALLTIME>
    no_copy = <TEST_NAME>
    copy_files = gold
    combine_streams = True
    cli_args = <CLI_ARGS>
  [../]