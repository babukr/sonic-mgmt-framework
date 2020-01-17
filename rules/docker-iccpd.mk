# docker image for iccpd agent

DOCKER_ICCPD_STEM = docker-iccpd
DOCKER_ICCPD = $(DOCKER_ICCPD_STEM).gz
$(DOCKER_ICCPD)_PATH = $(DOCKERS_PATH)/docker-iccpd
$(DOCKER_ICCPD)_DEPENDS += $(SWSS) $(REDIS_TOOLS) $(ICCPD)
$(DOCKER_ICCPD)_LOAD_DOCKERS += $(DOCKER_CONFIG_ENGINE_STRETCH)
$(DOCKER_ICCPD)_DBG_IMAGE_PACKAGES = $($(DOCKER_CONFIG_ENGINE_STRETCH)_DBG_IMAGE_PACKAGES)
SONIC_DOCKER_IMAGES += $(DOCKER_ICCPD)
SONIC_INSTALL_DOCKER_IMAGES += $(DOCKER_ICCPD)

SONIC_STRETCH_DOCKERS += $(DOCKER_ICCPD)


DOCKER_ICCPD_DBG = $(DOCKER_ICCPD_STEM)-$(DBG_IMAGE_MARK).gz
SONIC_DOCKER_DBG_IMAGES += $(DOCKER_ICCPD_DBG)
SONIC_STRETCH_DBG_DOCKERS += $(DOCKER_ICCPD_DBG)
SONIC_INSTALL_DOCKER_DBG_IMAGES += $(DOCKER_ICCPD_DBG)


$(DOCKER_ICCPD)_CONTAINER_NAME = iccpd
$(DOCKER_ICCPD)_RUN_OPT += --net=host --privileged -t
$(DOCKER_ICCPD)_RUN_OPT += -v /etc/sonic:/etc/sonic:ro

$(DOCKER_ICCPD)_BASE_IMAGE_FILES += mclagdctl:/usr/bin/mclagdctl
