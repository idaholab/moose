#!/bin/bash
#PBS -N <JOB_NAME>
#PBS -l select=<CHUNKS>:ncpus=<NCPUS_PER_CHUNK>:mpiprocs=<MPI_PROCS_PER_CHUNK>
#PBS -l place=<PLACE>
#PBS -l walltime=<WALLTIME>
<QUEUE>

source /etc/profile.d/modules.sh
<MODULE>

cd $PBS_O_WORKDIR

export MV2_ENABLE_AFFINITY=0
date
mpiexec <MOOSE_APPLICATION> -i <INPUT_FILE> <THREADS> <CLI_ARGS>
date