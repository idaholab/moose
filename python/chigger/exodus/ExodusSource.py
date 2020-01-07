#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import vtk
import mooseutils
from .ExodusReader import ExodusReader
from .. import utils
from .. import base
from .. import filters

class ExodusSource(base.ChiggerSource):
    """
    Object for visualizing 3D volumes from an ExodusReader object.

    See ExodusReader for details regarding the ExodusII file support provided.

    Inputs:
        reader[ExodusReader]: The reader object containing the ExodusII file desired to be open.
        **kwargs: see ChiggerSource
    """
    FILTER_TYPES = [filters.ContourFilter, filters.ClipperFilterBase, filters.GeometryFilter,
                    filters.TransformFilter, filters.TubeFilter, filters.RotationalExtrusionFilter]

    @staticmethod
    def getOptions():
        opt = base.ChiggerSource.getOptions()

        # Variable
        opt.add('variable', "The nodal or elemental variable to render.", vtype=str)
        opt.add('component', -1, "The vector variable component to render (-1 plots the "
                                 " magnitude).", vtype=int, allow=[-1, 0, 1, 2])

        # Subdomains/sidesets/nodesets
        opt.add('nodeset', None, "A list of nodeset ids or names to display, use [] to display all "
                                 "nodesets.", vtype=list)
        opt.add('boundary', None, "A list of boundary ids (sideset) ids or names to display, use "
                                  "[] to display all sidesets", vtype=list)
        opt.add('block', [], "A list of subdomain (block) ids or names to display, use [] to "
                             "dislpay all blocks.", vtype=list)

        opt.add('representation', 'surface', "View volume representation.",
                allow=['surface', 'wireframe', 'points'])

        opt.add('range', "The range of data to display on the volume and colorbar; range takes "
                         "precedence of min/max.", vtype=list)
        opt.add('min', "The minimum range.", vtype=float)
        opt.add('max', "The maximum range.", vtype=float)

        # Colormap
        opt += base.ColorMap.getOptions()
        return opt

    def __init__(self, reader, **kwargs):
        super(ExodusSource, self).__init__(**kwargs)

        if not isinstance(reader, ExodusReader):
            raise mooseutils.MooseException('The supplied reader must be a '
                                            '"chigger.readers.ExodusReader", but a "{}" was '
                                            'provided.'.format(type(reader).__name__))

        self.__reader = reader
        self.__current_variable = None
        self._colormap = base.ColorMap()

        self.__extract_indices = []
        self.__vtkextractblock = vtk.vtkExtractBlock()
        self.__vtkextractblock.SetInputConnection(self.__reader.getVTKReader().GetOutputPort())

        self._required_filters = [filters.GeometryFilter()]

    def getExodusReader(self):
        """
        Return the ExodusReader object.
        """
        return self.__reader

    def getCurrentVariableInformation(self):
        """
        Return the chigger ExodusReader object.
        """
        return self.__current_variable

    def getVTKSource(self):
        """
        Returns the vtkExtractBlock object used for pulling subdomain/sideset/nodeset data from the
        reader. (override)

        Returns:
            vtk.vtkExtractBlock (see ChiggerFilterSourceBase)
        """
        return self.__vtkextractblock

    def getBounds(self):
        """
        Return the extents of the active data objects.
        """
        self.checkUpdateState()

        bnds = []
        for i in range(self.__vtkextractblock.GetOutput().GetNumberOfBlocks()):
            current = self.__vtkextractblock.GetOutput().GetBlock(i)
            if isinstance(current, vtk.vtkUnstructuredGrid):
                bnds.append(current.GetBounds())

            elif isinstance(current, vtk.vtkMultiBlockDataSet):
                for j in range(current.GetNumberOfBlocks()):
                    bnds.append(current.GetBlock(j).GetBounds())

        return utils.get_bounds_min_max(*bnds)

    def getRange(self, local=False):
        """
        Return range of the active variable and blocks.
        """
        self.checkUpdateState()
        if self.__current_variable is None:
            return (None, None)
        elif not local:
            return self.__getRange()
        else:
            return self.__getLocalRange()

    def __getRange(self):
        """
        Private version of range for the update method.
        """
        component = self.getOption('component')
        pairs = []
        for i in range(self.__vtkextractblock.GetOutput().GetNumberOfBlocks()):
            current = self.__vtkextractblock.GetOutput().GetBlock(i)
            if isinstance(current, vtk.vtkUnstructuredGrid):
                array = self.__getActiveArray(current)
                if array:
                    pairs.append(array.GetRange(component))

            elif isinstance(current, vtk.vtkMultiBlockDataSet):
                for j in range(current.GetNumberOfBlocks()):
                    array = self.__getActiveArray(current.GetBlock(j))
                    if array:
                        pairs.append(array.GetRange(component))

        return utils.get_min_max(*pairs)

    def __getLocalRange(self):
        """
        Determine the range of visible items.
        """
        component = self.getOption('component')
        self.getVTKMapper().Update() # required to have up-to-date ranges
        data = self.getVTKMapper().GetInput()
        out = self.__getActiveArray(data)
        if out is not None:
            return out.GetRange(component)
        else:
            return [None, None]

    def __getActiveArray(self, data):
        """
        Return the vtkArray for the current variable.

        Inputs:
            data[vtkUnstructuredGrid]: The VTK data object to extract array from.

        see __GetRange and __GetBounds
        """

        name = self.__current_variable.name
        if self.__current_variable.object_type == ExodusReader.ELEMENTAL:
            for a in range(data.GetCellData().GetNumberOfArrays()):
                if data.GetCellData().GetArrayName(a) == name:
                    return data.GetCellData().GetAbstractArray(a)

        elif self.__current_variable.object_type == ExodusReader.NODAL:
            for a in range(data.GetPointData().GetNumberOfArrays()):
                if data.GetPointData().GetArrayName(a) == name:
                    return data.GetPointData().GetAbstractArray(a)
        else:
            raise mooseutils.MooseException('Unable to get the range for the '
                                            'variable "{}"'.format(self.__current_variable.name))

    def needsUpdate(self):
        """
        Indicates if this object needs its update method called. (override)

        This adds checking of the contained reader, if the reader needs updating then so does this
        class.
        """
        needs_update = super(ExodusSource, self).needsUpdate()
        mooseutils.mooseDebug('ExodusSource.needsUpdate() = {}'.format(needs_update))
        return needs_update or self.__reader.needsUpdate()

    def update(self, **kwargs):
        """
        Updates this object based on the reader and specified options.

        Inputs:
            see ChiggerSource
        """

        # Update the options, but do not call update from base class. See comment at end of this
        # method.
        self.setOptions(**kwargs)

        # Update the reader, if needed
        if self.__reader.needsUpdate():
            self.__reader.update()

        # Enable all blocks (subdomains) if nothing is enabled
        block_info = self.__reader.getBlockInformation()
        for item in ['block', 'boundary', 'nodeset']:
            if self.isOptionValid(item) and self.getOption(item) == []:
                self.setOption(item, [item.name for item in \
                                      block_info[getattr(ExodusReader, item.upper())].values()])
        self.setNeedsUpdate(False) # this function does not need to update again

        def get_indices(option, vtk_type):
            """
            Helper to populate vtkExtractBlock object from the selected blocks/sidesets/nodesets
            """
            indices = []
            if self.isOptionValid(option):
                blocks = self.getOption(option)
                for vtkid, item in block_info[vtk_type].items():
                    for name in blocks:
                        if (item.name == str(name)) or (str(name) == vtkid):
                            indices.append(item.multiblock_index)
            return indices
        extract_indices = get_indices('block', ExodusReader.BLOCK)
        extract_indices += get_indices('boundary', ExodusReader.SIDESET)
        extract_indices += get_indices('nodeset', ExodusReader.NODESET)

        if self.__extract_indices != extract_indices:
            self.__vtkextractblock.RemoveAllIndices()
            for index in extract_indices:
                self.__vtkextractblock.AddIndex(index)
            self.__extract_indices = extract_indices
        self.__vtkextractblock.Update()

        # Define the coloring to utilize for the object
        self.__updateVariable()

        # Representation
        if self.isOptionValid('representation'):
            func = 'SetRepresentationTo{}'.format(self.getOption('representation').title())
            attr = getattr(self._vtkactor.GetProperty(), func)
            attr()

        # Without this the results can be less than ideal
        # (https://blog.kitware.com/what-is-interpolatescalarsbeforemapping-in-vtk/)
        self._vtkmapper.InterpolateScalarsBeforeMappingOn()

        # Call the base class method, this is done last because the reader and block extracting
        # needs to be in place before the filters can be applied
        super(ExodusSource, self).update()

    def __updateVariable(self):
        """
        Method to update the active variable to display on the object. (private)
        """
        def get_available_variables():
            """
            Returns a sting listing the available nodal and elemental variable names.
            """
            nvars = self.__reader.getVariableInformation(var_types=[ExodusReader.NODAL]).keys()
            evars = self.__reader.getVariableInformation(var_types=[ExodusReader.ELEMENTAL]).keys()
            msg = ["Nodal:"]
            msg += [" " + var for var in nvars]
            msg += ["\nElemental:"]
            msg += [" " + var for var in evars]
            return ''.join(msg)

        # Define the active variable name
        available = self.__reader.getVariableInformation(var_types=[ExodusReader.NODAL,
                                                                    ExodusReader.ELEMENTAL])

        # Case when no variable exists
        if not available:
            return

        default = available[list(available.keys())[0]]
        if not self.isOptionValid('variable'):
            varinfo = default
        else:
            var_name = self.getOption('variable')
            if var_name not in available:
                msg = "The variable '{}' provided does not exist, using '{}', available " \
                      "variables include:\n{}"
                mooseutils.mooseError(msg.format(var_name, default.name, get_available_variables()))
                varinfo = default
            else:
                varinfo = available[var_name]

        # Update vtkMapper to the correct data mode
        if varinfo.object_type == ExodusReader.ELEMENTAL:
            self._vtkmapper.SetScalarModeToUseCellFieldData()
        elif varinfo.object_type == ExodusReader.NODAL:
            self._vtkmapper.SetScalarModeToUsePointFieldData()
        else:
            raise mooseutils.MooseException('Unknown variable type, not sure how you made it here.')
        self.__current_variable = varinfo

        # Colormap
        if not self.isOptionValid('color'):
            self._colormap.setOptions(cmap=self.getOption('cmap'),
                                      cmap_reverse=self.getOption('cmap_reverse'),
                                      cmap_num_colors=self.getOption('cmap_num_colors'))
            self._vtkmapper.SelectColorArray(varinfo.name)
            self._vtkmapper.SetLookupTable(self._colormap())
            self._vtkmapper.UseLookupTableScalarRangeOff()

        # Component
        component = -1 # Default component to utilize if not valid
        if self.isOptionValid('component'):
            component = self.getOption('component')

        if component == -1:
            self._vtkmapper.GetLookupTable().SetVectorModeToMagnitude()
        else:
            if component > varinfo.num_components:
                msg = 'Invalid component number ({}), the variable "{}" has {} components.'
                mooseutils.mooseError(msg.format(component, varinfo.name, varinfo.num_components))
            self._vtkmapper.GetLookupTable().SetVectorModeToComponent()
            self._vtkmapper.GetLookupTable().SetVectorComponent(component)

        # Range
        if (self.isOptionValid('min') or self.isOptionValid('max')) and self.isOptionValid('range'):
            mooseutils.mooseError('Both a "min" and/or "max" options has been set along with the '
                                  '"range" option, the "range" is being utilized, the others are '
                                  'ignored.')

        # Range
        rng = list(self.__getRange()) # Use range from all sources as the default
        if self.isOptionValid('range'):
            rng = self.getOption('range')
        else:
            if self.isOptionValid('min'):
                rng[0] = self.getOption('min')
            if self.isOptionValid('max'):
                rng[1] = self.getOption('max')

        if rng[0] > rng[1]:
            mooseutils.mooseDebug("Minimum range greater than maximum:", rng[0], ">", rng[1],
                                  ", the range/min/max settings are being ignored.")
            rng = list(self.__getRange())

        self.getVTKMapper().SetScalarRange(rng)

        # Handle Elemental variables that are not everywhere on the domain
        varname = self.__current_variable.name
        block = self.getOption('block')
        if (self.__current_variable.object_type == ExodusReader.ELEMENTAL) and (block is not None):
            for i in range(self.__vtkextractblock.GetOutput().GetNumberOfBlocks()):
                if not hasattr(self.__vtkextractblock.GetOutput().GetBlock(i), 'GetNumberOfBlocks'):
                    continue
                for j in range(self.__vtkextractblock.GetOutput().GetBlock(i).GetNumberOfBlocks()):
                    blk = self.__vtkextractblock.GetOutput().GetBlock(i).GetBlock(j)
                    if not blk.GetCellData().HasArray(varname):
                        data = vtk.vtkDoubleArray()
                        data.SetName(varname)
                        data.SetNumberOfTuples(blk.GetCellData().GetArray(0).GetNumberOfTuples())
                        data.FillComponent(0, vtk.vtkMath.Nan())
                        blk.GetCellData().AddArray(data)
