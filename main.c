#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <errno.h>

#include "mydrm_core.h"

int main(void) {

	int fd, ret = 0;

	drmModeRes *resources = mycore_openCard(DEV_CARD_PATH, &fd);
	if (resources == NULL) {
		perror("could not open device file");
		ret = -errno;
		goto freedom;
	}

	int try_set_master = drmSetMaster(fd);
	if (try_set_master < 0) {
		printf("could not take exclusive control of display output.\n");
		printf("is another compositor/desktop environment running?\n");
		ret = -errno;
		goto freedom;
	}

	drmModeConnector *active_connector = mycore_getConnectorActive(resources,
			 					       fd);
	if (active_connector == NULL) {
		perror("could not find active connector");
		ret = -errno;
		goto freedom;
	}

	drmModeModeInfo *top_res_mode = mycore_getHighestResolutionMode(active_connector);
	uint32_t vdisplay = top_res_mode->vdisplay;
	uint32_t hdisplay = top_res_mode->hdisplay;
	uint32_t vrefresh = top_res_mode->vrefresh;

	printf("best res mode: %ux%u@%u\n", 
		hdisplay, 
		vdisplay,
		vrefresh);
	printf("name: %s\n", top_res_mode->name);

	/* Get encoder currently connected to our active connector */
	drmModeEncoder *encoder = NULL;
	encoder = drmModeGetEncoder(fd, *active_connector->encoders);
	if (!encoder) {
		perror("could not get encoder");	
		ret = -errno;
		goto freedom;
	}

	printf("encoder id: %u\n", encoder->encoder_id);
	printf("encoder type: %u\n", encoder->encoder_type);

	drmModeCrtc *crtc = NULL;
	uint32_t working_crtc_id = 0;
	for (int i = 0; i < resources->count_crtcs; ++i) {
		uint32_t nth_bit_mask = (0x1 << i); // shift for each index
		if (encoder->possible_crtcs & nth_bit_mask) {
			crtc = drmModeGetCrtc(fd, resources->crtcs[i]);
			working_crtc_id = resources->crtcs[i];
			break;
		}
	}

	if (crtc == NULL) {
		perror("Couldn't determine right CRTC");
		ret = -errno;
		goto freedom;
	}

	struct drm_mode_create_dumb creq;
	struct drm_mode_map_dumb mreq;
	uint32_t fb_id;
	void *map = NULL;

	memset(&creq, '\0', sizeof(creq));

	creq.width = hdisplay;
	creq.height = vdisplay;
	creq.bpp = 32;
	drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
	
	drmModeAddFB(fd, creq.width, creq.height, 24, 32, creq.pitch, creq.handle, &fb_id);

	memset(&mreq, 0, sizeof(mreq));
	mreq.handle = creq.handle;
	drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);

	map = mmap(0, creq.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);

	drmModeSetCrtc(fd, working_crtc_id, fb_id, 0, 0, &active_connector->connector_id, 1, top_res_mode);
	
	uint32_t *pixels = (uint32_t *)map;

	for (size_t x = 50; x < 150; ++x) {
		for (size_t y = 50; y < 150; ++y) {

			uint32_t location = (y + x * (creq.pitch / 4));
			
			pixels[location] = 0x00FF0000;
		}
	}
	//drmModePageFlip(fd, working_crtc_id, fb_id, DRM_MODE_PAGE_FLIP_EVENT, NULL);

	drmModeClip clip;
	clip.x1 = 50;
	clip.y1 = 50;
	clip.x2 = 150;
	clip.y2 = 150;

	drmModeDirtyFB(fd, fb_id, &clip, 1);

	sleep(10);

freedom:
	if (map)
		munmap(map, creq.size);

	if (encoder)
		drmModeFreeEncoder(encoder);
	
	if (active_connector)
		drmModeFreeConnector(active_connector);
	
	if (resources)
		drmModeFreeResources(resources);

	if (fd > 0) {
		drmDropMaster(fd);
		close(fd);
	}

	return ret;
}
