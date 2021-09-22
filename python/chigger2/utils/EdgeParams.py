from .ChiggerInputParameters import ChiggerInputParameters

def validParams():
    """Returns options for edge properties for vtkActor objects."""
    opt = ChiggerInputParameters()

    opt.add('orientation', vtype=(int, float), size=3, doc="The orientation of the object.")
    opt.add('rotation', default=(0., 0., 0.), vtype=(int, float), size=3,
            doc="The rotation of the object about x, y, z axes.")
    opt.add('visible', default=False, doc="Enable/disable display of object edges.")
    opt.add('color', default=(1., 1., 1.), size=3, doc="Set the edge color.")
    opt.add('width', default=1, vtype=int, doc="The edge width, if None then no edges are shown.")
    opt.add('size', default=1, vtype=int, doc="The point size, if None then no points are shown.")
    return opt

def applyParams(vtkactor, opt):

    """
    if opt.isValid('orientation'):
        vtkactor.SetOrientation(opt.get('orientation'))

    x, y, z = opt.get('rotation')
    vtkactor.RotateX(x)
    vtkactor.RotateY(y)
    vtkactor.RotateZ(z)
    """

    vtkactor.GetProperty().SetEdgeVisibility(opt.getValue('visible'))
    vtkactor.GetProperty().SetEdgeColor(opt.getValue('color'))
    vtkactor.GetProperty().SetLineWidth(opt.getValue('width'))
    vtkactor.GetProperty().SetPointSize(opt.getValue('size'))
