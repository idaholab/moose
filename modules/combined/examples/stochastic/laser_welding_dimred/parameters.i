# Process parameters
scanning_speed=1.0 # m/s
power=60 # W (this is the effective power so multiplied by eta)
R=1.65e-4 # m (this is the effective radius)

# Geometric parameters
thickness=0.9e-4 # m
xmin=-0.1e-3 # m
xmax=0.75e-3 # m
ymin=${fparse -thickness}
surfacetemp=300 # K (temperature at the other side of the plate)

# Time stepping parameters
endtime=4e-4 # s
timestep=${fparse endtime/1000} # s
