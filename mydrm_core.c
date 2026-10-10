#include "mydrm_core.h"


drmModeRes *mycore_openCard(const char *path, int *fd) 
{
	if (!path || !fd) 
		return NULL;

	*fd = open(path, O_RDWR | O_CLOEXEC);
	if (*fd < 0) 
		return NULL;

	drmModeRes *resource = drmModeGetResources(*fd);
	if (!resource) {
		close(*fd);
		return NULL;
	}

	return resource;
}

drmModeConnector *mycore_getConnectorActive(drmModeRes *resources, int fd)
{
	int num_conn = resources->count_connectors;
	uint32_t *connectors = resources->connectors;

	for (int i = 0; i < num_conn; ++i) {
		drmModeConnector *connector = drmModeGetConnector(fd, connectors[i]);
		
		if (connector->connection == DRM_MODE_CONNECTED) {
			return connector;			
		}
	}

	return NULL;
}

drmModeModeInfo *mycore_getHighestResolutionMode(drmModeConnector *connector)
{
	int num_modes = connector->count_modes;
	uint32_t res = 0;
	drmModeModeInfo *highest_res_mode = NULL;

	for (int i = 0; i < num_modes; ++i) {
		drmModeModeInfo *mode_info = &connector->modes[i];
		uint32_t new_res = mode_info->hdisplay * mode_info->vdisplay;
		if (new_res > res) {
			res = new_res;
			highest_res_mode = mode_info;
		}
	}

	return highest_res_mode;
}
