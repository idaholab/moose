#This makes sure that each app only sees the includes it needs - even through multiple levels

# Save off app_INCLUDES
backup_app_INCLUDES := $(app_INCLUDES)

# Clear it
app_INCLUDES :=
