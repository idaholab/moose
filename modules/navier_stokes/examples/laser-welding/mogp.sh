#!/bin/bash
#PBS -M Som.Dhulipala@inl.gov
#PBS -m abe
#PBS -N MOGP_LASER_WELD
#PBS -P moose
#PBS -l select=25:ncpus=40:mpiprocs=40
#PBS -l walltime=24:00:00

JOB_NUM=${PBS_JOBID%%\.*}

cd $PBS_O_WORKDIR

\rm -f out
date > out

module purge
module load use.moose moose-dev

MV2_ENABLE_AFFINITY=0 mpiexec ~/projects/moose/modules/combined/combined-opt -i mogp.i --allow-test-objects >> out

date >> out