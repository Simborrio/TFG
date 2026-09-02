#
# Core Flight Software CMake / GNU make wrapper
#
# ABOUT THIS MAKEFILE:
# This defines the configuration targets for the multi-target build
# It is moved to a separate file to isolate this from the other logic
#
# This should reside in the same top-level directory as the main Makefile
#


# Trimmed to the one configuration this project actually builds/runs:
# native_std (the flight-segment Raspberry Pi target). Removed native_eds,
# pc686_rtems5, gr712_rtems5, rpi_vxworks7, rpi_linux, qemu_yocto_linux,
# osal, edslib - none are used by this mission.
CONFIG_NAMES := native_std

# The CFS config names is a subset of the CONFIG_NAMES which use CFS
# Being in this list means the standard set of CFS options are applied
NONCFS_CONFIG_NAMES :=
CFS_CONFIG_NAMES    := $(filter-out $(NONCFS_CONFIG_NAMES),$(CONFIG_NAMES))

# Define the output dir (O) for each target group
# this is required for everything listed in CONFIG_NAMES

O_native_std = build-native_std

# Define the ARCH used for each target group
# this is required for everything listed in CFS_CONFIG_NAMES
ARCH_native_std = native

# Define extra prep options for each target group

##############
# Native build (traditional standard development config)
##############
PREP_OPTS_native_std += -DENABLE_UNIT_TESTS=TRUE
PREP_OPTS_native_std += -DSIMULATION=$(ARCH)
PREP_OPTS_native_std += -DCFE_EDS_ENABLED=OFF
PREP_OPTS_native_std += -DMISSIONCONFIG=sample
PREP_OPTS_native_std += -DCMAKE_BUILD_TYPE=debug
PLATFORM_native_std  =  default_cpu1
