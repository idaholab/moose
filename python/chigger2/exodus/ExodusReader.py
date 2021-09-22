#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import sys
import os
import collections
import bisect
import contextlib
import fcntl
import vtk
from vtk.util.vtkAlgorithm import VTKPythonAlgorithmBase

from moosetools import mooseutils
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

@mooseutils.addProperty('name', ptype=str, required=True)
@mooseutils.addProperty('object_type', ptype=int, required=True)
@mooseutils.addProperty('num_components', ptype=int, required=True)
@mooseutils.addProperty('active', ptype=bool, default=True)
class VarInfo(mooseutils.AutoPropertyMixin):
    """
    Storage for variable information.

    This is a stand-alone class to allow for the 'active' property to be altered.
    """
    @property
    def fullname(self):
        return '{}::{}'.format(self.name, ExodusReader.VARIABLE_TYPES_NAMES[self.object_type])

    def __repr__(self):
        return '{} (name={}, object_type={}, num_components={}, active={})'.format(self.fullname, self.name, self.object_type, self.num_components, self.active)

@mooseutils.addProperty('name', ptype=str, required=True)
@mooseutils.addProperty('number', ptype=int, required=True)
@mooseutils.addProperty('object_type', ptype=int, required=True)
@mooseutils.addProperty('object_index', ptype=int, required=True)
@mooseutils.addProperty('multiblock_index', ptype=int, required=True)
@mooseutils.addProperty('active', ptype=bool, default=True)
class BlockInfo(mooseutils.AutoPropertyMixin):
    """
    Storage for block information.

    This is a stand-alone class to allow for the 'active' property to be altered.
    """
    def __repr__(self):
        return '{} (name={}, number={}, object_type={}, object_index={}, multiblock_index={}, active={})'.format(self.name, self.number, self.object_type, self.object_index, self.multiblock_index, self.active)

FileInfo = collections.namedtuple('FileInformation', ['filename', 'times', 'modified', 'vtkreader'])
TimeInfo = collections.namedtuple('TimeInformation', ['timestep', 'time', 'filename', 'index'])

class ExodusReader(base.ChiggerAlgorithm, VTKPythonAlgorithmBase):
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
    # vtkExodusIIReader::ObjectType values in the order used here.
    MULTIBLOCK_INDEX_TO_OBJECTTYPE = [vtk.vtkExodusIIReader.ELEM_BLOCK, # 0 (MOOSE Subdomains)
                                      vtk.vtkExodusIIReader.FACE_BLOCK, # 1
                                      vtk.vtkExodusIIReader.EDGE_BLOCK, # 2
                                      vtk.vtkExodusIIReader.ELEM_SET,   # 3
                                      vtk.vtkExodusIIReader.SIDE_SET,   # 4 (MOOSE Sidesets)
                                      vtk.vtkExodusIIReader.FACE_SET,   # 5
                                      vtk.vtkExodusIIReader.EDGE_SET,   # 6
                                      vtk.vtkExodusIIReader.NODE_SET]   # 7 (MOOSE Nodesets)

    # "Enum" values for Subdomains, Sidesets, and Nodesets
    BLOCK = vtk.vtkExodusIIReader.ELEM_BLOCK
    SIDESET = vtk.vtkExodusIIReader.SIDE_SET
    NODESET = vtk.vtkExodusIIReader.NODE_SET
    BLOCK_TYPES = [BLOCK, SIDESET, NODESET]

    # "Enum" values for Variable Types
    NODAL = vtk.vtkExodusIIReader.NODAL
    ELEMENTAL = vtk.vtkExodusIIReader.ELEM_BLOCK
    GLOBAL = vtk.vtkExodusIIReader.GLOBAL
    VARIABLE_TYPES = [ELEMENTAL, NODAL, GLOBAL]
    VARIABLE_TYPES_NAMES = {ELEMENTAL:'ELEMENTAL', NODAL:'NODAL', GLOBAL:'GLOBAL'}

    # Used by utils.get_current_exodus_reader for automatically adding reader objects to the
    # current ExodusSource.
    __CHIGGER_CURRENT__ = None

    @staticmethod
    def validParams():
        opt = base.ChiggerAlgorithm.validParams()
        opt.add('filename', vtype=str, doc="The filename to load, this is typically set via the constructor.")

        opt.add('time', vtype=(int, float),
                doc="The time to view, if not specified the 'timestep' is used.")
        opt.add("timestep", default=-1, vtype=int,
                doc="The simulation timestep, this is ignored if 'time' is set (-1 for latest.)")

        opt.add("adaptive", default=True, vtype=bool,
                doc="Load adaptive files (*.e-s* files).")

        opt.add('time_interpolation', default=True, vtype=bool,
                doc="Enable/disable automatic time interpolation.")

        opt.add('displacements', default=True, vtype=bool,
                doc="Enable the viewing of displacements.")
        opt.add('displacement_magnitude', default=1.0, vtype=(int, float),
                doc="The displacement magnitude vector.")

        opt.add('squeeze', default=False,
                doc="Calls SetSqueezePoints on vtkExodusIIReader, according to the "
                    "VTK documentation setting this to False should be faster.")

        opt.add('variables', vtype=str, array=True,
                doc="A tuple of  active variables, if not specified all variables are loaded.")

        opt.add('blocks', vtype=(int, str), array=True, default=tuple(),
                doc="A tuple of subdomain (block) ids or names to load, by default all are loaded. "
                    "If 'blocks', 'nodesets', or 'sidesets' is set, the default for the others "
                    "becomes unloaded.")
        opt.add('nodesets', vtype=(int, str), array=True, default=tuple(),
                doc="A tuple of nodeset ids to load, by default all are loaded. "
                    "If 'blocks', 'nodesets', or 'sidesets' is set, the default for the others "
                    "becomes unloaded.")
        opt.add('sidesets', vtype=(int, str), array=True, default=tuple(),
                doc="A tuple of boundary ids (sideset) to load, by default all are loaded."
                    "If 'blocks', 'nodesets', or 'sidesets' is set, the default for the others "
                    "becomes unloaded.")
        return opt

    def __init__(self, **kwargs):
        ExodusReader.__CHIGGER_CURRENT__ = self
        base.ChiggerAlgorithm.__init__(self, **kwargs)
        VTKPythonAlgorithmBase.__init__(self, nInputPorts=0, nOutputPorts=1, outputType='vtkMultiBlockDataSet')

        self.__filenames = list()                   # current list of files
        self.__timeinfo = list()                    # TimeInfo objects
        self.__fileinfo = collections.OrderedDict() # FileInfo objects
        self.__blockinfo = set()                    # BlockInfo objects
        self.__variableinfo = list()                # VarInfo objects

    def _onRequestInformation(self, *args):
        """(override,protected)
        Do not call this method, call updateInformation.
        """
        base.ChiggerAlgorithm._onRequestInformation(self, *args)

        # The file to load
        filename = self.getParam('filename')
        if not os.path.isfile(filename):
            self.error("The file {} is not a valid filename.", filename)
            return 0

        # Complete list of filenames with adaptive suffixes (-s002, ...) the file, time, and
        # block information only needs to be updated if the file list changed
        filenames = self.__getActiveFilenames(self.getParam('filename'))
        if self.__filenames != filenames:
            self.__filenames = filenames

            # Build FileInfo object for each filename
            self.__updateFileInformation()

            # Build TimeInfo object for each timestep
            self.__updateTimeInformation()

            # Build BlockInfo from the first file
            self.__updateBlockInformation()

            # Build VarInfo from the first file
            self.__updateVariableInformation()

        # Update active blocks, etc.
        self.__updateActiveBlocks()

        # Update active variables
        self.__updateActiveVariables()

    def _onRequestData(self, inInfo, outInfo):
        """(override, protected)
        Do not call this method, call updateData.
        """
        base.ChiggerAlgorithm._onRequestData(self, inInfo, outInfo)

        # Initialize the current time data
        time0, time1 = self.__getCurrentTimeInformation()

        # Time Interpolation
        if (time0 is not None) and (time1 is not None):
            file0 = self.__fileinfo[time0.filename]
            file1 = self.__fileinfo[time1.filename]

            # Interpolation on same file
            if file0.vtkreader is file1.vtkreader:
                self.__onRequestDataHelper(file0.vtkreader)

                vtkobject = vtk.vtkTemporalInterpolator()
                vtkobject.SetInputConnection(0, file0.vtkreader.GetOutputPort(0))
                vtkobject.UpdateTimeStep(self.getParam('time'))

            # Interpolation across files
            else:
                self.error("Support for time interpolation across adaptive time steps is not supported.")
                return 0

        elif (time0 is not None):
            vtkobject = self.__fileinfo[time0.filename].vtkreader
            vtkobject.SetTimeStep(time0.index)
            self.__onRequestDataHelper(vtkobject)

        else:
            return 0

        # Update the Reader and output port
        vtkobject.Update()
        out_data = outInfo.GetInformationObject(0).Get(vtk.vtkDataObject.DATA_OBJECT())
        out_data.ShallowCopy(vtkobject.GetOutputDataObject(0))

    def getFilename(self):
        """(public)
        Return the filename supplied to this reader.
        """
        return self.__filename

    def getTimes(self):
        """(public)
        All times for the current Exodus file(s). (public)

        Returns:
            list: A list of all times.
        """
        self.updateInformation()
        return [t.time for t in self.__timeinfo if t.time != None]

    def getGlobalData(self, variable):
        """(public)
        Access the global (i.e., Postprocessor) data contained in the Exodus file. (public)

        Inputs:
            variable[str]: An available and active (in 'variables' setting) GLOBAL variable name.

        Returns:
            float: The global data (Postprocessor value) for the current timestep and defined
                   variable.
        """
        self.updateInformation()
        self.updateData()
        if not self.hasVariable(self.GLOBAL, variable):
            self.error("The supplied global variable, '{}', does not exist in {}.", variable,
                       self.getParam('filename'))
            return sys.float_info.min

        time0, time1 = self.__getCurrentTimeInformation()

        # Time Interpolation
        if (time0 is not None) and (time1 is not None):
            file0 = self.__fileinfo[time0.filename]
            file1 = self.__fileinfo[time1.filename]

            vtkobject0 = self.__fileinfo[time0.filename].vtkreader
            vtkobject0.SetTimeStep(time0.index)
            vtkobject0.Update()

            vtkobject1 = self.__fileinfo[time1.filename].vtkreader
            vtkobject1.SetTimeStep(time1.index)
            vtkobject1.Update()

            g0 = vtkobject0.GetOutput().GetBlock(0).GetBlock(0).GetFieldData().GetAbstractArray(variable).GetComponent(time0.index, 0)
            g1 = vtkobject1.GetOutput().GetBlock(0).GetBlock(0).GetFieldData().GetAbstractArray(variable).GetComponent(time1.index, 0)

            return utils.interp(self.getParam('time'), [time0.time, time1.time], [g0, g1])

        elif (time0 is not None):
            vtkobject = self.__fileinfo[time0.filename].vtkreader
            vtkobject.SetTimeStep(time0.index)
            vtkobject.Update()
            g0 = vtkobject.GetOutput().GetBlock(0).GetBlock(0).GetFieldData().GetAbstractArray(variable).GetComponent(time0.index, 0)
            return g0

    def getFileInformation(self):
        """(public)
        The complete set of file information. (public)

        Returns:
            dict: Keys are the filename and items are FileInfo objects.
        """
        self.updateInformation()
        return self.__fileinfo

    def getTimeInformation(self):
        """(public)
        The complete time information. (public)

        Returns:
            list: List of TimeInfo objects.
        """
        self.updateInformation()
        return self.__timeinfo

    def getCurrentTimeInformation(self):
        """(public)
        Gets the current TimeInfo object(s) using the 'time' and 'timestep' options.

        Returns:
            tuple(TimeInfo, TimeInfo|None): The current TimeInfo object(s), if the selected timestep
                or time matches exactly the first entry contains the current object and the second
                will be None. If it doesn't match exact the two TimeInfo objects returned will be
                those that bracket the desired value.
        """
        self.updateInformation()
        return self.__getCurrentTimeInformation()

    def getBlockInformation(self):
        """(public)
        Get the block (subdomain, nodeset, sideset) information. (public)

        Returns:
            dict: The keys are the object type id from VTK (see MULTIBLOCK_INDEX_TO_OBJECTTYPE),
                  the values are a list of BlockInfo objects.
        """
        self.updateInformation()
        return self.__blockinfo

    def getVariableInformation(self):
        """(public)
        Information on available variables. (public)

        Inputs:
            var_types[list]: List of variable types to return (default: ExodusReader.VARIABLE_TYPES)

        Returns:
            dict: The keys are the variable type id from VTK (see VARIABLE_TYPES), the values are
                  a list of VarInfo objects.
        """
        self.updateInformation()
        return self.__variableinfo

    def __getCurrentTimeInformation(self):
        """(private)
        getCurrentTimeInformation, without updateInformation call.
        """
        time = self.getParam('time')
        timestep = self.getParam('timestep')

        # "time" option
        n = len(self.__timeinfo) - 1
        if time is not None:
            times = [t.time for t in self.__timeinfo]

            # Error if supplied time is out of range
            if (time > times[-1]) or (time < times[0]):
                self.warning("Time out of range, {} not in {}, using the latest timestep.",
                             time, repr([times[0], times[-1]]))
                return None, None

            # Exact match
            try:
                idx = times.index(time)
                return self.__timeinfo[idx], None
            except ValueError:
                pass

            # Locate index less than or equal to the desired time
            idx = bisect.bisect_right(times, time) - 1
            if self.getParam('time_interpolation'):
                return self.__timeinfo[idx], self.__timeinfo[idx+1]
            else:
                return self.__timeinfo[idx], None

        # "timestep" option
        elif timestep is not None:

            # Account for out-of-range timesteps
            idx = timestep
            if (timestep < 0) and (timestep != -1):
                self.warning("Timestep out of range: {} not in {}.", timestep, repr([0, n]))
                idx = 0
            elif (timestep > n) or (timestep == -1):
                if timestep != -1:
                    self.warning("Timestep out of range: {} not in {}.", timestep, repr([0, n]))
                idx = n

            return self.__timeinfo[idx], None

        # Default to last timestep
        return self.__timeinfo[-1], None

    def hasVariable(self, var_type, var_name):
        """(public)
        Return True if the supplied variable name for the given type exists.
        """
        for var in self.__variableinfo[var_type]:
            if (var.name == var_name) or (var.fullname == var_name):
                return True
        return False

    def __onRequestDataHelper(self, vtkreader):
        """(private)
        Apply options to the current vtkExodusIIReader objects.

        Ideally, the content of this method would be implemented in the _onUpdateInformation, but
        but because of temporal interpolation which can have two vtkExodusIIReader objects a
        separate method is required. A separate method could be called during _onUpdateInformation,
        but to eliminate the duplication of the time logic it is called in _onUpdateData.
        """

        # Variable Status
        for var_group in self.__variableinfo.values():
            for vinfo in var_group:
                vtkreader.SetObjectArrayStatus(vinfo.object_type, vinfo.name, vinfo.active)

        # Block Status
        for blk_group in self.__blockinfo.values():
            for binfo in blk_group:
                vtkreader.SetObjectStatus(binfo.object_type, binfo.object_index, binfo.active)

        # Displacement Settings
        vtkreader.SetApplyDisplacements(self.getParam('displacements'))
        vtkreader.SetDisplacementMagnitude(self.getParam('displacement_magnitude'))

        # According to the VTK documentation setting this to False (not the default) speeds
        # up data loading. In my testing I was seeing load times cut in half or more with
        # "squeezing" disabled. I am leaving this as an option just in case we discover some
        # reason it shouldn't be disabled.
        vtkreader.SetSqueezePoints(self.getParam('squeeze'))

    def __getActiveFilenames(self, filename):
        """(private)
        The active ExodusII file(s).

        Returns:
            list: Contains tuples (filename, modified time) of active file(s).
        """
        if self.getParam('adaptive'):
            return utils.get_active_filenames(filename, filename + '-s*')
        else:
            return utils.get_active_filenames(filename)

    def __updateFileInformation(self):
        """(private)
        Helper that creates dict() that contains a FileInfo object for each file.
        """
        self.debug('__updateFileInformation')

        # Re-move any old files in timeinfo dict()
        filenames = [f[0] for f in self.__filenames]
        for fname in list(self.__fileinfo.keys()):
            if fname not in filenames:
                self.__fileinfo.pop(fname)

        # Loop through each file and determine the times
        key = vtk.vtkStreamingDemandDrivenPipeline.TIME_STEPS()
        for filename, current_modified in self.__filenames:

            tinfo = self.__fileinfo.get(filename, None)
            if tinfo and (tinfo.modified == current_modified):
                continue

            with lock_file(filename):
                vtkreader = vtk.vtkExodusIIReader()
                vtkreader.SetFileName(filename)
                vtkreader.UpdateInformation()

                vtkinfo = vtkreader.GetExecutive().GetOutputInformation(0)
                steps = range(vtkreader.GetNumberOfTimeSteps())
                times = [vtkinfo.Get(key, i) for i in steps]

                if not times:
                    times = [None] # When --mesh-only is used, no time information is written
                self.__fileinfo[filename] = FileInfo(filename=filename,
                                                     times=times,
                                                     modified=current_modified,
                                                     vtkreader=vtkreader)


    def __updateTimeInformation(self):
        """(private)
        Create a TimeInfo object for each timestep that indicates the correct file.
        """
        self.debug('__updateTimeInformation')

        self.__timeinfo = []
        timestep = 0
        for tinfo in self.__fileinfo.values():
            for i, t in enumerate(tinfo.times):
                tdata = TimeInfo(timestep=timestep, time=t, filename=tinfo.filename, index=i)
                self.__timeinfo.append(tdata)
                timestep += 1

    def __updateBlockInformation(self):
        """(private)
        Queries the base file for the available blocks, sideset, and nodesets. It is assumed
        that all files contain the same blocks, etc.
        """
        self.debug('__updateBlockInformation')

        self.__blockinfo = collections.defaultdict(list)
        finfo = self.__fileinfo[self.getParam('filename')]
        with lock_file(finfo.filename):
            index = 0                   # index to be used with the vtkExtractBlock::AddIndex method
            vtkreader = finfo.vtkreader # the vtkExodusIIReader object for the first file

            # Loop over all blocks of the vtk.MultiBlockDataSet
            for obj_type in ExodusReader.MULTIBLOCK_INDEX_TO_OBJECTTYPE:
                index += 1
                for j in range(vtkreader.GetNumberOfObjects(obj_type)):
                    index += 1
                    name = vtkreader.GetObjectName(obj_type, j)
                    vtkid = vtkreader.GetObjectId(obj_type, j)
                    if name.startswith('Unnamed'):
                        name = str(vtkid)

                    binfo = BlockInfo(name=name,
                                      number=vtkid,
                                      object_type=obj_type,
                                      object_index=j,
                                      multiblock_index=index)
                    self.__blockinfo[obj_type].append(binfo)

    def __updateVariableInformation(self):
        """(private)
        Queries the base file for the available variables, it is assumed that all files contain the
        same variables.
        """
        self.debug('__updateVariableInformation')
        finfo = self.__fileinfo[self.getParam('filename')]

        self.__variableinfo = dict()
        with lock_file(finfo.filename):
            vtkreader = finfo.vtkreader # the vtkExodusIIReader object for the first file
            for variable_type in ExodusReader.VARIABLE_TYPES:
                unsorted = set()
                for i in range(vtkreader.GetNumberOfObjectArrays(variable_type)):
                    var_name = vtkreader.GetObjectArrayName(variable_type, i)
                    if var_name is not None:
                        num = vtkreader.GetNumberOfObjectArrayComponents(variable_type, i)
                        vinfo = VarInfo(name=var_name, object_type=variable_type, num_components=num)
                        unsorted.add(vinfo)

                self.__variableinfo[variable_type] = sorted(unsorted, key=lambda x: '{}-{}'.format(x.name.lower(), x.object_type))

    def __updateActiveVariables(self):
        """(private)
        Set the data arrays (variables) to load.
        """
        self.debug('__updateActiveVariables')
        variables = self.getParam('variables')
        if variables is not None:
            self.__activeVariableCheck(variables)
            for var_group in self.__variableinfo.values():
                for var in var_group:
                    var.active = (var.name in variables) or (var.fullname in variables)
        else:
            for var_group in self.__variableinfo.values():
                for var in var_group:
                    var.active = True

    def __updateActiveBlocks(self):
        """(private)
        Set the geometric arrays (variables) to load.
        """
        self.debug('__updateActiveBlocks')
        b = self.getParam('blocks')
        n = self.getParam('nodesets')
        s = self.getParam('sidesets')
        self.__setActiveBlocks('blocks', b, (n, s), self.__blockinfo[ExodusReader.BLOCK])
        self.__setActiveBlocks('nodesets', n, (b, s), self.__blockinfo[ExodusReader.NODESET])
        self.__setActiveBlocks('sidesets', s, (b, n), self.__blockinfo[ExodusReader.SIDESET])

    def __setActiveBlocks(self, cmd, blocks, others, blk_info):
        """(private)
        see __updateActiveBlocks
        """
        if blocks:
            # Error check
            available = set([blk.name for blk in blk_info] + [blk.number for blk in blk_info] + [str(blk.number) for blk in blk_info])
            blk_set = set(blocks)
            blk_set.difference_update(available)
            if blk_set:
                self.warning("The following items in '{}' do not exist: {}", cmd, ', '.join(blk_set))

            # Set active status
            for blk in blk_info:
                blk.active = (blk.name in blocks) or (blk.number in blocks) or (str(blk.number) in blocks)

        elif blocks is None or any([bool(o) for o in others]):
            for blk in blk_info:
                blk.active = False
        else:
            for blk in blk_info:
                blk.active = True

    def __activeVariableCheck(self, variables):
        """(private)
        Helper for handling variable type suffix.
        """

        active_variables = collections.defaultdict(set)
        for var in variables:
            x = var.split("::")
            var_types = ExodusReader.VARIABLE_TYPES
            if (len(x) == 2) and (x[1] == 'NODAL'):
                var_types = [ExodusReader.NODAL]
            elif (len(x) == 2) and (x[1] == 'ELEMENTAL'):
                var_types = [ExodusReader.ELEMENTAL]
            elif (len(x) == 2) and (x[1] == 'GLOBAL'):
                var_types = [ExodusReader.GLOBAL]
            elif len(x) == 2:
                self.error("Unknown variable prefix '::{}', must be 'NODAL', 'ELEMENTAL', or 'GLOBAL'", x[1])

            for vtype in var_types:
                if self.hasVariable(vtype, var):
                    active_variables[var].add(vtype)

            for name, vtype in active_variables.items():
                if len(vtype) > 1:
                    self.warning("The variable name '{0}' exists with multiple data types ('{1}'), use the variable type as a prefix (e.g., '{0}::{2}') to limit the loading to a certain type.", var, ', '.join(self.VARIABLE_TYPES_NAMES[v] for v in vtype), self.VARIABLE_TYPES_NAMES[list(vtype)[0]])
