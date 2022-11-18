# Saline
SALINE_DIR ?= $(APPLICATION_DIR)/contrib/saline
SALINE_SRC = $(SALINE_DIR)/src
ifneq ($(wildcard $(SALINE_SRC)/cpp/data_store.cc),)
	ADDITIONAL_CPPFLAGS += -DSALINE_ENABLED
	include $(APPLICATION_DIR)/contrib/saline.mk
endif
