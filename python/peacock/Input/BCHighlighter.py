
def highlightBlock(block, vtkwindow):
    """
    Input:
        block[BlockInfo]: This block will be a child of /BCs
        vtkwindow[VTKWindowPlugin]: The vtk window to set the highlights on
    """
    if not vtkwindow.isVisible():
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

    master = block.getParamInfo("master")
    if boundary_param or block_param:
        vtkwindow.onHighlight(boundary=boundary_param, block=block_param)
    elif master:
        slave = block.getParamInfo("slave")
        if slave:
            vtkwindow.onHighlight(boundary=[master.value, slave.value])
        else:
            vtkwindow.onHighlight()
    else:
        vtkwindow.onHighlight()
