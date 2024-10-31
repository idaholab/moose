# NEML2
ADDITIONAL_NEML2_DIRS += $(APPLICATION_DIR)/test/neml2
ADDITIONAL_NEML2_DIRS += $(APPLICATION_DIR)/neml2
# needed for J2FlowDirection
ADDITIONAL_NEML2_DIRS += $(APPLICATION_DIR)/contrib/neml2/tests
include $(APPLICATION_DIR)/contrib/neml2.mk

# Depth of subfolders used to create unity groups
app_unity_depth = 2
