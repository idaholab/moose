#!/usr/bin/env python3
import vtk

def get_program_parameters():
    import argparse
    description = 'Read and display ExodusII data from multiple files.'
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('filenames', nargs='+', help="A required filename pattern, e.g, mug_*.e.")
    parser.add_argument('--variable', type=str, help="The nodal variable, e.g. 'convected'.")
    parser.add_argument('--timestep', type=int, help="The timestep to load.")
    args = parser.parse_args()
    return args

def main():
    colors = vtk.vtkNamedColors()

    # Input files and variable
    args = get_program_parameters()

    # Read Exodus Data
    blocks = vtk.vtkMultiBlockDataSet()
    readers = list()
    for i, filename in enumerate(args.filenames):
        reader = vtk.vtkExodusIIReader()
        reader.SetFileName(filename)
        reader.UpdateInformation()
        reader.SetTimeStep(args.timestep)
        reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)  # enables all NODAL variables
        reader.Update()
        blocks.SetBlock(i, reader.GetOutput())
        readers.append(reader)

    extract = vtk.vtkExtractBlock()
    extract.AddIndex(0)
    extract.SetInputData(blocks)

    geometry = vtk.vtkCompositeDataGeometryFilter()
    geometry.SetInputConnection(0, extract.GetOutputPort(0))
    geometry.Update()

    # Mapper
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(geometry.GetOutputPort())
    mapper.SelectColorArray(args.variable)
    mapper.SetScalarModeToUsePointFieldData()
    mapper.InterpolateScalarsBeforeMappingOn()

    # Actor
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)

    # Renderer
    renderer = vtk.vtkRenderer()
    renderer.AddViewProp(actor)
    renderer.SetBackground(colors.GetColor3d('DimGray'))

    # Window and Interactor
    window = vtk.vtkRenderWindow()
    window.AddRenderer(renderer)
    window.SetSize(600, 600)
    window.SetWindowName('ReadExodusData')

    interactor = vtk.vtkRenderWindowInteractor()
    interactor.SetRenderWindow(window)
    interactor.Initialize()

    # Show the result
    window.Render()

    recorder = vtk.vtkInteractorEventRecorder()
    recorder.SetInteractor(interactor)
    recorder.EnabledOn()
    recorder.SetFileName('record.log')
    recorder.Record()
    print(recorder.GetEnabled())


    interactor.Start()

if __name__ == '__main__':
    main()
