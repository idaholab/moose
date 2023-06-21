# AverageWallTemperature3EqnMaterial

!syntax description /Materials/AverageWallTemperature3EqnMaterial

This material enables defining a wall temperature with multiple heat sources and/or heat transfer coefficient
definitions.

!equation
T_w = T_{fluid} + \dfrac{\sum_i (T_{\text{wall source i}} - T_{fluid} h_{\text{source i}} P_{\text{source i}}}{h_{average}P_{\text{heated sources}}}

with $T_w$ the average wall temperature, $T_{fluid}$ the fluid temperature, $T_{\text{wall source i}}$ the
wall temperature of a heat source, $h_{\text{source i}}$ the heat transfer coefficient associated with the wall
for that heat source, $P_{\text{source i}}$ the heated perimeter for that heat source, $h_{average}$
the average heat transfer coefficient
and $P_{\text{heated sources}}$ the total heated perimeter for all source.

For a zero heat transfer coefficient (abnormal situation), the average wall temperature instead falls back to:

!equation
T_w = \dfrac{\sum_i T_{\text{wall source i}} P_{\text{source i}}}{P_{\text{heated sources}}}

!syntax parameters /Materials/AverageWallTemperature3EqnMaterial

!syntax inputs /Materials/AverageWallTemperature3EqnMaterial

!syntax children /Materials/AverageWallTemperature3EqnMaterial
