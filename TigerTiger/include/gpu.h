#pragma once

#include "common.h"

#define GPU_SCR_W 320
#define GPU_SCR_H 240
#define GPU_OBJ_MAX 64
#define GPU_PAL_MAX 64
#define GPU_PAL_COL_MAX 16
#define GPU_TILE_MAX 4096
#define GPU_TILE_WH 8
#define GPU_BPP 4
#define GPU_FG_WH 64
#define GPU_BG_WH 64
#define GPU_UI_W 64
#define GPU_UI_H 32
#define GPU_SPRITE_FLIP_X 0x1
#define GPU_SPRITE_FLIP_Y 0x2
#define GPU_SPRITE_PRIO_FG 0x80
#define GPU_SPRITE_PRIO_BG 0x40
#define GPU_UI_FLIP_X 0x400
#define GPU_UI_FLIP_Y 0x800

typedef struct gpu_tile_attr_t
{
	uint16 tile_idx : 12;
	uint16 pal_idx : 4;
} tile_attr_t;

typedef struct gpu_obj_attr_t
{
	uint16 col_idx : 4;
	uint16 pal_idx : 8;
	uint16 prio : 4;
} obj_attr_t;

typedef struct gpu_obj_t
{
	uint8 flip;
	uint8 prio;
	int16 x;
	int16 y;
	uint16 tile_idx : 12;
	uint16 pal_idx : 4;
	uint8 tile_num_x;
	uint8 tile_num_y;
} obj_t;

typedef struct gpu_t
{
	uint8 overlay_enable;
	const void* overlay_data;
	const rgb_t* overlay_pal;
	uint32 overlay_stride;
	uint8 fade;
	uint8 obj_pal_start;
	uint16 obj_tile_start;
	int16 obj_scroll_x;
	int16 obj_scroll_y;
	uint8 fg_pal_start;
	uint16 fg_tile_start;
	int16 fg_scroll_x;
	int16 fg_scroll_y;
	uint8 gb_pal_start;
	uint16 bg_tile_start;
	int16 bg_scroll_x;
	int16 bg_scroll_y;
	uint8 ui_pal_start;
	uint16 ui_tile_start;
	int8 ui_size_x;
	int8 ui_size_y;
	uint32 tile_buffer[GPU_TILE_MAX][GPU_TILE_WH];
	rgb_t pal_table[GPU_PAL_MAX][GPU_PAL_COL_MAX];
	int16 fg_line_scroll[GPU_SCR_H];
	int16 bg_line_scroll[GPU_SCR_H];
	obj_t obj_table[GPU_OBJ_MAX];
	tile_attr_t fg_attr_table[GPU_FG_WH][GPU_FG_WH];
	tile_attr_t bg_attr_table[GPU_BG_WH][GPU_BG_WH];
	tile_attr_t ui_attr_table[GPU_UI_H][GPU_UI_W];
	obj_attr_t obj_attr_table[GPU_SCR_H][GPU_SCR_W];
	rgba_t framebuffer[GPU_SCR_H][GPU_SCR_W];
} gpu_t;

void gpu_init(gpu_t* gpu);
void gpu_fini(gpu_t* gpu);
void gpu_reset(gpu_t* gpu);
void gpu_update(gpu_t* gpu);
void gpu_obj_scroll_x(gpu_t* gpu, int16 val);
void gpu_obj_scroll_y(gpu_t* gpu, int16 val);
void gpu_fg_scroll_x(gpu_t* gpu, int16 val);
void gpu_fg_scroll_y(gpu_t* gpu, int16 val);
void gpu_bg_scroll_x(gpu_t* gpu, int16 val);
void gpu_bg_scroll_y(gpu_t* gpu, int16 val);
void gpu_ui_size_x(gpu_t* gpu, int8 val);
void gpu_ui_size_y(gpu_t* gpu, int8 val);
void gpu_obj_pal(gpu_t* gpu, uint8 idx);
void gpu_fg_pal(gpu_t* gpu, uint8 idx);
void gpu_bg_pal(gpu_t* gpu, uint8 idx);
void gpu_ui_pal(gpu_t* gpu, uint8 idx);
void gpu_fade(gpu_t* gpu, uint8 val);
void gpu_fg_clear(gpu_t* gpu);
void gpu_bg_clear(gpu_t* gpu);
void gpu_ui_clear(gpu_t* gpu);
void gpu_overlay_enable(gpu_t* gpu, uint8 val);
void gpu_overlay_set(gpu_t* gpu, const void* data, const rgb_t* pal, uint32 stride);
void gpu_overlay_clear(gpu_t* gpu);
void gpu_fg_draw(gpu_t* gpu, uint8 x, uint8 y, uint16 tile_idx, uint8 pal_idx);
void gpu_bg_draw(gpu_t* gpu, uint8 x, uint8 y, uint16 tile_idx, uint8 pal_idx);
void gpu_ui_draw(gpu_t* gpu, uint8 x, uint8 y, uint16 tile_idx, uint8 pal_idx);
void gpu_obj_draw(gpu_t* gpu, uint8 idx, uint8 flip, uint8 prio, int16 x, int16 y, uint16 tile_idx, uint8 pal_idx, uint16 w, uint16 h);
void gpu_obj_clear(gpu_t* gpu, uint8 idx);
void gpu_pal_set(gpu_t* gpu, uint8 pal_idx, const rgb_t* pal);
void gpu_pal_col_set(gpu_t* gpu, uint8 pal_idx, uint8 col_idx, const rgb_t* col);
void gpu_tile_copy(gpu_t* gpu, uint16 tile_idx, const void* data, uint32 size);
void gpu_tile_set_row(gpu_t* gpu, uint16 tile_idx, uint32 offs, uint32 val);
rgba_t* gpu_get_framebuffer(gpu_t* gpu);
