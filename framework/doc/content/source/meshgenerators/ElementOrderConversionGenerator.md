# ElementOrderConversionGenerator

!syntax description /Mesh/ElementOrderConversionGenerator

## Overview

The `ElementOrderConversionGenerator` converts all the elements of the [!param](/Mesh/ElementOrderConversionGenerator/input) mesh into the element types that are specified by an order category parameter, [!param](/Mesh/ElementOrderConversionGenerator/conversion_type):

- `FIRST_ORDER`: Converts the input mesh with second-order and/or higher-order elements into a mesh with only first-order elements. 
- `SECOND_ORDER_NONFULL`: Converts the input mesh with first-order elements into a mesh with second-order elements. For those first-order element types with two second-order equivalents, this option only converts the elements to the "non-full-ordered" element types. Note that the original "full-ordered" second-order or higher-order elements in the input mesh are retained without conversion (i.e., no downgrades).
- `SECOND_ORDER`: Converts the input mesh with first-order elements and/or "non-full-ordered" second-order elements into a mesh with "full-ordered" second-order elements.
- `COMPLETE_ORDER`: Converts the input mesh with first-order elements and/or second-order elements into a mesh with "complete" order elements.

The element type conversion maps of different [!param](/Mesh/ElementOrderConversionGenerator/conversion_type) options are summarized in [conversion_map_1d], [conversion_map_2d], and [conversion_map_3d]

!table id=conversion_map_1d caption=1D Element Type Conversion Maps of Different [!param](/Mesh/ElementOrderConversionGenerator/conversion_type) Options 
|   | EDGE2 | EDGE3 |
| - | - | - |
| `FIRST_ORDER` | EDGE2 | EDGE2 |
| `SECOND_ORDER_NONFULL` | EDGE3 | EDGE3 |
| `SECOND_ORDER` | EDGE3 | EDGE3 |
| `COMPLETE_ORDER` | EDGE3 | EDGE3 |

!table id=conversion_map_2d caption=2D Element Type Conversion Maps of Different [!param](/Mesh/ElementOrderConversionGenerator/conversion_type) Options 
|   | TRI3 | TRI6 | TRI7 | QUAD4 | QUAD8 | QUAD9 |
| - | - | - | - | - | - | - |
| `FIRST_ORDER` | TRI3 | TRI3 | TRI3 | QUAD4 | QUAD4 | QUAD4 |
| `SECOND_ORDER_NONFULL` | TRI6 | TRI6 | TRI7 | QUAD8 | QUAD8 | QUAD9 |
| `SECOND_ORDER` | TRI6 | TRI6 | TRI7 | QUAD9 | QUAD9 | QUAD9 |
| `COMPLETE_ORDER` | TRI7 | TRI7 | TRI7 | QUAD9 | QUAD9 | QUAD9 |

!table id=conversion_map_3d caption=3D Element Type Conversion Maps of Different [!param](/Mesh/ElementOrderConversionGenerator/conversion_type) Options 
|   | TET4 | TET10 | TET14 | HEX8 | HEX20 | HEX27 | PRISM6 | PRISM15 | PRISM18 | PRISM20 | PRISM21 | PYRAMID5 | PYRAMID13 | PYRAMID14 | PYRAMID18 |
| - | - | - | - | - | - | - | - | - | - | - | - | - | - | - | - |
| `FIRST_ORDER` | TET4 | TET4 | TET4 | HEX8 | HEX8 | HEX8 | PRISM6 | PRISM6 | PRISM6 | PRISM6 | PRISM6 | PYRAMID5 | PYRAMID5 | PYRAMID5 | PYRAMID5 |
| `SECOND_ORDER_NONFULL` | TET10 | TET10 | TET14 | HEX20 | HEX20 | HEX27 | PRISM15 | PRISM15 | PRISM18 | PRISM20 | PRISM21 | PYRAMID13 | PYRAMID13 | PYRAMID14 | PYRAMID18 |
| `SECOND_ORDER` | TET10 | TET10 | TET14 | HEX27 | HEX27 | HEX27 | PRISM18 | PRISM18 | PRISM18 | PRISM20 | PRISM21 | PYRAMID14 | PYRAMID14 | PYRAMID14 | PYRAMID18 |
| `COMPLETE_ORDER` | TET14 | TET14 | TET14 | HEX27 | HEX27 | HEX27 | PRISM21 | PRISM21 | PRISM21 | PRISM21 | PRISM21 | PYRAMID18 | PYRAMID18 | PYRAMID18 | PYRAMID18 |

!syntax parameters /Mesh/ElementOrderConversionGenerator

!syntax inputs /Mesh/ElementOrderConversionGenerator

!syntax children /Mesh/ElementOrderConversionGenerator
