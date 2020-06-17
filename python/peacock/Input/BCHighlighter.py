#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

def highlightBlock(block, vtkwindow):
    """
    Input:
        block[BlockInfo]: This block will be a child of /BCs
        vtkwindow[VTKWindowPlugin]: The vtk window to set the highlights on
    """
    if not vtkwindow.isVisible() or not vtkwindow.isEnabled():
        return

    if not block.path.startswith("/BCs/"):
        vtkwindow.onHighlight()
        return

    boundary_param = None
    block_param = None
    if block.getParamInfo("boundary"):
        boundary_param = block.getParamInfo("boundary").value.split()

    if block.getParamInfo("block"):
        block_param = block.getParamInfo("block").value.split()

    primary = block.getParamInfo("primary")
    if boundary_param or block_param:
        vtkwindow.onHighlight(boundary=boundary_param, block=block_param)
    elif primary:
        secondary = block.getParamInfo("secondary")
        if secondary:
            vtkwindow.onHighlight(boundary=[primary.value, secondary.value])
        else:
            vtkwindow.onHighlight()
    else:
        vtkwindow.onHighlight()
