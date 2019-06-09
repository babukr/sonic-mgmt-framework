# Docker image for L2MCD

DOCKER_L2MCD = docker-l2mcd.gz
$(DOCKER_L2MCD)_PATH = $(DOCKERS_PATH)/docker-l2mcd
$(DOCKER_L2MCD)_DEPENDS += $(L2MCD) $(SWSS)

$(DOCKER_L2MCD)_LOAD_DOCKERS = $(DOCKER_CONFIG_ENGINE)

SONIC_DOCKER_IMAGES += $(DOCKER_L2MCD)

SONIC_INSTALL_DOCKER_IMAGES += $(DOCKER_L2MCD)


$(DOCKER_L2MCD)_CONTAINER_NAME = l2mcd
$(DOCKER_L2MCD)_RUN_OPT += --net=host --privileged -t
$(DOCKER_L2MCD)_RUN_OPT += -v /etc/sonic:/etc/sonic:ro
$(DOCKER_L2MCD)_RUN_OPT += -v /host/warmboot:/var/warmboot
