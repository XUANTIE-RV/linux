#ifndef _UAPI_LINUX_ULTRAFB_H
#include <stdbool.h>

#define _UAPI_LINUX_ULTRAFB_H

#define ULTRAFBIO_CURSOR 0x46A0
#define ULTRAFBIO_BLENDING_MODE 0x46A4
#define ULTRAFBIO_GLOBAL_MODE_VALUE 0x46A5
#define ULTRAFBIO_BUFFER_SIZE 0x46A7
#define ULTRAFBIO_OVERLAY_RECT 0x46A8
#define ULTRAFBIO_SCALE_FILTER_TAP 0x46A9
#define ULTRAFBIO_SYNC_TABLE 0x46AA
#define ULTRAFBIO_GAMMA 0x46BD
#define ULTRAFBIO_DITHER 0x46BE
#define ULTRAFBIO_ROTATION 0x46BF
#define ULTRAFBIO_TILEMODE 0x46C1
#define ULTRAFBIO_COLORKEY 0x46C2

#define CURSOR_SIZE 32
#define GAMMA_INDEX_MAX 256

typedef struct _dc_frame_info {
	unsigned int width;
	unsigned int height;
	unsigned int stride;
}
dc_frame_info;

typedef struct _dc_overlay_rect {
	unsigned int tlx;
	unsigned int tly;
	unsigned int brx;
	unsigned int bry;
}
dc_overlay_rect;

typedef enum _dc_alpha_blending_mode {
	DC_BLEND_MODE_CLEAR = 0x0,
	DC_BLEND_MODE_SRC,
	DC_BLEND_MODE_DST,
	DC_BLEND_MODE_SRC_OVER,
	DC_BLEND_MODE_DST_OVER,
	DC_BLEND_MODE_SRC_IN,
	DC_BLEND_MODE_DST_IN,
	DC_BLEND_MODE_SRC_OUT,
}
dc_alpha_blending_mode;

typedef enum _dc_rot_angle {
	DC_ROT_ANGLE_ROT0 = 0x0,
	DC_ROT_ANGLE_FLIP_X,
	DC_ROT_ANGLE_FLIP_Y,
	DC_ROT_ANGLE_FLIP_XY,
	DC_ROT_ANGLE_ROT90,
	DC_ROT_ANGLE_ROT180,
	DC_ROT_ANGLE_ROT270,
}
dc_rot_angle;

typedef enum _dc_tile_mode {
	DC_TILE_MODE_LINEAR = 0x0,
	DC_TILE_MODE_TILED4X4,
	DC_TILE_MODE_SUPER_TILED_XMAJOR,
	DC_TILE_MODE_SUPER_TILED_YMAJOR,
	DC_TILE_MODE_TILE_MODE8X8,
	DC_TILE_MODE_TILE_MODE8X4,
	DC_TILE_MODE_SUPER_TILED_XMAJOR8X4,
	DC_TILE_MODE_SUPER_TILED_YMAJOR4X8,
	DC_TILE_MODE_TILE_Y,
}
dc_tile_mode;

typedef struct _dc_global_alpha {
	unsigned int global_alpha_mode;
	unsigned int global_alpha_value;
}
dc_global_alpha;

typedef struct _dc_filter_tap {
	unsigned int vertical_filter_tap;
	unsigned int horizontal_filter_tap;
}
dc_filter_tap;

typedef struct _dc_sync_table {
	unsigned int horkernel[128];
	unsigned int verkernel[128];
}
dc_sync_table;

typedef struct _dc_gamma_table {
	bool gamma_enable;
	unsigned int gamma[GAMMA_INDEX_MAX][3];
}
dc_gamma_table;

typedef struct _dc_color_key {
	unsigned char enable;
	unsigned int colorkey_low;
	unsigned int colorkey_high;
	/* background color only available for video, not available for overlay*/
	unsigned int bg_color;
}
dc_color_key;

#endif
