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
from ExodusReader import ExodusReader
from moosetools import mooseutils
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
    #FILTER_TYPES = [filters.ContourFilter,
    #                filters.ClipperFilterBase,
    #                filters.GeometryFilter,
    #                filters.TransformFilter,
    #                filters.TubeFilter,
    #                filters.RotationalExtrusionFilter,
    #                filters.OutlineFilter]

    @staticmethod
    def validParams():
        opt = base.ChiggerSource.validParams()

        # Variable
        opt.add('variable', vtype=str, doc="The nodal or elemental variable to render.")
        opt.add('component', -1, vtype=int, allow=(-1, 0, 1, 2),
                doc="The vector variable component to render (-1 plots the magnitude).")

        # Subdomains/sidesets/nodesets
        opt.add('nodeset', None, vtype=list,
                doc="A list of nodeset ids or names to display, use [] to display all nodesets.")
        opt.add('boundary', None, vtype=list,
                doc="A list of boundary ids (sideset) ids or names to display, use [] to display " \
                    "all sidesets.")
        opt.add('block', [], vtype=list,
                doc="A list of subdomain (block) ids or names to display, use [] to display all " \
                    "blocks.")

        opt.add('representation', 'surface', allow=('volume', 'surface', 'wireframe', 'points'),
                doc="View volume representation.")

        opt.add('range', vtype=(int, float), size=2,
                doc="The range of data to display on the volume and colorbar; range takes " \
                    "precedence of min/max.")
        opt.add('min', vtype=(int, float), doc="The minimum range.")
        opt.add('max', vtype=(int, float), doc="The maximum range.")

        # Colormap
        opt += base.ColorMap.validParams()
        return opt

    def __init__(self, viewport, reader, **kwargs):
        super(ExodusSource, self).__init__(viewport, **kwargs)

        if not isinstance(reader, ExodusReader):
            raise mooseutils.MooseException('The supplied reader must be a '
                                            '"chigger.readers.ExodusReader", but a "{}" was '
                                            'provided.'.format(type(reader).__name__))

        self.__reader = reader
        self.__current_variable = None
        #self._colormap = base.ColorMap()

        self.__extract_indices = []
        self.__vtkextractblock = vtk.vtkExtractBlock()
        self.__vtkextractblock.SetInputConnection(self.__reader.GetOutputPort())

        #self._required_filters = [filters.GeometryFilter()]

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
        Returns the vtkExtractBlock object used for pulling subdomsin/sideset/nodeset data from the
        reader. (override)

        Returns:
            vtk.vtkExtractBlock (see ChiggerFilterSourceBase)
        """
        return self.__vtkextractblock

    def getBounds(self):
        """
        Return the extents of the active data objects.
        """
        bnds = []
        for i in range(self.__vtkextractblock.GetOutput().GetNumberOfBlocks()):
            current = self.__vtkextractblock.GetOutput().GetBlock(i)
            if isinstance(current, vtk.vtkCommonDataModelPython.vtkUnstructuredGrid):
                bnds.append(current.GetBounds())

            elif isinstance(current, vtk.vtkCommonDataModelPython.vtkMultiBlockDataSet):
                for j in range(current.GetNumberOfBlocks()):
                    bnds.append(current.GetBlock(j).GetBounds())

        return utils.get_vtk_bounds_min_max(*bnds)

    def getRange(self, local=False):
        """
        Return range of the active variable and blocks.
        """
        self.update()
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
        component = self.getParam('component')
        pairs = []
        for i in range(self.__vtkextractblock.GetOutput().GetNumberOfBlocks()):
            current = self.__vtkextractblock.GetOutput().GetBlock(i)
            if isinstance(current, vtk.vtkCommonDataModelPython.vtkUnstructuredGrid):
                array = self.__getActiveArray(current)
                if array:
                    pairs.append(array.GetRange(component))

            elif isinstance(current, vtk.vtkCommonDataModelPython.vtkMultiBlockDataSet):
                for j in range(current.GetNumberOfBlocks()):
                    array = self.__getActiveArray(current.GetBlock(j))
                    if array:
                        pairs.append(array.GetRange(component))

        return utils.get_min_max(*pairs)

    def __getLocalRange(self):
        """
        Determine the range of visible items.
        """
        component = self.getParam('component')
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

    def update(self, **kwargs):
        """
        Updates this object based on the reader and specified options.

        Inputs:
            see ChiggerSource
        """

        # Update the options, but do not call update from base class. See comment at end of this
        # method.
        self.setParams(**kwargs)

        # Update the reader
        self.__reader.update()

        # Enable all blocks (subdomains) if nothing is enabled
        block_info = self.__reader.getBlockInformation()
        for item in ['block', 'boundary', 'nodeset']:
            if self.isValid(item) and self.getParam(item) == []:
                self.setParam(item, [item.name for item in \
                                      block_info[getattr(ExodusReader, item.upper())].itervalues()])

        def get_indices(option, vtk_type):
            """
            Helper to populate vtkExtractBlock object from the selected blocks/sidesets/nodesets
            """
            indices = []
            if self.isValid(option):
                blocks = self.getParam(option)
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
        self.__vtkextractblock.SetInputConnection(self.__reader.getVTKReader().GetOutputPort())
        self.__vtkextractblock.Update()

        # Define the coloring to utilize for the object
        self.__updateVariable()

        # Representation
        if self.isValid('representation'):
            func = 'SetRepresentationTo{}'.format(self.getParam('representation').title())
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

        default = available[available.keys()[0]]
        if not self.isValid('variable'):
            varinfo = default
        else:
            var_name = self.getParam('variable')
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
        if not self.getParam('color'):
            self._colormap.setParams(cmap=self.getParam('cmap'),
                                      cmap_reverse=self.getParam('cmap_reverse'),
                                      cmap_num_colors=self.getParam('cmap_num_colors'))
            self._vtkmapper.SelectColorArray(varinfo.name)
            self._vtkmapper.SetLookupTable(self._colormap())
            self._vtkmapper.UseLookupTableScalarRangeOff()

        # Component
        component = -1 # Default component to utilize if not valid
        if self.isValid('component'):
            component = self.getParam('component')

        if component == -1:
            self._vtkmapper.GetLookupTable().SetVectorModeToMagnitude()
        else:
            if component > varinfo.num_components:
                msg = 'Invalid component number ({}), the variable "{}" has {} components.'
                mooseutils.mooseError(msg.format(component, varinfo.name, varinfo.num_components))
            self._vtkmapper.GetLookupTable().SetVectorModeToComponent()
            self._vtkmapper.GetLookupTable().SetVectorComponent(component)

        # Range
        if (self.isValid('min') or self.isValid('max')) and self.isValid('range'):
            mooseutils.mooseError('Both a "min" and/or "max" options has been set along with the '
                                  '"range" option, the "range" is being utilized, the others are '
                                  'ignored.')

        # Range
        rng = list(self.__getRange()) # Use range from all sources as the default
        if self.isValid('range'):
            rng = self.getParam('range')
        else:
            if self.isValid('min'):
                rng[0] = self.getParam('min')
            if self.isValid('max'):
                rng[1] = self.getParam('max')

        if rng[0] > rng[1]:
            mooseutils.mooseDebug("Minimum range greater than maximum:", rng[0], ">", rng[1],
                                  ", the range/min/max settings are being ignored.")
            rng = list(self.__getRange())

        self.getVTKMapper().SetScalarRange(rng)
