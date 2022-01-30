# HeatTransferFromHeatStructure3D1Phase

This component couples multiple single-phase flow channels with a 3D heat structure.

## Usage

The flow channels must be aligned with one of the axes (x, y or z). The number of elements of the heat structure
in the direction of the flow channels must match the number of elements of the flow channels, and the axial locations of the element centroids must match.
The user should check the flow channel and heat structure elements are properly aligned to ensure the coupling is properly executed. No internal check is performed.

The following parameters need to be specified:

- `flow_channel`: List of single-phase flow channel names
- `hs`: Name of the 3D heat structure
- `boundary`: Name of the heat structure boundary that is coupled to the flow channels
- `P_hf`: Heated perimeter. To ensure energy conservation, the heated perimeter should be calculated on the discretized heat structure boundary.


!syntax parameters /Components/HeatTransferFromHeatStructure3D1Phase

!syntax inputs /Components/HeatTransferFromHeatStructure3D1Phase

!syntax children /Components/HeatTransferFromHeatStructure3D1Phase
