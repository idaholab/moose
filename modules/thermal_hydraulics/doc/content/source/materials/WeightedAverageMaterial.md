# WeightedAverageMaterial

!syntax description /Materials/WeightedAverageMaterial

The output material property $p$ is equal to:

!equation
p = \dfrac{\sum_i v_i m_i}{\sum_i v_i}

where $v_i$ are the variables passed in the [!param](/Materials/WeightedAverageMaterial/weights) parameter and
$m_i$ are the material properties passed in the [!param](/Materials/WeightedAverageMaterial/values) parameter.

!syntax parameters /Materials/WeightedAverageMaterial

!syntax inputs /Materials/WeightedAverageMaterial

!syntax children /Materials/WeightedAverageMaterial
