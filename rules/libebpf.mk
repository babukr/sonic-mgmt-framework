# BPF package
BPF_VERSION := 1.0.0
LIBEBPF = libebpf_$(BPF_VERSION)_amd64.deb
$(LIBEBPF)_SRC_PATH = $(SRC_PATH)/libebpf
$(LIBEBPF)_DEPENDS += $(LINUX_HEADERS) $(LINUX_COMMON_HEADERS)
SONIC_MAKE_DEBS += $(LIBEBPF)

LIBEBPF_DBG = libebpf-dbgsym_$(BPF_VERSION)_amd64.deb
$(LIBEBPF_DBG)_DEPENDS += $(LIBEBPF)
$(eval $(call add_derived_package,$(LIBEBPF),$(LIBEBPF_DBG)))

export LIBEBPF LIBEBPF_DBG BPF_VERSION

