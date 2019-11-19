# SONiC TAM package

TAM_VERSION = 1.0.0
export TAM_VERSION

SONIC_TAM = tam_$(TAM_VERSION)_amd64.deb
$(SONIC_TAM)_SRC_PATH = $(SRC_PATH)/sonic-tam
$(SONIC_TAM)_DEPENDS = $(LIBSWSSCOMMON_DEV) $(PROTOBUF)
$(SONIC_TAM)_RDEPENDS = $(LIBSWSSCOMMON) $(PROTOBUF)
SONIC_DPKG_DEBS += $(SONIC_TAM)

TAM_DBG = tam-dbg_1.0.0_amd64.deb
$(TAM_DBG)_DEPENDS += $(TAM)
$(TAM_DBG)_RDEPENDS += $(TAM)
$(eval $(call add_derived_package,$(TAM),$(TAM_DBG)))

# SONiC Timestamping Manager
SONIC_TS_MGR = tsmgrd
$(SONIC_TS_MGR)_PATH = $(SRC_PATH)/sonic-tam/tsmgrd
SONIC_COPY_FILES += $(SONIC_TS_MGR)

# SONiC Drop monitor Manager
SONIC_DROP_MGR = dropmgrd
$(SONIC_DROP_MGR)_PATH = $(SRC_PATH)/sonic-tam/dropmgr
SONIC_COPY_FILES += $(SONIC_DROP_MGR)

# SONiC IFA Manager
SONIC_IFA_MGR = ifamgrd
$(SONIC_IFA_MGR)_PATH = $(SRC_PATH)/sonic-tam/ifamgr
SONIC_COPY_FILES += $(SONIC_IFA_MGR)
