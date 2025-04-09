# * This file is part of the MOOSE framework
# * https://www.mooseframework.org
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html

import json
import numpy as np

# This python script creates a dummy LAROMANCE 6D interpolation grid for testing


# Six dimensional interpolation grid.
node_dict = {
    "in_stress": [-1.0, -0.8, 0.3, 0, 0.5, 0.6, 1],
    "in_temperature": [
        -1.0,
        -0.9,
        -0.8,
        -0.7,
        -0.6,
        -0.5,
        -0.4,
        -0.3,
        -0.2,
        -0.1,
        0,
        0.1,
        0.3,
        0.5,
        0.7,
        1.0,
    ],
    "in_plastic_strain": [-1.0, -0.5, -0.1, 1.0],
    "in_cell": [0, 0.1, 0.5, 0.1],
    "in_wall": [-1.0, -0.5, -0.1, 0.1, 0.5, 1.0],
    "in_env": [-1.0, -0.5, 0.5, 1.0],
}

dimensions = [
    len(node_dict[key])
    for key in [
        "in_stress",
        "in_temperature",
        "in_plastic_strain",
        "in_cell",
        "in_wall",
        "in_env",
    ]
]

# Making up dummy grid point values for output dimensions.
np.random.seed(0)
out_cell_grid = np.random.uniform(1.801, 1.8002, dimensions)
out_wall_grid = np.random.uniform(1.801, 1.8002, dimensions)
out_ep_grid = np.random.uniform(0.351, 0.352, dimensions)

node_dict["out_cell"] = out_cell_grid.tolist()
node_dict["out_wall"] = out_wall_grid.tolist()
node_dict["out_ep"] = out_ep_grid.tolist()

# definitning input dimension transforms and parameters
node_dict["in_stress_transform_type"] = "MINMAX"
node_dict["in_temperature_transform_type"] = "MINMAX"
node_dict["in_plastic_strain_transform_type"] = "LOG10BOUNDED"
node_dict["in_cell_transform_type"] = "COMPRESS"
node_dict["in_wall_transform_type"] = "MINMAX"
node_dict["in_env_transform_type"] = "MINMAX"

node_dict["in_stress_transform_values"] = [
    1e-8,
    300,
    -1.0,
    1.0,
]
node_dict["in_temperature_transform_values"] = [5e2, 9e2, -1.0, 1.0]
node_dict["in_plastic_strain_transform_values"] = [
    1.0e-22,
    0.0,
    1.0,
    -20.0,
    -0.5,
]
node_dict["in_cell_transform_values"] = [2e-4, 2e-2, 2]
node_dict["in_wall_transform_values"] = [
    1e8,
    1e13,
    -1.0,
    1.0,
]
node_dict["in_env_transform_values"] = [
    1e15,
    5e15,
    -1.0,
    1.0,
]

# Defining output dimension transforms and parameters
node_dict["out_cell_rate_transform_type"] = "DECOMPRESS"
node_dict["out_wall_rate_transform_type"] = "DECOMPRESS"
node_dict["out_strain_rate_transform_type"] = "EXP10BOUNDED"

node_dict["out_cell_rate_transform_values"] = [1.0e-8, 0.3, -60.0]
node_dict["out_wall_rate_transform_values"] = [1.0e-8, 0.3, -60.0]
node_dict["out_strain_rate_transform_values"] = [0.0, 0.0, 1.0, -12.0, 5.0]

# Writing to JSON
with open("random_value_6d_grid.json", "w") as fp:
    json.dump(node_dict, fp)
