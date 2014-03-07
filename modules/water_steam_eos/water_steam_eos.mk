#
# WATER_STEAM_EOS

# F90 module dependency rules
$(APPLICATION_DIR)/src/water_steam_phase_prop.f90: $(APPLICATION_DIR)/src/IAPWS97.$(obj-suffix)
$(APPLICATION_DIR)/src/water_steam_functions.f90: $(APPLICATION_DIR)/src/IAPWS97.$(obj-suffix)
