#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <errno.h>

#define DEV_CARD_PATH "/dev/dri/card0"

drmModeRes		*mycore_openCard(const char *path, int *fd);
drmModeConnector	*mycore_getConnectorActive(drmModeRes *resources, int fd);
drmModeModeInfo 	*mycore_getHighestResolutionMode(drmModeConnector *connector);


