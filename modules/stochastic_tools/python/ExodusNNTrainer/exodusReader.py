#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
"""
ExodusReader.py reads MOOSE Exodus output files and returns the requested variable values on cells throughout the mesh.
The user can specify a time sub-sampling factor S, so that values are read only every Sth time step.
Users can either run this code as a bash script to store the variable values as a Python pickle file that can be read back
in a Python script, or directly import this module and call the get_var_vals(fileName,varNames,S) function. 
"""

import chigger
import sys
from os import path
from vtk.util import numpy_support as VN
from vtk import vtkStructuredPointsReader
import vtk
import pickle
import argparse
import numpy as np

#reads the exodus file using chigger
def read_exodus(file_path):
    reader = chigger.exodus.ExodusReader(file_path)
    reader.update()
    reader.setOptions(block=['0'])
    return reader

#converts all the nodal variable values to cell type values
def get_cell_format_data(vtkReader):
    p2c = vtk.vtkPointDataToCellData()
    p2c.SetProcessAllArrays(True)

    p2c.SetInputConnection(vtkReader.GetOutputPort())
    p2c.Update()
    new_vtkReader=p2c.GetOutputDataObject(0)
    return new_vtkReader

#we call this funciton to read the exodus file and output the values for the requested variables at all time steps except 0, because 0 is initial condition data
def get_var_vals(fileName,varNames,S):
    reader = read_exodus(fileName)
    T = reader.getTimes()

    var_info=reader.getVariableInformation()
    if not (all(var in var_info.keys() for var in varNames ) ):
        print("varNames not found in exodus file")
        exit()

    for i,time in enumerate(T):
        if i==0:
            var_values_dict={}
            continue
        elif i%S ==0:
            reader.setOptions(timestep =i)
            reader.update()
            vtkReader  = reader.getVTKReader()

            data = get_cell_format_data(vtkReader).GetBlock(0).GetBlock(0)
            for var in varNames:
                time_step_data=data.GetCellData().GetArray(var)
                var_vals=[]
                var_vals = VN.vtk_to_numpy(time_step_data )
                try:
                    var_values_dict[var] = np.hstack([var_values_dict[var],var_vals] )
                except Exception as e:
                    var_values_dict[var]=var_vals

    return var_values_dict

if __name__ == '__main__':

    # PARSE USER INPUT
    parser=argparse.ArgumentParser(description='Python script for parsing Exodus-II files to generate training data for machine learning algorithms')
    parser.add_argument('--i',help='Exodus file name')

    parser.add_argument('--vars','--names-list',nargs='+',help='List of variable names to extract from the exodus file')

    parser.add_argument('--o',help='File name to output variable data to. Acceptable format is *.pkl') #currently only implemented pkl format

    parser.add_argument('--s',help='Sampling factor for time steps. Samples every s time steps',type=int,default=1)
    args = parser.parse_args()

    #EXTRACT PARAMETERS NEEDED FOR CODE EXECUTION

    exodus_file_name = args.i
    var_names = args.vars
    output_file = args.o
    S = args.s

    #PERFORM BASIC CHECKS ON USER INPUTS
    if not path.isfile(exodus_file_name):
        print("Exodus file or directory does not exist.")
        exit()
    if not path.exists(path.dirname(output_file)):
        print("Output path directory does not exist. Please provide full directory path to file")
        exit()
    if not S>0:
        print("Sampling factor cannot be negative")
        exit()

    print("Parsing file...")

    var_values = get_var_vals(exodus_file_name,var_names,S)

    #Dump as PKL file
    with open(output_file,'wb') as f:
        pickle.dump(var_values,f)
