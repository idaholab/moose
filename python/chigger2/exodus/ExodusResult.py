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
#from ExodusSource import ExodusSource
from ExodusReader import ExodusReader
from MultiAppExodusReader import MultiAppExodusReader
from moosetools import mooseutils
from .. import base
from .. import utils
from .. import filters

#TODO: MOVE TO ExodusSource
#@base.addFilter(filters.GeometryFilter, required=True)
#@base.addFilter(filters.ExtractBlockFilter)
class ExodusResult(base.ChiggerResult):
    """
    Result object to displaying ExodusII data from a single reader.
    """
    VTKACTORTYPE = vtk.vtkActor
    VTKMAPPERTYPE = vtk.vtkPolyDataMapper
    INPUTTYPE = 'vtkMultiBlockDataSet'

    @staticmethod
    def validParams():
        opt = base.ChiggerResult.validParams()

        # Variable selection
        opt.add('variable', vtype=str, doc="The nodal or elemental variable to render.")
        opt.add('component', -1, vtype=int, allow=(-1, 0, 1, 2),
                doc="The vector variable component to render (-1 plots the magnitude).")

        # Data range
        opt.add('lim', vtype=(int, float), size=2,
                doc="The range of data to display on the volume and colorbar; range takes " \
                    "precedence of min/max.")
        opt.add('local_lim', False, "Use local range when computing the default data range.")
        opt.add('min', vtype=(int, float), doc="The minimum range.")
        opt.add('max', vtype=(int, float), doc="The maximum range.")

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

        # Colormap
        opt += base.ColorMap.validParams()

        opt.add('explode', None, vtype=(int, float),
                doc="When multiple sources are being used (e.g., NemesisReader) setting this to a "
                    "value will cause the various sources to be 'exploded' away from the center of "
                    "the entire object.")
        return opt

    @staticmethod
    def validKeyBindings():

        bindings = base.ChiggerResult.validKeyBindings()

        # Opacity keybindings
        bindings.add('a', ExodusResult._updateOpacity,
                     desc='Increase the alpha (opacity) by 1%.')
        bindings.add('a', ExodusResult._updateOpacity, shift=True,
                     desc='Decrease the alpha (opacity) by 1%.')

        # Colormap keybindings
        bindings.add('m', ExodusResult._updateColorMap,
                     desc="Toggle through available colormaps.")
        bindings.add('m', ExodusResult._updateColorMap, shift=True,
                     desc="Toggle through available colormaps, in reverse direction.")

        # Time keybindngs
        bindings.add('t', ExodusResult._updateTimestep,
                     desc="Increase timestep by 1.")
        bindings.add('t', ExodusResult._updateTimestep, shift=True,
                     desc="Decrease the timestep by 1.")

        return bindings

    def __init__(self, *args, **kwargs):

        self.__current_variable = None
        self._colormap = base.ColorMap()

        base.ChiggerResult.__init__(self, *args, **kwargs)

    def applyParams(self):

        self.__ACTIVE_FILTERS__.add('extract')
        base.ChiggerResult.applyParams(self)

        # Update the block/boundary/nodeset settings to convert [] to all.
        block_info = self._inputs[0].getBlockInformation()
        for item in ['block', 'boundary', 'nodeset']:
            opt = self.getParam(item)
            if opt == []:
                self.setParam(item, [item.name for item in \
                                      block_info[getattr(ExodusReader, item.upper())].itervalues()])

        # Update the extract indices
        def get_indices(option, vtk_type):
            """Helper to populate vtkExtractBlock object from selected blocks/sidesets/nodesets"""
            indices = []
            blocks = self.getParam(option)
            if blocks:
                for vtkid, item in block_info[vtk_type].items():
                    for name in blocks:
                        if (item.name == str(name)) or (str(name) == vtkid):
                            indices.append(item.multiblock_index)
            return indices

        extract_indices = get_indices('block', ExodusReader.BLOCK)
        extract_indices += get_indices('boundary', ExodusReader.SIDESET)
        extract_indices += get_indices('nodeset', ExodusReader.NODESET)

        for filters in self._filters:
            for fobject in filters:
                if fobject.FILTERNAME == 'extract':
                    fobject.setParam('indices', extract_indices)

        for vtkmapper in self._vtkmappers:
            vtkmapper.SetScalarModeToUsePointFieldData()
            vtkmapper.InterpolateScalarsBeforeMappingOn()


        self.__updateVariable()

    #def __init__(self, reader, **kwargs):
    #    base.ChiggerResult(reader, **kwargs)

    #    self._reader = reader

    #    # Build the ExodusSource objects
    #    if isinstance(reader, ExodusReader):
    #        sources = [ExodusSource(reader)]
    #    elif isinstance(reader, MultiAppExodusReader):
    #        sources = [ExodusSource(r) for r in reader]
    #    else:
    #        raise mooseutils.MooseException('The supplied object of type {} is not supported, '
    #                                        'only ExodusReader and MultiAppExodusReader objects '
    #                                        'may be utilized.'.format(reader.__class__.__name__))

    #    # Supply the sources to the base class
    #    #super(ExodusResult, self).__init__(*sources, **kwargs)

    #    self.__outline_result = None

    """
    def update(self, **kwargs):
        super(ExodusResult, self).update(**kwargs)

        # Update the ExodusReader objects
        self._reader.update()

        # Do not mess with the range if there is a source without a variable
        if any([src.getCurrentVariableInformation() is None for src in self._sources]):
            return

        # Re-compute ranges for all sources
        rng = list(self.getRange(local=self.getParam('local_range')))
        if self.isValid('lim'):
            rng = self.getParam('lim')
        else:
            if self.isValid('min'):
                rng[0] = self.getParam('min')
            if self.isValid('max'):
                rng[1] = self.getParam('max')

        if rng[0] > rng[1]:
            mooseutils.mooseDebug("Minimum range greater than maximum:", rng[0], ">", rng[1],
                                  ", the range/min/max settings are being ignored.")
            rng = list(self.getRange())

        for src in self._sources:
            src.getVTKMapper().SetScalarRange(rng)

        # Explode
        if self.isValid('explode'):
            factor = self.applyOption('explode')
            m = self.getCenter()
            for src in self._sources:
                c = src.getVTKActor().GetCenter()
                d = (c[0]-m[0], c[1]-m[1], c[2]-m[2])
                src.getVTKActor().AddPosition(d[0]*factor, d[1]*factor, d[2]*factor)
    """
    #def getRange(self, **kwargs):
    #    """
    #    Return the min/max range for the selected variables and blocks/boundary/nodeset.

    #    NOTE: For the range to be restricted by block/boundary/nodest the reader must have
    #          "squeeze=True", which can be much slower.
    #    """
    #    rngs = [src.getRange(**kwargs) for src in self._sources]
    #    return utils.get_min_max(*rngs)

    #def getCenter(self):
    #    """
    #    Return the center (based on the bounds) of all the objects.
    #    """
    #    b = self.getBounds()
    #    return ((b[1]-b[0])/2., (b[3]-b[2])/2., (b[5]-b[4])/2.)


    def __updateVariable(self):
        """
        Method to update the active variable to display on the object. (private)
        """
        def get_available_variables():
            """
            Returns a sting listing the available nodal and elemental variable names.
            """
            nvars = self._inputs[0].getVariableInformation(var_types=[ExodusReader.NODAL]).keys()
            evars = self._inputs[0].getVariableInformation(var_types=[ExodusReader.ELEMENTAL]).keys()

            msg = []
            if nvars:
                msg += ["Nodal:"]
                msg += [" " + var for var in nvars]
            if evars:
                msg += ["\nElemental:"]
                msg += [" " + var for var in evars]
            return ''.join(msg)

        # Define the active variable name
        available = self._inputs[0].getVariableInformation(var_types=[ExodusReader.NODAL,
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
                mooseutils.mooseWarning(msg.format(var_name, default.name, get_available_variables()))
                varinfo = default
            else:
                varinfo = available[var_name]

        # Update vtkMapper to the correct data mode
        for vtkmapper in self._vtkmappers:
            if varinfo.object_type == ExodusReader.ELEMENTAL:
                vtkmapper.SetScalarModeToUseCellFieldData()
            elif varinfo.object_type == ExodusReader.NODAL:
                vtkmapper.SetScalarModeToUsePointFieldData()
            else:
                raise mooseutils.MooseException('Unknown variable type, not sure how you made it here.')

        self.__current_variable = varinfo

        # Colormap
        if not self.getParam('color'):
            self._colormap.setParams(cmap=self.getParam('cmap'),
                                      cmap_reverse=self.getParam('cmap_reverse'),
                                      cmap_num_colors=self.getParam('cmap_num_colors'))
            for vtkmapper in self._vtkmappers:
                vtkmapper.SelectColorArray(varinfo.name)
                vtkmapper.SetLookupTable(self._colormap())
                vtkmapper.UseLookupTableScalarRangeOff()

        # Component
        component = -1 # Default component to utilize if not valid
        if self.isValid('component'):
            component = self.getParam('component')

        for vtkmapper in self._vtkmappers:
            if component == -1:
                vtkmapper.GetLookupTable().SetVectorModeToMagnitude()
            else:
                if component > varinfo.num_components:
                    msg = 'Invalid component number ({}), the variable "{}" has {} components.'
                    mooseutils.mooseError(msg.format(component, varinfo.name, varinfo.num_components))
                vtkmapper.GetLookupTable().SetVectorModeToComponent()
                vtkmapper.GetLookupTable().SetVectorComponent(component)

        # Range
        if (self.isValid('min') or self.isValid('max')) and self.isValid('lim'):
            mooseutils.mooseError('Both a "min" and/or "max" options has been set along with the '
                                  '"range" option, the "range" is being utilized, the others are '
                                  'ignored.')

        # Range
        rng = list(self.getRange(local=self.getParam('local_lim')))
        if self.isValid('lim'):
            rng = self.getParam('lim')
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

    def getRange(self, local=False):
        """
        Return range of the active variable and blocks.
        """
        for reader in self._inputs:
            reader.Update()

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
        for filters in self._filters:
            filter_obj = None
            for f in filters:
                if f.FILTERNAME == 'extract':
                    filter_obj = f
                    break

            filter_obj.Update() # required to get correct ranges from ExtractBlockFilter
            data = filter_obj.GetOutputDataObject(0)
            for i in range(data.GetNumberOfBlocks()):
                current = data.GetBlock(i)
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
        pairs = []
        for vtkmapper in self._vtkmappers:
            vtkmapper.Update() # required to have up-to-date ranges
            data = vtkmapper.GetInput()
            out = self.__getActiveArray(data)
            if out is not None:
                pairs.append(out.GetRange(component))

        return utils.get_min_max(*pairs)

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

    def _updateOpacity(self, window, binding): #pylint: disable=unuysed-argument
        opacity = self.getParam('opacity')
        if binding.shif
            if opacity > 0.05:
                opacity -= 0.05
        else:
            if opacity <= 0.95:
                opacity += 0.05
        self.update(opacity=opacity)
        self.printOption('opacity')

    def _updateColorMap(self, window, binding): #pylint: disable=unused-argument
        step = 1 if not binding.shift else -1
        available = self._sources[0]._colormap.names() #pylint: disable=protected-access
        index = available.index(self.getParam('cmap'))

        n = len(available)
        index += step
        if index == n:
            index = 0
        elif index < 0:
            index = n - 1

        self.setParam('cmap', available[index])
        self.printOption('cmap')

    def _updateTimestep(self, window, binding): #pylint: disable=unused-argument
        step = 1 if not binding.shift else -1
        current = self._reader.getTimeData().timestep + step
        n = len(self._reader.getTimes())
        if current == n:
            current = 0
        self._reader.setParam('time', None)
        self._reader.setParam('timestep', current)
        self._reader.printOption('timestep')
