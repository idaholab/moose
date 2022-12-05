# Saline
SALINE_DIR ?= $(MOOSE_DIR)/modules/fluid_properties/contrib/saline
SALINE_SRC = $(SALINE_DIR)/src
ifneq ($(wildcard $(SALINE_SRC)/cpp/data_store.cc),)
	ADDITIONAL_CPPFLAGS += -DSALINE_ENABLED
	include $(SALINE_DIR)/../saline.mk
endif
