#include "Gpu.h"

void gpu_init(gpu_t* gpu)
{
	memset(gpu, 0, sizeof(gpu_t));
}

void gpu_fini(gpu_t* gpu)
{
	memset(gpu, 0, sizeof(gpu_t));
}

void gpu_reset(gpu_t* gpu)
{
	gpu->fade = 0;
	gpu->obj_pal_start = 0;
	gpu->obj_tile_start = 0;
	gpu->obj_scroll_x = 0;
	gpu->obj_scroll_y = 0;
	gpu->fg_pal_start = 0;
	gpu->fg_tile_start = 0;
	gpu->fg_scroll_x = 0;
	gpu->fg_scroll_y = 0;
	gpu->gb_pal_start = 0;
	gpu->bg_tile_start = 0;
	gpu->bg_scroll_x = 0;
	gpu->bg_scroll_y = 0;
	gpu->ui_pal_start = 0;
	gpu->ui_tile_start = 0;
	gpu->ui_size_x = 0;
	gpu->ui_size_y = 0;
	gpu->overlay_pal = NULL;
	gpu->overlay_data = NULL;
	gpu->overlay_stride = 0;

	memset(gpu->fg_line_scroll, 0, sizeof(gpu->fg_line_scroll));
	memset(gpu->bg_line_scroll, 0, sizeof(gpu->bg_line_scroll));

	for (uint8 i = 0; i < GPU_OBJ_MAX; ++i)
	{
		gpu_obj_clear(gpu, i);
	}
}

void gpu_update(gpu_t* gpu)
{
	memset(gpu->obj_attr_table, 0, sizeof(gpu->obj_attr_table));

	uint8 prio_min = UINT8_MAX;
	uint8 obj_idxs[GPU_OBJ_MAX];
	uint8 obj_num = 0;

	for (uint32 i = 0; i < GPU_OBJ_MAX; ++i)
	{
		uint8 obj_idx = GPU_OBJ_MAX - i - 1;
		obj_t* obj = &gpu->obj_table[obj_idx];

		if (obj->tile_num_x)
		{
			obj_idxs[obj_num++] = obj_idx;

			prio_min = MIN(prio_min, obj->prio);
		}
	}

	uint8 sorted_obj_idxs[GPU_OBJ_MAX + 1];
	uint8 sorted_obj_num = 0;

	while (obj_num > 0)
	{
		uint8 new_obj_num = 0;
		uint8 new_prio_min = UINT8_MAX;

		for (uint32 i = 0; i < obj_num; ++i)
		{
			obj_t* obj = &gpu->obj_table[obj_idxs[i]];

			if (obj->prio == prio_min)
			{
				sorted_obj_idxs[sorted_obj_num++] = obj_idxs[i];
			}
			else
			{
				if (i != new_obj_num)
				{
					obj_idxs[new_obj_num] = obj_idxs[i];
				}

				new_obj_num++;

				if (obj->prio < new_prio_min && obj->prio > prio_min)
				{
					new_prio_min = obj->prio;
				}
			}
		}

		obj_num = new_obj_num;
		prio_min = new_prio_min;
	}

	for (uint32 i = 0; i < sorted_obj_num; ++i)
	{
		obj_t* obj = &gpu->obj_table[sorted_obj_idxs[i]];
		int16 obj_w = obj->tile_num_x * GPU_TILE_WH;
		int16 obj_h = obj->tile_num_y * GPU_TILE_WH;

		if (obj->x <= -obj_w || obj->x >= GPU_SCR_W)
		{
			continue;
		}

		if (obj->y <= -obj_h || obj->y >= GPU_SCR_H)
		{
			continue;
		}

		uint32 draw_x;
		uint32 draw_y;
		uint32 sprite_x;
		uint32 sprite_y;
		uint32 sprite_w;
		uint32 sprite_h;

		if (obj->x < 0)
		{
			draw_x = 0;
			sprite_x = -obj->x;
			sprite_w = obj_w;
		}
		else
		{
			draw_x = obj->x;
			sprite_x = 0;
			sprite_w = MIN(obj_w, GPU_SCR_W - obj->x);
		}

		if (obj->y < 0)
		{
			draw_y = 0;
			sprite_y = -obj->y;
			sprite_h = obj_h;
		}
		else
		{
			draw_y = obj->y;
			sprite_y = 0;
			sprite_h = MIN(obj_h, GPU_SCR_H - obj->y);
		}

		uint8 flip_x = obj->flip & GPU_SPRITE_FLIP_X;
		uint8 flip_y = obj->flip & GPU_SPRITE_FLIP_Y;
		uint32 tile_offs_x = flip_x ? (obj_w - 1 - sprite_x) : sprite_x;
		uint32 tile_offs_y = flip_y ? (obj_h - 1 - sprite_y) : sprite_y;
		const uint32* tile_stream = &gpu->tile_buffer[gpu->obj_tile_start + obj->tile_idx][MAD(tile_offs_y, obj->tile_num_x, tile_offs_x / GPU_TILE_WH)];
		obj_attr_t obj_attr;

		obj_attr.pal_idx = gpu->obj_pal_start + obj->pal_idx;
		obj_attr.prio = obj->prio >> 4;

		for (uint32 y = sprite_y; y < sprite_h; ++y)
		{
			int32 cursor = (draw_x - obj->x);
			uint32 accum = 0;
			const uint32* row_stream = tile_stream;

			for (uint32 x = sprite_x; x < sprite_w; ++x)
			{
				if (flip_x)
				{
					if (x == sprite_x || !(cursor % GPU_TILE_WH))
					{
						accum = *row_stream-- >> GPU_BPP * (cursor % GPU_TILE_WH);
					}

					obj_attr.col_idx = accum & 0xF;

					accum >>= GPU_BPP;
				}
				else
				{
					if (x == sprite_x || !(cursor % GPU_TILE_WH))
					{
						accum = *row_stream++ << GPU_BPP * (cursor % GPU_TILE_WH);
					}

					obj_attr.col_idx = accum >> 28;

					accum <<= GPU_BPP;
				}

				if (obj_attr.col_idx)
				{
					gpu->obj_attr_table[draw_y + (y - sprite_y)][draw_x + (x - sprite_x)] = obj_attr;
				}

				cursor++;
			}

			if (flip_y)
			{
				tile_stream -= obj->tile_num_x;
			}
			else
			{
				tile_stream += obj->tile_num_x;
			}
		}
	}

	for (int16 y = 0; y < GPU_SCR_H; ++y)
	{
		const rgb_t* fg_pal = NULL;
		const rgb_t* bg_pal = NULL;
		uint32 fg_accum = 0;
		uint32 bg_accum = 0;

		for (int16 x = 0; x < GPU_SCR_W; ++x)
		{
			obj_attr_t* obj_attr = &gpu->obj_attr_table[y][x];
			uint32 fg_x = x + gpu->fg_scroll_x + gpu->fg_line_scroll[y];
			uint32 fg_y = y + gpu->fg_scroll_y;
			uint32 bg_x = x + gpu->bg_scroll_x + gpu->bg_line_scroll[y];
			uint32 bg_y = y + gpu->bg_scroll_y;
			const uint8* overlay_col_idx = NULL;

			if (gpu->overlay_data && gpu->overlay_enable)
			{
				overlay_col_idx = MEMOFFS(gpu->overlay_data, MAD(fg_y, gpu->overlay_stride, fg_x));
			}

			if (gpu->ui_size_x || gpu->ui_size_y)
			{
				uint8 draw_ui = 1;

				if (gpu->ui_size_x)
				{
					uint16 ui_x;
					uint16 ui_w;

					if (gpu->ui_size_x > 0)
					{
						ui_x = x / GPU_TILE_WH;
						ui_w = gpu->ui_size_x;
					}
					else
					{
						ui_x = (GPU_SCR_W - x - 1) / GPU_TILE_WH;
						ui_w = -gpu->ui_size_x;
					}

					if (ui_x >= ui_w)
					{
						draw_ui = 0;
					}

					if (ui_x == ui_w)
					{
						fg_pal = NULL;
					}
				}

				if (gpu->ui_size_y)
				{
					uint16 ui_y;
					uint16 ui_h;

					if (gpu->ui_size_y > 0)
					{
						ui_y = y / GPU_TILE_WH;
						ui_h = gpu->ui_size_y;
					}
					else
					{
						ui_y = (GPU_SCR_H - y - 1) / GPU_TILE_WH;
						ui_h = -gpu->ui_size_y;
					}

					if (ui_y >= ui_h)
					{
						draw_ui = 0;
					}
				}

				if (draw_ui)
				{
					if (!fg_pal || !(x % GPU_TILE_WH))
					{
						tile_attr_t attr = gpu->ui_attr_table[y / 8][x / GPU_TILE_WH];
						uint16 tile_idx = attr.tile_idx & ~(GPU_UI_FLIP_X | GPU_UI_FLIP_Y);
						uint16 flip_x = attr.tile_idx & GPU_UI_FLIP_X;
						uint16 flip_y = attr.tile_idx & GPU_UI_FLIP_Y;

						fg_pal = gpu->pal_table[gpu->ui_pal_start + attr.pal_idx];
						fg_accum = gpu->tile_buffer[gpu->ui_tile_start + tile_idx][flip_y ? (GPU_TILE_WH - (y % 8) - 1) : (y % 8)];

						if (flip_x)
						{
							fg_accum = ((_byteswap_ulong(fg_accum) << 4) & 0xF0F0F0F0) | ((_byteswap_ulong(fg_accum) >> 4) & 0x0F0F0F0F);
						}
					}

					fg_x = 1;
					overlay_col_idx = NULL;
				}
			}

			if (!fg_pal || !(fg_x % GPU_TILE_WH))
			{
				if (overlay_col_idx)
				{
					fg_pal = gpu->overlay_pal;
					fg_accum = 0;
				}
				else
				{
					tile_attr_t attr = gpu->fg_attr_table[(fg_y / GPU_TILE_WH) % GPU_FG_WH][(fg_x / GPU_TILE_WH) % GPU_FG_WH];

					fg_pal = gpu->pal_table[gpu->fg_pal_start + attr.pal_idx];
					fg_accum = gpu->tile_buffer[gpu->fg_tile_start + attr.tile_idx][fg_y % GPU_TILE_WH] << (GPU_BPP * (fg_x % GPU_TILE_WH));
				}
			}

			if (!bg_pal || !(bg_x % GPU_TILE_WH))
			{
				tile_attr_t attr = gpu->bg_attr_table[(bg_y / GPU_TILE_WH) % GPU_BG_WH][(bg_x / GPU_TILE_WH) % GPU_BG_WH];

				bg_pal = gpu->pal_table[gpu->gb_pal_start + attr.pal_idx];
				bg_accum = gpu->tile_buffer[gpu->bg_tile_start + attr.tile_idx][bg_y % GPU_TILE_WH] << (GPU_BPP * (bg_x % GPU_TILE_WH));
			}

			int32 obj_prio = obj_attr->prio;

			if (!obj_attr->col_idx)
			{
				obj_prio = -1;
			}

			uint8 fg_col_idx = 0;
			uint8 bg_col_idx = 0;

			if (obj_prio < (GPU_SPRITE_PRIO_FG >> 4))
			{
				if (overlay_col_idx)
				{
					fg_col_idx = *overlay_col_idx;
				}
				else
				{
					fg_col_idx = fg_accum >> 28;
				}
			}

			if (obj_prio < (GPU_SPRITE_PRIO_BG >> 4))
			{
				bg_col_idx = bg_accum >> 28;
			}

			rgb_t col;

			if (fg_col_idx)
			{
				col = fg_pal[fg_col_idx];
			}
			else if (bg_col_idx)
			{
				col = bg_pal[bg_col_idx];
			}
			else
			{
				col = gpu->pal_table[obj_attr->pal_idx][obj_attr->col_idx];
			}

			rgba_t screen_col;

			screen_col.r = col.r * gpu->fade / UINT8_MAX;
			screen_col.g = col.g * gpu->fade / UINT8_MAX;
			screen_col.b = col.b * gpu->fade / UINT8_MAX;
			screen_col.a = UINT8_MAX;

			gpu->framebuffer[y][x] = screen_col;

			fg_accum <<= GPU_BPP;
			bg_accum <<= GPU_BPP;
		}
	}
}

void gpu_obj_scroll_x(gpu_t* gpu, int16 val)
{
	gpu->obj_scroll_x = val;
}

void gpu_obj_scroll_y(gpu_t* gpu, int16 val)
{
	gpu->obj_scroll_y = val;
}

void gpu_fg_scroll_x(gpu_t* gpu, int16 val)
{
	gpu->fg_scroll_x = val;
}

void gpu_fg_scroll_y(gpu_t* gpu, int16 val)
{
	gpu->fg_scroll_y = val;
}

void gpu_bg_scroll_x(gpu_t* gpu, int16 val)
{
	gpu->bg_scroll_x = val;
}

void gpu_bg_scroll_y(gpu_t* gpu, int16 val)
{
	gpu->bg_scroll_y = val;
}

void gpu_ui_size_x(gpu_t* gpu, int8 val)
{
	gpu->ui_size_x = val;
}

void gpu_ui_size_y(gpu_t* gpu, int8 val)
{
	gpu->ui_size_y = val;
}

void gpu_obj_pal(gpu_t* gpu, uint8 idx)
{
	gpu->obj_pal_start = idx;
}

void gpu_fg_pal(gpu_t* gpu, uint8 idx)
{
	gpu->fg_pal_start = idx;
}

void gpu_bg_pal(gpu_t* gpu, uint8 idx)
{
	gpu->gb_pal_start = idx;
}

void gpu_ui_pal(gpu_t* gpu, uint8 idx)
{
	gpu->ui_pal_start = idx;
}

void gpu_fade(gpu_t* gpu, uint8 val)
{
	gpu->fade = val;
}

void gpu_fg_clear(gpu_t* gpu)
{
	memset(gpu->fg_attr_table, 0, sizeof(gpu->fg_attr_table));
}

void gpu_bg_clear(gpu_t* gpu)
{
	memset(gpu->bg_attr_table, 0, sizeof(gpu->bg_attr_table));
}

void gpu_ui_clear(gpu_t* gpu)
{
	memset(gpu->ui_attr_table, 0, sizeof(gpu->ui_attr_table));
}

void gpu_overlay_enable(gpu_t* gpu, uint8 val)
{
	gpu->overlay_enable = val;
}

void gpu_overlay_set(gpu_t* gpu, const void* data, const rgb_t* pal, uint32 stride)
{
	gpu->overlay_data = data;
	gpu->overlay_pal = pal;
	gpu->overlay_stride = stride;
	gpu->overlay_enable = 1;
}

void gpu_overlay_clear(gpu_t* gpu)
{
	gpu->overlay_data = NULL;
	gpu->overlay_pal = NULL;
	gpu->overlay_stride = 0;
	gpu->overlay_enable = 1;
}

void gpu_fg_draw(gpu_t* gpu, uint8 x, uint8 y, uint16 tile_idx, uint8 pal_idx)
{
	gpu->fg_attr_table[y][x].tile_idx = tile_idx;
	gpu->fg_attr_table[y][x].pal_idx = pal_idx;
}

void gpu_bg_draw(gpu_t* gpu, uint8 x, uint8 y, uint16 tile_idx, uint8 pal_idx)
{
	gpu->bg_attr_table[y][x].tile_idx = tile_idx;
	gpu->bg_attr_table[y][x].pal_idx = pal_idx;
}

void gpu_ui_draw(gpu_t* gpu, uint8 x, uint8 y, uint16 tile_idx, uint8 pal_idx)
{
	gpu->ui_attr_table[y][x].tile_idx = tile_idx;
	gpu->ui_attr_table[y][x].pal_idx = pal_idx;
}

void gpu_obj_draw(gpu_t* gpu, uint8 idx, uint8 flip, uint8 prio, int16 x, int16 y, uint16 tile_idx, uint8 pal_idx, uint16 w, uint16 h)
{
	gpu->obj_table[idx].flip = flip;
	gpu->obj_table[idx].prio = prio;
	gpu->obj_table[idx].x = gpu->obj_scroll_x + x;
	gpu->obj_table[idx].y = gpu->obj_scroll_y + y;
	gpu->obj_table[idx].tile_idx = tile_idx;
	gpu->obj_table[idx].pal_idx = pal_idx;
	gpu->obj_table[idx].tile_num_x = w / GPU_TILE_WH;
	gpu->obj_table[idx].tile_num_y = h / GPU_TILE_WH;
}

void gpu_obj_clear(gpu_t* gpu, uint8 idx)
{
	gpu->obj_table[idx].tile_num_x = 0;
}

void gpu_pal_set(gpu_t* gpu, uint8 pal_idx, const rgb_t* pal)
{
	memcpy(gpu->pal_table[pal_idx], pal, GPU_PAL_COL_MAX * sizeof(rgb_t));
}

void gpu_pal_col_set(gpu_t* gpu, uint8 pal_idx, uint8 col_idx, const rgb_t* col)
{
	memcpy(&gpu->pal_table[pal_idx][col_idx], col, sizeof(rgb_t));
}

void gpu_tile_copy(gpu_t* gpu, uint16 tile_idx, const void* data, uint32 size)
{
	memcpy(gpu->tile_buffer[tile_idx], data, size);
}

void gpu_tile_set_row(gpu_t* gpu, uint16 tile_idx, uint32 offs, uint32 val)
{
	gpu->tile_buffer[tile_idx][offs] = val;
}

rgba_t* gpu_get_framebuffer(gpu_t* gpu)
{
	return gpu->framebuffer;
}
