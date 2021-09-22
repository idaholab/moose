import vtk
from .ChiggerInputParameters import ChiggerInputParameters

def validParams(actor_type=vtk.vtkActor):
    """Returns options for edge properties for vtkActor objects."""
    opt = ChiggerInputParameters()
    return opt

def applyParams(vtkactor, opt):

    pass
