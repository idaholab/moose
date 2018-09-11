#!/usr/bin/env python2
import vtk

string = "$\\hat{H}\\psi = \\left(-\\frac{\\hbar}{2m}\\nabla^2 + V(r)\\right) \\psi = \\psi\\cdot E $";

image = vtk.vtkImageData()
utils = vtk.vtkMathTextUtilities()
utils.SetScaleToPowerOfTwo(False)

tprop = vtk.vtkTextProperty()
tprop.SetColor(1, 1, 1)
tprop.SetFontSize(50)


viewer = vtk.vtkImageViewer2()
utils.RenderString(string, image, tprop, viewer.GetRenderWindow().GetDPI())
viewer.SetInputData(image)

iren = vtk.vtkRenderWindowInteractor()
viewer.SetupInteractor(iren)

viewer.Render();
viewer.GetRenderWindow().GetInteractor().Initialize()
viewer.GetRenderWindow().GetInteractor().Start() # Text Mapper
