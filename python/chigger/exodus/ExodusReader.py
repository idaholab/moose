#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import collections
import bisect
import contextlib
import fcntl
import vtk

import mooseutils
from .. import utils
from .. import base

@contextlib.contextmanager
def lock_file(filename):
    """
    Locks a file so that the exodus reader can safely read
    a file without MOOSE writing to it while we do it.
    """
    with open(filename, "a+") as f: # "a+" to make sure it gets created
        fcntl.flock(f, fcntl.LOCK_SH)
        yield
        fcntl.flock(f, fcntl.LOCK_UN)

class ExodusReaderErrorObserver(object):
    """
    Observes the errors that occur in ExodusReader.

    see peacock.ExodusViewer.plugins.VTKWindowPlugin
    """
    def __init__(self):
        self._errors = []

    def __call__(self, *args):
        """
        This method is called by VTK and includes the error information.
        """
        self._errors.append(args)

    def __bool__(self):
        """
        Return True if an error occured.
        """
        return len(self._errors) > 0

    def errors(self):
        """
        Return the list of errors.
        """
        return self._errors


class ExodusReader(base.ChiggerObject):
    """
    A reader for an ExodusII file.

    This class automatically handles the adaptivity files names as follows:
        file.e, file.e-s002, file.e-s003, etc.

    Additionally, it also investigates the modified times of the file(s) so that adaptive files that
    are older do not get loaded with newer data.
    """

    # The vtkMultiBlockDataSet stored by vtkExodusIIReader has 8 data blocks, each data block is
    # associated with a different connectivity type. This list contains a list of enums used by VTK
    # to make the linkage between the connectivity and the point/field data stored.
    #
    # vtkThe MultiBlockDataSet (vtkExodusIIReader::GetOutput()) is ordered according to
    # vtkExodusIIReader::cont_types, which is the order used here.
    MULTIBLOCK_INDEX_TO_OBJECTTYPE = [vtk.vtkExodusIIReader.ELEM_BLOCK, # 0 (MOOSE Subdomains)
                                      vtk.vtkExodusIIReader.FACE_BLOCK, # 1
                                      vtk.vtkExodusIIReader.EDGE_BLOCK, # 2
                                      vtk.vtkExodusIIReader.ELEM_SET,   # 3
                                      vtk.vtkExodusIIReader.SIDE_SET,   # 4 (MOOSE Boundaries)
                                      vtk.vtkExodusIIReader.FACE_SET,   # 5
                                      vtk.vtkExodusIIReader.EDGE_SET,   # 6
                                      vtk.vtkExodusIIReader.NODE_SET]   # 7 (MOOSE Nodesets)

    # "Enum" values for Subdomains, Sidesets, and Nodesets
    BLOCK = vtk.vtkExodusIIReader.ELEM_BLOCK
    SIDESET = vtk.vtkExodusIIReader.SIDE_SET
    BOUNDARY = vtk.vtkExodusIIReader.SIDE_SET
    NODESET = vtk.vtkExodusIIReader.NODE_SET
    BLOCK_TYPES = [BLOCK, SIDESET, NODESET]

    # "Enum" values for Variable Types
    NODAL = vtk.vtkExodusIIReader.NODAL
    ELEMENTAL = vtk.vtkExodusIIReader.ELEM_BLOCK
    GLOBAL = vtk.vtkExodusIIReader.GLOBAL
    VARIABLE_TYPES = [ELEMENTAL, NODAL, GLOBAL]

    # Information data structures
    BlockInformation = collections.namedtuple('BlockInformation', ['name', 'object_type',
                                                                   'object_index', 'number',
                                                                   'multiblock_index'])
    VariableInformation = collections.namedtuple('VariableInformation', ['name', 'object_type',
                                                                         'num_components'])
    FileInformation = collections.namedtuple('FileInformation', ['filename', 'times', 'modified'])
    TimeData = collections.namedtuple('TimeData', ['timestep', 'time', 'filename', 'index'])

    @staticmethod
    def getOptions():
        opt = base.ChiggerObject.getOptions()
        opt.add('time', "The time to view, if not specified the last timestep is displayed.",
                vtype=float)
        opt.add("timestep", -1, "The simulation timestep. (Use -1 for latest.)", vtype=int)
        opt.add("adaptive", True, "Load adaptive files (*.e-s* files).")
        opt.add('displacements', True, "Enable the viewing of displacements.")
        opt.add('displacement_magnitude', 1.0, "The displacement magnitude vector.")
        opt.add('variables', "A list of  active variables, if not specified all variables are "
                             "loaded.", vtype=list)
        opt.add('nodeset', None, "A list of nodeset ids or names to display, use [] to display all "
                                 "nodesets.", vtype=list)
        opt.add('boundary', None, "A list of boundary ids (sideset) ids or names to display, use "
                                  "[] to display all sidesets", vtype=list)
        opt.add('block', None, "A list of subdomain (block) ids or names to display, by default if "
                               "'nodeset' and 'sideset' are not specified all blocks are shown.",
                vtype=list)
        opt.add('squeeze', False, "Calls SetSqueezePoints on vtkExodusIIReader, according to the "
                                  "VTK documentation setting this to False should be faster.")
        return opt

    def __init__(self, filename, **kwargs):
        super(ExodusReader, self).__init__(**kwargs)

        # Set the filename for the reader.
        self.__filename = filename
        if not os.path.isfile(self.__filename):
            raise IOError("The file {} is not a valid filename.".format(self.__filename))

        self.__vtkreader = vtk.vtkExodusIIReader()
        self.__active = None # see utils.get_active_filenames
        self.__current = None # current TimeData object
        self.__timedata = [] # all the TimeData objects
        self.__fileinfo = collections.OrderedDict() # sorted FileInformation objects
        self.__blockinfo = dict() # BlockInformation objects
        self.__variableinfo = collections.OrderedDict() # VariableInformation objects

        # Error handling
        self._error_observer = ExodusReaderErrorObserver()
        self.__vtkreader.AddObserver('ErrorEvent', self._error_observer)

    def update(self, **kwargs):
        """
        After changing settings and prior to using data accessing methods, this method should be
        called. (public)

        In general, if you are not calling methods on this object then the "update" method does not
        need to be called, it will be called automatically by other classes that use it.

        Inputs: key,value pairs containing valid options as defined in the getOptions static
        function.

        Returns: None
        """
        super(ExodusReader, self).update(**kwargs)

        # Initialize the current time data
        self.__initializeTimeInformation()
        self.__current = self.__getTimeInformation()
        self.__vtkreader.SetFileName(None) # http://vtk.1045678.n5.nabble.com/How-to-re-load-time-information-in-ExodusIIReader-tp5741615.html pylint: disable=line-too-long
        with lock_file(self.__current.filename):
            self.__vtkreader.SetFileName(self.__current.filename)
            self.__vtkreader.SetTimeStep(self.__current.index)
            self.__vtkreader.UpdateInformation()
            self.__vtkreader.Modified()

            # Displacement Settings
            if self.getOption('displacements'):
                self.__vtkreader.ApplyDisplacementsOn()
                self.__vtkreader.SetDisplacementMagnitude(self.getOption('displacement_magnitude'))
            else:
                self.__vtkreader.ApplyDisplacementsOff()

            # Set the geometric objects to load (i.e., subdomains, nodesets, sidesets)
            active_blockinfo = self.__getActiveBlocks()
            blockinfo = self.getBlockInformation()
            for object_type in ExodusReader.BLOCK_TYPES:
                for data in blockinfo[object_type].values():
                    if (not active_blockinfo) or (data in active_blockinfo):
                        self.__vtkreader.SetObjectStatus(data.object_type, data.object_index, 1)
                    else:
                        self.__vtkreader.SetObjectStatus(data.object_type, data.object_index, 0)

            # According to the VTK documentation setting this to False (not the default) speeds
            # up data loading. In my testing I was seeing load times cut in half or more with
            # "squeezing" disabled. I am leaving this as an option just in case we discover some
            # reason it shouldn't be disabled.
            self.__vtkreader.SetSqueezePoints(self.getOption('squeeze'))

            # Set the data arrays to load
            #
            # If the object has not been initialized then all of the variables should be enabled
            # so that the block and variable information are complete when populated. After this
            # only the variables listed in the 'variables' options, if any, are activated, which
            # reduces loading times. If 'variables' is not given, all the variables are loaded.
            variables = self.getOption('variables')
            variable_info = self.getVariableInformation()
            for vinfo in variable_info.values():
                if (not variables) or (vinfo.name in variables):
                    self.__vtkreader.SetObjectArrayStatus(vinfo.object_type, vinfo.name, 1)
                else:
                    self.__vtkreader.SetObjectArrayStatus(vinfo.object_type, vinfo.name, 0)

            self.__vtkreader.Update()

    def needsUpdate(self):
        """ Determine the status of the object to indicate if the "update" method should be called.
        (public)

        Returns: bool: True when the settings and/or file(s) have been changed and the reader needs
        to be updated. """
        active = self.__getActiveFilenames()
        if self.__active != active:
            self.setNeedsUpdate(True)
        return super(ExodusReader, self).needsUpdate()

    def getErrorObserver(self):
        return self._error_observer

    def filename(self):
        """
        Return the filename supplied to this reader.
        """
        return self.__filename

    def getGlobalData(self, variable):
        """
        Access the global (i.e., Postprocessor) data contained in the Exodus file. (public)

        Inputs: variable[str]: An available and active (in 'variables' setting) GLOBAL variable
        name.

        Returns: float: The global data (Postprocessor value) for the current timestep and defined
        variable.

        Note: This function can also return the "Info_Records", which in MOOSE contains input file
        and other information from MOOSE, in this case a list of strings is returned.
        reader.GetFieldData('Info_Records') """
        self.checkUpdateState()

        field_data = self.__vtkreader.GetOutput().GetBlock(0).GetBlock(0).GetFieldData()
        varinfo = self.__variableinfo[variable]

        if varinfo.object_type != self.GLOBAL:
            msg = 'The variable "{}" must be a global variable.'.format(variable)
            raise mooseutils.MooseException(msg)

        vtk_array = field_data.GetAbstractArray(variable)
        return vtk_array.GetComponent(self.__current.index, 0)

    def getTimeData(self):
        """
        The current time information. (public)

        Returns:
            ExodusReader.TimeData: A collections.namedtuple with the current timestep, time,
                                   filename and local time index for the file.
        """
        self.checkUpdateState()
        return self.__current

    def getTimes(self):
        """
        All times for the current Exodus file(s). (public)

        Returns:
            list: A list of all times.
        """
        self.checkUpdateState()
        return [t.time for t in self.__timedata if t.time != None]

    def getBlockInformation(self):
        """
        Get the block (subdomain, nodeset, sideset) information. (public)

        Inputs:
            check[bool]: (Default: True) When True, perform an update check and raise an exception
                                         if object is not up-to-date. This should not be used.

        TODO: For Peacock, on linux check=False must be set, but I am not sure why.

        Returns:
            dict: BlockInformation objects for all data types in this object (keys are "enum" values
                  found in ExodusReader.MULTIBLOCK_INDEX_TO_OBJECTTYPE)
        """
        self.__initializeBlockInformation()
        return self.__blockinfo

    def getVariableInformation(self, var_types=None):
        """
        Information on available variables. (public)

        Inputs:
            var_types[list]: List of variable types to return (default: ExodusReader.VARIABLE_TYPES)

        Returns:
            OrderedDict: VariableInformation objects for all variables in the file.
        """
        self.__initializeVariableInformation()

        if var_types is None:
            var_types = ExodusReader.VARIABLE_TYPES

        variables = collections.OrderedDict()
        for name, var in self.__variableinfo.items():
            if var.object_type in var_types:
                variables[name] = var
        return variables

    def getVTKReader(self):
        """
        Return the underlying vtkExodusIIReder object. (public)

        Generally, this should not be utilized. This method exists for connecting output ports with
        the ExodusSource.
        """
        return self.__vtkreader

    def __getTimeInformation(self):
        """
        Helper for getting the current TimeData object using the 'time' and 'timestep' options.
        (private)

        Returns:
            TimeData: The current TimeData object.
        """

        # Time/timestep both set, unset 'time' and use 'timestep'
        if self.isOptionValid('timestep') and self.isOptionValid('time'):
            time = self.getOption('time')
            timestep = self.getOption('timestep')
            self._options.raw('time').unset()
            msg = "Both 'time' ({}) and 'timestep' ({}) are set, 'timestep' is being used."
            mooseutils.mooseWarning(msg.format(time, timestep))

        # Timestep
        timestep = -1
        n = len(self.__timedata)-1
        if self.isOptionValid('timestep'):
            timestep = self.getOption('timestep')

            # Account for out-of-range timesteps
            if (timestep < 0) and (timestep != -1):
                mooseutils.mooseWarning("Timestep out of range:", timestep, 'not in', repr([0, n]))
                self.setOption('timestep', 0)
                timestep = 0
            elif timestep > n:
                mooseutils.mooseWarning("Timestep out of range:", timestep, 'not in', repr([0, n]))
                self.setOption('timestep', n)
                timestep = n

        # Time
        elif self.isOptionValid('time'):
            times = [t.time for t in self.__timedata]
            idx = bisect.bisect_right(times, self.getOption('time')) - 1
            if idx < 0:
                idx = 0
            elif idx > n:
                idx = -1
            timestep = idx

        else:
            t = self.getOption('time')
            ts = self.getOption('timestep')
            mooseutils.mooseError('Invalid time ({}) and timestep({}) options.'.format(t, ts))

        return self.__timedata[timestep]

    def __getActiveFilenames(self):
        """
        The active ExodusII file(s). (private)

        Returns:
            list: Contains tuples (filename, modified time) of active file(s).
        """
        if self.isOptionValid('adaptive'):
            return utils.get_active_filenames(self.__filename, self.__filename + '-s*')
        else:
            return utils.get_active_filenames(self.__filename)

    def __getActiveBlocks(self):
        """
        Get a list of active blocks/boundary/nodesets. (private)

        Returns an empty list if all should be enabled.
        """
        blockinfo = self.getBlockInformation()
        output = []
        for param, object_type in zip(['block', 'boundary', 'nodeset'],
                                      [self.BLOCK, self.SIDESET, self.NODESET]):
            if self.isOptionValid(param):
                blocks = self.getOption(param)
                for data in blockinfo[object_type].values():
                    if data.name in blocks:
                        output.append(data)
        return output

    def __initializeTimeInformation(self):
        """
        Queries the vtkExodusIIReader for the current time information. (private)
        """

        # Create list of filenames to consider
        filenames = self.__getActiveFilenames()
        self.__active = filenames

        # Re-move any old files in timeinfo dict()
        for fname in list(self.__fileinfo.keys()):
            if fname not in filenames:
                self.__fileinfo.pop(fname)

        # Loop through each file and determine the times
        key = vtk.vtkStreamingDemandDrivenPipeline.TIME_STEPS()
        for filename, current_modified in filenames:

            tinfo = self.__fileinfo.get(filename, None)
            if tinfo and (tinfo.modified == current_modified):
                continue

            with lock_file(filename):
                self.__vtkreader.SetFileName(filename)
                self.__vtkreader.Modified()
                self.__vtkreader.UpdateInformation()

                vtkinfo = self.__vtkreader.GetExecutive().GetOutputInformation(0)
                steps = range(self.__vtkreader.GetNumberOfTimeSteps())
                times = [vtkinfo.Get(key, i) for i in steps]

                if not times:
                    times = [None] # When --mesh-only is used, not time information is written
                self.__fileinfo[filename] = ExodusReader.FileInformation(filename=filename,
                                                                         times=times,
                                                                         modified=current_modified)

        # Re-populate the time data
        self.__timedata = []
        timestep = 0
        for tinfo in self.__fileinfo.values():
            for i, t in enumerate(tinfo.times):
                tdata = ExodusReader.TimeData(timestep=timestep, time=t, filename=tinfo.filename,
                                              index=i)
                self.__timedata.append(tdata)
                timestep += 1

    def __initializeBlockInformation(self):
        """
        Queries the vtkExodusIIReader object for the subdomain, sideset, nodeset information.
        (private)
        """
        self.__blockinfo = dict()

        # Index to be used with the vtkExtractBlock::AddIndex method
        index = 0

        # Loop over all blocks of the vtk.MultiBlockDataSet
        for obj_type in ExodusReader.MULTIBLOCK_INDEX_TO_OBJECTTYPE:
            index += 1
            self.__blockinfo[obj_type] = dict()
            for j in range(self.__vtkreader.GetNumberOfObjects(obj_type)):
                index += 1
                name = self.__vtkreader.GetObjectName(obj_type, j)
                vtkid = str(self.__vtkreader.GetObjectId(obj_type, j))
                if name.startswith('Unnamed'):
                    name = vtkid

                binfo = ExodusReader.BlockInformation(object_type=obj_type, name=name, number=vtkid,
                                                      object_index=j, multiblock_index=index)
                self.__blockinfo[obj_type][vtkid] = binfo

    def __initializeVariableInformation(self):
        """
        Queries the vtkExodusIIReader for the current time information. (private)
        """
        unsorted = dict()
        for variable_type in ExodusReader.VARIABLE_TYPES:
            for i in range(self.__vtkreader.GetNumberOfObjectArrays(variable_type)):
                var_name = self.__vtkreader.GetObjectArrayName(variable_type, i)
                if var_name is not None:
                    num = self.__vtkreader.GetNumberOfObjectArrayComponents(variable_type, i)
                    vinfo = ExodusReader.VariableInformation(name=var_name,
                                                             object_type=variable_type,
                                                             num_components=num)
                    unsorted[var_name] = vinfo

        self.__variableinfo = collections.OrderedDict(sorted(unsorted.items(),
                                                             key=lambda x: x[0].lower()))

    def __str__(self):
        """
        Overload the str function so that information is printed when print is called on the object.
        """
        self.checkUpdateState()

        # The string to return
        out = ''

        # Variables
        variables = [(ExodusReader.NODAL, 'NODAL VARIABLES'),
                     (ExodusReader.ELEMENTAL, 'ELEMENTAL VARIABLES'),
                     (ExodusReader.GLOBAL, 'POSTPROCESSORS')]
        for vartype, vartitle in variables:
            out += '\n{}:\n'.format(mooseutils.colorText(vartitle, 'GREEN'))
            for varinfo in self.getVariableInformation([vartype]).values():
                out += '  {}\n'.format(mooseutils.colorText(varinfo.name, 'CYAN'))
                out += '{:>16}: {}\n'.format('components', varinfo.num_components)
        return out
