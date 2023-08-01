#!/bin/bash
#PBS -N FSP_large
#PBS -l select=295:ncpus=48:mpiprocs=48
#PBS -l walltime=1:00:00
#PBS -j oe
#PBS -P neams
cd /scratch/lindad/moose/modules/navier_stokes/test/tests/finite_element/ins/lid_driven
module load mvapich2
mpiexec -np 14160 ../../../../../navier_stokes-opt -i lid_driven_fs.i --color off
