#include "game.h"

#include "csvb.h"
#include "file.h"

#define GAME2DRAW(x) (int16)((x) / 256)
#define GAME2SCROLL(x) ((x) * 256)
#define SCROLL2GAME(x) ((x) / 256)
#define SCROLL2DRAW(x) (int16)((x) / 65536)
#define DRAW2GAME(x) ((x) * 256)
#define DRAW2SCROLL(x) ((x) * 65536)
#define BADGE_ETHER_MULT 150
#define BADGE_HP_MULT 120
#define BADGE_NO_KILL_MULT 200
#define BADGE_PERFECT_MULT 150
#define BADGE_PERFECT_ETHER 777

typedef struct player_info_t
{
	uint16 hp_max;
	uint8 hix_w;
	uint8 hit_h;
	uint8 atk_w;
	uint8 atk_h;
	uint16 spd_u;
	uint16 spd_lr;
	uint16 spd_d;
} player_info_t;

static player_info_t player_table[] =
{
	{ 255, 8, 8, 12, 6, 450, 400, 350 },
};

typedef struct enemy_info_t
{
	uint8 move_type;
	uint8 hit_type;
	uint8 hit_w;
	uint8 hit_h;
	uint8 atk;
	uint16 spd;
	uint16 range;
	uint16 search;
	uint16 act_spd;
	uint8 Hp;
	uint8 back_time;
	uint16 back_spd;
	uint16 score;
	uint8 res_idx;
} enemy_info_t;

static enemy_info_t enemy_table[] =
{
	{ 1,  0, 20, 8,  100, 150, 80,  100, 200,  1, 30, 0,  80,  0  },
	{ 0,  3, 8,  8,  255, 0,   0,   0,   0,    1, 30, 0,  1,   1  },
	{ 1,  2, 16, 8,  50,  100, 50,  100, 100,  1, 30, 0,  100, 2  },
	{ 2,  1, 12, 20, 75,  30,  10,  100, 100,  1, 30, 0,  85,  3  },
	{ 4,  0, 12, 12, 30,  100, 70,  100, 250,  1, 30, 0,  200, 4  },
	{ 5,  0, 12, 12, 75,  150, 100, 75,  1300, 1, 30, 0,  75,  5  },
	{ 6,  0, 12, 24, 100, 500, 100, 150, 1600, 1, 30, 0,  115, 6  },
	{ 7,  0, 12, 12, 75,  300, 100, 120, 1400, 1, 30, 0,  70,  7  },
	{ 3,  0, 20, 8,  125, 100, 80,  80,  150,  2, 10, 75, 120, 10 },
	{ 0,  3, 8,  8,  255, 0,   0,   0,   0,    1, 30, 0,  1,   1  },
	{ 3,  0, 20, 8,  125, 150, 80,  80,  200,  3, 10, 75, 130, 10 },
	{ 9,  3, 12, 12, 50,  0,   0,   0,   0,    1, 30, 0,  0,   8  },
	{ 10, 3, 1,  16, 0,   0,   0,   0,   0,    1, 30, 0,  0,   11 },
};

typedef struct item_info_t
{
	uint8 type;
	uint16 score;
} item_info_t;

static item_info_t item_table[] =
{
	{ 0, 100  },
	{ 1, 300  },
	{ 0, 100  },
	{ 1, 300  },
	{ 0, 100  },
	{ 1, 300  },
	{ 0, 100  },
	{ 1, 300  },
	{ 0, 100  },
	{ 1, 300  },
	{ 2, 1111 },
};

typedef struct ether_info_t
{
	uint16 score;
} ether_info_t;

static ether_info_t ether_table[] =
{
	{ 100 },
	{ 100 },
};

typedef struct stage_info_t
{
	uint16 scroll_down_spd;
	uint16 scroll_up_spd;
	uint16 scroll_up_accel;
	uint16 scroll_up_spd_max;
	uint16 scroll_threshold;
	uint16 star_rate;
	uint16 o2_rate;
	uint16 o2_val;
	uint16 bonus_time;
	uint16 bonus_uni_rate;
	uint8 bonus_ether_rate;
	uint8 bous_entry_time;
} stage_info_t;

static stage_info_t stage_table[] =
{
	{ 125, 240, 2, 400, 0,    200,   500,   120, 450, 2200, 20, 5 },
	{ 125, 240, 2, 400, 0,    200,   500,   120, 450, 2300, 20, 5 },
	{ 125, 240, 3, 400, 0,    200,   500,   120, 450, 2400, 25, 5 },
	{ 125, 240, 4, 400, 0,    200,   500,   120, 450, 2500, 25, 5 },
	{ 125, 240, 5, 400, 0,    200,   500,   120, 450, 2600, 30, 5 },
	{ 100, 200, 0, 400, 160,  0,     0,     0,   0,   0,    0,  0 },
	{ 100, 180, 2, 400, 160,  500,   500,   255, 450, 2200, 20, 5 },
	{ 100, 100, 0, 400, 160,  10000, 10000, 255, 450, 2200, 20, 5 },
};

static uint8 cheat_table[][CHEAT_INPUT_MAX] =
{
	{ INPUT_DOWN, INPUT_DOWN, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT },
	{ INPUT_LEFT, INPUT_UP, INPUT_RIGHT, INPUT_DOWN, INPUT_CANCEL },
	{ INPUT_CANCEL, INPUT_LEFT, INPUT_CANCEL, INPUT_LEFT, INPUT_CANCEL },
};

#define TILE_ID_ENEMY_START 0x10
#define TILE_ID_ENEMY_END (0x10 + ARRAYNUM(enemy_table))
#define TILE_ID_ITEM_START 0x40
#define TILE_ID_ITEM_END (0x40 + ARRAYNUM(item_table))
#define TILE_ID_ETHER_START 0x60
#define TILE_ID_ETHER_END (0x60 + ARRAYNUM(ether_table))
#define TILE_ID_WALL_START 0x80
#define TILE_IS_ENEMY(x) ((x) >= TILE_ID_ENEMY_START && (x) < TILE_ID_ENEMY_END)
#define TILE_IS_ITEM(x) ((x) >= TILE_ID_ITEM_START && (x) < TILE_ID_ITEM_END)
#define TILE_IS_ETHER(x) ((x) >= TILE_ID_ETHER_START && (x) < TILE_ID_ETHER_END)
#define TILE_IS_WALL(x) ((x) >= TILE_ID_WALL_START)
#define TILE_ENEMYID(x) ((x) - TILE_ID_ENEMY_START)
#define TILE_ITEMID(x) ((x) - TILE_ID_ITEM_START)
#define TILE_ETHERID(x) ((x) - TILE_ID_ETHER_START)

rgb_t col_black = { 0, 0, 0 };
rgb_t col_red = { UINT8_MAX, 0, 0 };

static rgb_t pal1[] =
{
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
};

static rgb_t pal2[] =
{
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0xFF, 0xFF, 0xFF },
	{ 0x80, 0x00, 0x80 },
	{ 0x00, 0x80, 0x00 },
	{ 0x00, 0x80, 0x80 },
	{ 0x80, 0x80, 0x00 },
	{ 0x80, 0x80, 0x80 },
	{ 0xC0, 0xC0, 0xC0 },
	{ 0x00, 0x00, 0xFF },
	{ 0xFF, 0x00, 0x00 },
	{ 0xFF, 0x00, 0xFF },
	{ 0x00, 0xFF, 0x00 },
	{ 0x00, 0xFF, 0xFF },
	{ 0xFF, 0xFF, 0x00 },
	{ 0xFF, 0xFF, 0xFF },
};

static rgb_t pal3[] =
{
	{ 0x10, 0x00, 0x10 },
	{ 0x00, 0x00, 0x20 },
	{ 0x20, 0x00, 0x00 },
	{ 0x20, 0x00, 0x20 },
	{ 0x00, 0x20, 0x00 },
	{ 0x00, 0x20, 0x20 },
	{ 0x20, 0x20, 0x00 },
	{ 0x20, 0x20, 0x20 },
	{ 0x30, 0x30, 0x30 },
	{ 0x00, 0x00, 0x40 },
	{ 0x40, 0x00, 0x00 },
	{ 0x40, 0x00, 0x40 },
	{ 0x00, 0x40, 0x00 },
	{ 0x00, 0x40, 0x40 },
	{ 0x40, 0x40, 0x00 },
	{ 0x40, 0x40, 0x40 },
};

static rgb_t pal4[] =
{
	{ 0x40, 0x00, 0x40 },
	{ 0x00, 0x00, 0x00 },
	{ 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00 },
};

static rgb_t pal5[] =
{
	{ 0x00, 0x00, 0x00 },
	{ 0x11, 0x11, 0x11 },
	{ 0x22, 0x22, 0x22 },
	{ 0x33, 0x33, 0x33 },
	{ 0x44, 0x44, 0x44 },
	{ 0x55, 0x55, 0x55 },
	{ 0x66, 0x66, 0x66 },
	{ 0x77, 0x77, 0x77 },
	{ 0x88, 0x88, 0x88 },
	{ 0x99, 0x99, 0x99 },
	{ 0xAA, 0xAA, 0xAA },
	{ 0xBB, 0xBB, 0xBB },
	{ 0xCC, 0xCC, 0xCC },
	{ 0xDD, 0xDD, 0xDD },
	{ 0xEE, 0xEE, 0xEE },
	{ 0xFF, 0xFF, 0xFF },
};

#define ___ 0

static uint8 title_art[] =
{
	___,___,___,___,0x1,0x1,0x1,0x6,___,0x1,0x6,___,___,0x1,0x1,0x1,0x6,___,0x1,0x1,0x1,0x6,___,0x1,0x1,0x6,___,___,0x1,0x6,___,___,___,___,___,___,___,___,___,___,
	___,___,___,___,___,0x2,0x6,___,___,0x2,0x6,___,0x2,0x6,___,___,___,___,0x2,0x6,___,___,___,0x2,0x6,0x2,0x6,___,0x2,0x6,___,___,___,___,___,___,___,___,___,___,
	___,___,___,___,___,0x3,0x6,___,___,0x3,0x6,___,0x3,0x6,0x3,0x3,0x6,___,0x3,0x3,0x6,___,___,0x3,0x3,0x6,___,___,0x3,0x6,___,___,___,___,___,___,___,___,___,___,
	___,___,___,___,___,0x4,0x6,___,___,0x4,0x6,___,0x4,0x6,___,0x4,0x6,___,0x4,0x6,___,___,___,0x4,0x6,0x4,0x6,___,___,___,___,___,___,___,___,___,___,___,___,___,
	___,___,___,___,___,0x5,0x6,___,___,0x5,0x6,___,___,0x5,0x5,0x6,___,___,0x5,0x5,0x5,0x6,___,0x5,0x6,0x5,0x6,___,0x5,0x6,___,___,___,___,___,___,___,___,___,___,
	___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,
	___,___,___,___,___,___,___,___,___,___,0x7,0x7,0x7,0xC,___,0x7,0xC,___,___,0x7,0x7,0x7,0xC,___,0x7,0x7,0x7,0xC,___,0x7,0x7,0xC,___,___,0x7,0xC,___,___,___,___,
	___,___,___,___,___,___,___,___,___,___,___,0x8,0xC,___,___,0x8,0xC,___,0x8,0xC,___,___,___,___,0x8,0xC,___,___,___,0x8,0xC,0x8,0xC,___,0x8,0xC,___,___,___,___,
	___,___,___,___,___,___,___,___,___,___,___,0x9,0xC,___,___,0x9,0xC,___,0x9,0xC,0x9,0x9,0xC,___,0x9,0x9,0xC,___,___,0x9,0x9,0xC,___,___,0x9,0xC,___,___,___,___,
	___,___,___,___,___,___,___,___,___,___,___,0xA,0xC,___,___,0xA,0xC,___,0xA,0xC,___,0xA,0xC,___,0xA,0xC,___,___,___,0xA,0xC,0xA,0xC,___,___,___,___,___,___,___,
	___,___,___,___,___,___,___,___,___,___,___,0xB,0xC,___,___,0xB,0xC,___,___,0xB,0xB,0xC,___,___,0xB,0xB,0xB,0xC,___,0xB,0xC,0xB,0xC,___,0xB,0xC,___,___,___,___,
	___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,
};

static uint8 clear_art[] =
{
	___,0x1,0x1,0xD,0x1,0x1,0x1,0xD,___,0x1,0xD,___,___,0x1,0x1,0xD,0x1,0x1,0x1,0xD,___,0x7,0x7,0xE,0x7,0xE,___,___,0x7,0x7,0x7,0xE,___,0x7,0xE,___,0x7,0x7,0xE,___,
	0x2,0xD,___,___,___,0x2,0xD,___,0x2,0xD,0x2,0xD,0x2,0xD,___,___,0x2,0xD,___,___,0x8,0xE,___,___,0x8,0xE,___,___,0x8,0xE,___,___,0x8,0xE,0x8,0xE,0x8,0xE,0x8,0xE,
	___,0x3,0xD,___,___,0x3,0xD,___,0x3,0x3,0x3,0xD,0x3,0xD,0x3,0xD,0x3,0x3,0xD,___,0x9,0xE,___,___,0x9,0xE,___,___,0x9,0x9,0xE,___,0x9,0x9,0x9,0xE,0x9,0x9,0xE,___,
	___,___,0x4,0xD,___,0x4,0xD,___,0x4,0xD,0x4,0xD,0x4,0xD,0x4,0xD,0x4,0xD,___,___,0xA,0xE,___,___,0xA,0xE,___,___,0xA,0xE,___,___,0xA,0xE,0xA,0xE,0xA,0xE,0xA,0xE,
	0x5,0x5,0xD,___,___,0x5,0xD,___,0x5,0xD,0x5,0xD,___,0x5,0x5,0xD,0x5,0x5,0x5,0xD,___,0xB,0xB,0xE,0xB,0xB,0xB,0xE,0xB,0xB,0xB,0xE,0xB,0xE,0xB,0xE,0xB,0xE,0xB,0xE,
};

static uint8 OverArt[] =
{
	___,___,___,___,___,0x1,0x1,0xD,___,0x1,0xD,___,0x1,0xD,0x1,0xD,0x1,0x1,0x1,0xD,___,0x7,0xE,___,0x7,0xE,0x7,0xE,0x7,0x7,0x7,0xE,0x7,0x7,0xE,___,___,___,___,___,
	___,___,___,___,0x2,0xD,___,___,0x2,0xD,0x2,0xD,0x2,0x2,0x2,0xD,0x2,0xD,___,___,0x8,0xE,0x8,0xE,0x8,0xE,0x8,0xE,0x8,0xE,___,___,0x8,0xE,0x8,0xE,___,___,___,___,
	___,___,___,___,0x3,0xD,0x3,0xD,0x3,0x3,0x3,0xD,0x3,0xD,0x3,0xD,0x3,0x3,0xD,___,0x9,0xE,0x9,0xE,0x9,0xE,0x9,0xE,0x9,0x9,0xE,___,0x9,0x9,0xE,___,___,___,___,___,
	___,___,___,___,0x4,0xD,0x4,0xD,0x4,0xD,0x4,0xD,0x4,0xD,0x4,0xD,0x4,0xD,___,___,0xA,0xE,0xA,0xE,0xA,0xE,0xA,0xE,0xA,0xE,___,___,0xA,0xE,0xA,0xE,___,___,___,___,
	___,___,___,___,___,0x5,0x5,0xD,0x5,0xD,0x5,0xD,0x5,0xD,0x5,0xD,0x5,0x5,0x5,0xD,___,0xB,0xE,___,___,0xB,0xE,___,0xB,0xB,0xB,0xE,0xB,0xE,0xB,0xE,___,___,___,___,
};

#undef ___

static void number_to_text(char* buffer, uint32 number, uint8 ez_mode)
{
	if (number & EZ_MODE_FLAG)
	{
		ez_mode = 1;
		number &= ~EZ_MODE_FLAG;
	}

	for (uint32 i = 0; i < 6; ++i)
	{
		if (number > 0 || i == 0)
		{
			buffer[5 - i] = '0' + (number % 10);
		}
		else
		{
			buffer[5 - i] = ' ';
		}

		number /= 10;
	}

	if (ez_mode)
	{
		buffer[6] = '.';
		buffer[7] = '\0';
	}
	else
	{
		buffer[6] = '\0';
	}
}

static const void* arc_read(const arc_t* arc, uint32 idx)
{
	uint64 offs = ALIGNUP(arc->data_start + sizeof(arc_t), 16);

	if (idx > 0)
	{
		offs += arc->toc[idx - 1];
	}

	return MEMOFFS(arc, offs);
}

static uint32 arc_size(const arc_t* arc, uint32 idx)
{
	return (const uint8*)arc_read(arc, idx + 1) - (const uint8*)arc_read(arc, idx);
}

static const char* path_combine(const char* root, const char* file)
{
	static char buffer[512];

	buffer[0] = '\0';

	strcat(buffer, root);
	strcat(buffer, file);

	return buffer;
}

uint8 game_init(game_t* game, const char* path)
{
	memset(game, 0, sizeof(game_t));

	game->arc = file_read(path_combine(path, "minigame/tora.bin"), NULL);

	if (!game->arc)
	{
		return 0;
	}

	game->rand_seed = time(NULL);

	gpu_init(&game->gpu);
	sound_init(&game->sound_sys);

	sound_bgm_load(&game->sound_sys, 0, path_combine(path, "stream/bgm/m102a.nop"));
	sound_bgm_load(&game->sound_sys, 1, path_combine(path, "stream/bgm/m102b.nop"));
	sound_bgm_load(&game->sound_sys, 2, path_combine(path, "stream/bgm/m102c.nop"));
	sound_bgm_load(&game->sound_sys, 3, path_combine(path, "stream/bgm/m102d.nop"));

	sound_se_load(&game->sound_sys, path_combine(path, "sound/se/minigame.xsp"));

	uint32 task_size = 0;

	task_size = MAX(task_size, sizeof(player_task_t));
	task_size = MAX(task_size, sizeof(enemy_task_t));
	task_size = MAX(task_size, sizeof(item_task_t));
	task_size = MAX(task_size, sizeof(effect_task_t));

	task_init(&game->task_sys, task_size, TASK_MAX);

	size_t save_data_size;
	save_t* save_file = file_read("save.bin", &save_data_size);

	if (save_data_size == sizeof(save_t))
	{
		memcpy(&game->save, save_file, sizeof(save_t));
	}
	else
	{
		game->save.clear_stage = 0;
		game->save.leaderboard[0].score = 3000;
		strcpy(game->save.leaderboard[0].name, "MON");
		game->save.leaderboard[1].score = 3000;
		strcpy(game->save.leaderboard[1].name, "YJM");
		game->save.leaderboard[2].score = 3000;
		strcpy(game->save.leaderboard[2].name, "TNY");
		game->save.leaderboard[3].score = 3000;
		strcpy(game->save.leaderboard[3].name, "KWN");
		game->save.leaderboard[4].score = 3000;
		strcpy(game->save.leaderboard[4].name, "HAG");
	}

	free(save_file);

	game->phase = PHASE_BOOT;

	return 1;
}

void game_fini(game_t* game)
{
	free(game->stage);
	free(game->stage_img);
	free(game->arc);

	task_fini(&game->task_sys);
	sound_fini(&game->sound_sys);
	gpu_fini(&game->gpu);

	memset(game, 0, sizeof(game_t));
}

uint8 game_update(game_t* game, uint32 input)
{
	game->frame_num++;
	game->input_down = input & ~game->input_hold;
	game->input_hold = input;

	switch (game->phase)
	{
		case PHASE_BOOT:
			game_phase_boot(game);
			break;
		case PHASE_TITLE:
			game_phase_title(game);
			break;
		case PHASE_SELECT:
			game_phase_select(game);
			break;
		case PHASE_PLAY:
			game_phase_play(game);
			break;
		case PHASE_CLEAR:
			game_phase_clear(game);
			break;
		case PHASE_OVER:
			game_phase_over(game);
			break;
		case PHASE_NAME:
			game_phase_name(game);
			break;
	}

	gpu_update(&game->gpu);
	sound_update(&game->sound_sys);

	return game->phase < PHASE_EXIT;
}

void game_phase_boot(game_t* game)
{
	switch (game->step)
	{
		case BOOT_STEP_INIT:
		{
			gpu_reset(&game->gpu);
			gpu_fg_clear(&game->gpu);
			gpu_bg_clear(&game->gpu);

			const uint8* font_data = arc_read(game->arc, FILE_FONT);

			for (uint16 tile_idx = 0; tile_idx < 128; ++tile_idx)
			{
				for (uint8 row_idx = 0; row_idx < GPU_TILE_WH; ++row_idx)
				{
					uint8 font_val = *font_data++;
					uint32 row_val = 0;

					for (uint32 k = 0; k < GPU_TILE_WH; ++k)
					{
						row_val |= ((font_val & (1U << k)) ? 2 : 1) << (GPU_BPP * k);
					}

					gpu_tile_set_row(&game->gpu, tile_idx, row_idx, row_val);
				}
			}

			for (uint8 row_idx = 0; row_idx < GPU_TILE_WH; ++row_idx)
			{
				gpu_tile_set_row(&game->gpu, 0, row_idx, 0);
			}

			gpu_pal_set(&game->gpu, 0, pal1);
			gpu_pal_set(&game->gpu, 1, pal2);

			game_fadeset(game, UINT8_MAX);

			game_fg_draw_text(game, 4, 4, 0, "TIGER! TIGER!");
			game_fg_draw_text(game, 4, 8, 0, "ROM CHECK :");
			game_fg_draw_text(game, 4, 10, 0, "RAM CHECK :");
			game_fg_draw_text(game, 4, 12, 0, "BACKUP    :");

			game->title.credit = 9;
			game->select_idx = 0;
			game->step = BOOT_STEP_ROM_CHECK;

			break;
		}
		case BOOT_STEP_ROM_CHECK:
		{
			game_fg_draw_text(game, 16, 8, 0, "OK");

			game->step = BOOT_STEP_RAM_CHECK;

			break;
		}
		case BOOT_STEP_RAM_CHECK:
		{
			if (game->select_idx < 16)
			{
				char hex = "0123456789ABCDEF"[game->select_idx];

				game_fg_draw_tile(game, 16, 10, 1, hex, 1, 1);
				game_fg_draw_tile(game, 17, 10, 1, hex, 1, 1);

				game->select_idx++;
			}
			else
			{
				game_fg_draw_text(game, 16, 10, 0, "OK");

				game->select_idx = 0;
				game->step = BOOT_STEP_BACKUP;
			}

			break;
		}
		case BOOT_STEP_BACKUP:
		{
			if (game->select_idx < 8)
			{
				game->select_idx++;
			}
			else
			{
				game_fg_draw_text(game, 16, 12, 0, "OK");

				game->select_idx = 0;
				game->step = BOOT_STEP_READY;
			}

			break;
		}
		case BOOT_STEP_READY:
		{
			if (game->select_idx < 16)
			{
				game->select_idx++;
			}
			else
			{
				game->phase = PHASE_TITLE;
				game->step = TITLE_STEP_INIT;

				game_bgm_play(game, 0);
			}

			break;
		}
	}
}

void game_phase_title(game_t* game)
{
	switch (game->step)
	{
		case TITLE_STEP_INIT:
		{
			gpu_reset(&game->gpu);
			gpu_fg_clear(&game->gpu);

			gpu_obj_pal(&game->gpu, 16);

			game_load_sprite_file(game, 0x80, 0, FILE_IMAGE_TITLE);

			gpu_pal_col_set(&game->gpu, 0, 0, &col_black);

			for (uint8 y = 0; y < 12; ++y)
			{
				for (uint8 x = 0; x < 40; ++x)
				{
					game_fg_draw_tile(game, x, 2 + y, 0, 0x80 + title_art[MAD(y, 40, x)], 1, 1);
				}
			}

			game_fg_draw_text(game, 12, 27, 1, "*2017 MONOLITHSOFT");
			game_fg_draw_text(game, 12, 27, 1, "\x18");

			uint16 tile_idx = 0x1E0;

			tile_idx += game_load_sprite_file(game, tile_idx, 16, FILE_IMAGE_TORA);
			tile_idx += game_load_sprite_file(game, tile_idx, 17, FILE_IMAGE_AIR_BUBBLE);
			tile_idx += game_load_sprite_file(game, tile_idx, 18, FILE_IMAGE_ITEMS);
			tile_idx += game_load_sprite_file(game, tile_idx, 19, FILE_IMAGE_ETHER);
			tile_idx += game_load_sprite_file(game, tile_idx, 20, FILE_IMAGE_SHARK);
			tile_idx += game_load_sprite_file(game, tile_idx, 21, FILE_IMAGE_SEA_URCHIN);
			tile_idx += game_load_sprite_file(game, tile_idx, 22, FILE_IMAGE_TURTLE);
			tile_idx += game_load_sprite_file(game, tile_idx, 23, FILE_IMAGE_JELLYFISH);
			tile_idx += game_load_sprite_file(game, tile_idx, 24, FILE_IMAGE_SEA_SLUG);
			tile_idx += game_load_sprite_file(game, tile_idx, 25, FILE_IMAGE_MONKFISH);
			tile_idx += game_load_sprite_file(game, tile_idx, 29, FILE_IMAGE_SHARK_RED);

			gpu_pal_set(&game->gpu, 3, pal3);

			game_fadeset(game, 0);

			for (uint32 i = 0; i < ARRAYNUM(game->title.vm_objs); ++i)
			{
				game->title.vm_objs[i].pack_idx = 0;
			}

			game->title.vm_stream_idx = 0;
			game->title.vm_data = NULL;
			game->title.vm_stream = NULL;
			game->stage_idx = -1;
			game->step = TITLE_STEP_FADEIN;

			break;
		}
		case TITLE_STEP_FADEIN:
		{
			if (game_fadein(game))
			{
				game->select_idx = 0;
				game->step = TITLE_STEP_TITLE;
			}

			break;
		}
		case TITLE_STEP_TITLE:
		{
			if (!game->title.vm_stream)
			{
				const arc_t* vm_arc = arc_read(game->arc, FILE_TITLE_VM);

				game->title.vm_data = arc_read(vm_arc, game->title.vm_stream_idx);
				game->title.vm_stream = game->title.vm_data;
				game->title.vm_stream_idx = (game->title.vm_stream_idx + 1) % 3;
			}

			int8 new_select_idx = game->select_idx;

			if (game->input_down & INPUT_DOWN)
			{
				new_select_idx++;
			}
			else if (game->input_down & INPUT_UP)
			{
				new_select_idx--;
			}

			for (uint8 i = 0; i < 6; ++i)
			{
				uint8 input = 1U << i;

				if (game->input_down & input)
				{
					game->title.cheat_hist[game->title.cheat_hist_idx] = input;
					game->title.cheat_hist_idx = (game->title.cheat_hist_idx + 1) % CHEAT_INPUT_MAX;
				}
			}

			for (uint8 i = 0; i < ARRAYNUM(cheat_table); ++i)
			{
				uint8 match = 0;

				for (uint8 j = 0; j < CHEAT_INPUT_MAX; ++j)
				{
					if (game->title.cheat_hist[(game->title.cheat_hist_idx + j) % CHEAT_INPUT_MAX] == cheat_table[i][j])
					{
						match++;
					}
					else
					{
						break;
					}
				}

				if (match == CHEAT_INPUT_MAX)
				{
					memset(game->title.cheat_hist, 0, sizeof(game->title.cheat_hist));
					game->title.cheat_hist_idx = 0;

					if (i == CHEAT_STAGE_UNLOCK)
					{
						game->title.cheat_stage_unlock = 1;
						strcpy(game->title.cheat_name, "STAGE UNLOCK");
					}
					else if (i == CHEAT_GODMODE)
					{
						game->title.cheat_god_mode = 1;
						strcpy(game->title.cheat_name, "GOD MODE");
					}
					else if (i == CHEAT_NOCLIP)
					{
						game->title.cheat_no_clip = 1;
						strcpy(game->title.cheat_name, "NO CLIP");
					}

					game->title.cheat_name_counter = 30;

					break;
				}
			}

			if (game->title.cheat_name_counter > 0)
			{
				game->title.cheat_name_counter--;

				if (game->title.cheat_name_counter == 0)
				{
					game_fg_clear(game, 21, 29, 16, 1);
				}
				else
				{
					game_fg_draw_text(game, 21, 29, 1, game->title.cheat_name);
				}
			}

			if (game->title.credit == 0)
			{
				new_select_idx = 2;
			}

			new_select_idx = CLAMP(new_select_idx, 0, 2);

			if (game->select_idx != new_select_idx)
			{
				game->select_idx = new_select_idx;

				game_se_play(game, 10);
			}

			for (uint8 i = 0; i <= 2; ++i)
			{
				if (game->select_idx == i)
				{
					game_fg_draw_text(game, 16, 21 + (i * 2), 1, ">");
				}
				else
				{
					game_fg_clear(game, 16, 21 + (i * 2), 1, 1);
				}
			}

			if (game->title.credit > 0)
			{
				game_fg_draw_text(game, 18, 21, 1, "NORMAL");
				game_fg_draw_text(game, 18, 23, 1, "EASY");
			}

			game_fg_draw_text(game, 18, 25, 1, "END");

			if (game->input_down & INPUT_CONFIRM)
			{
				if (game->select_idx == 2 || game->title.credit > 0)
				{
					game_se_play(game, 11);

					if (game->select_idx != 2)
					{
						game->title.credit--;
					}

					game->step = TITLE_STEP_FADEOUT;
				}
				else
				{
					game_se_play(game, 12);
				}
			}

			break;
		}
		case TITLE_STEP_FADEOUT:
		{
			if (game_fadeout(game))
			{
				if (game->select_idx == 2)
				{
					game->phase = PHASE_EXIT;
				}
				else
				{
					if (game->stage_idx != UINT32_MAX)
					{
						game->phase = PHASE_PLAY;
						game->step = GAME_STEP_INIT;
					}
					else
					{
						game->phase = PHASE_SELECT;
						game->step = SELECT_STEP_INIT;
					}

					if (game->select_idx == 1)
					{
						game->play.ez_mode = 1;
					}
				}
			}

			break;
		}
	}

	if (game->title.credit == 0)
	{
		game_fg_draw_text(game, 2, 29, 1, "INSERT COIN");
	}
	else
	{
		if (game->title.credit > 1)
		{
			game_fg_draw_text(game, 2, 29, 1, "CREDITS   ");
		}
		else
		{
			game_fg_draw_text(game, 2, 29, 1, "CREDIT    ");
		}

		game_fg_draw_tile(game, 10, 29, 1, '0' + game->title.credit, 1, 1);
	}

	game_vm_update(game);
	game_gpu_obj_clear(game);

	for (uint32 i = 0; i < ARRAYNUM(game->title.vm_objs); ++i)
	{
		const vm_obj_t* vm_obj = &game->title.vm_objs[i];

		if (vm_obj->pack_idx != 0)
		{
			game_draw_sprite(game, vm_obj->pack_idx, vm_obj->sprite_idx, vm_obj->x >> 6, vm_obj->y >> 6, vm_obj->flip, vm_obj->prio);
		}
	}
}

void game_phase_select(game_t* game)
{
	switch (game->step)
	{
		case SELECT_STEP_INIT:
		{
			gpu_reset(&game->gpu);

			const image_t* select_img = arc_read(game->arc, FILE_IMAGE_STAGE_MAP);

			game_load_img(game, 0x300, select_img, 0, 128, 128);
			game_load_img_pal(game, 2, select_img);

			gpu_pal_set(&game->gpu, 1, pal1);

			uint16 tile_idx = 0x140;

			tile_idx += game_load_sprite_file(game, tile_idx, 3, FILE_IMAGE_STAGE_MARKER);
			tile_idx += game_load_sprite_file(game, tile_idx, 4, FILE_IMAGE_TORA);

			game_fg_draw_tile(game, 0, 0, 2, 0, GPU_FG_WH, GPU_FG_WH);
			game_bg_draw_tile(game, 0, 0, 2, 0, GPU_FG_WH, GPU_FG_WH);

			for (uint8 y = 0; y < 16; ++y)
			{
				for (uint8 x = 0; x < 16; ++x)
				{
					game_fg_draw_tile(game, 12 + x, 7 + y, 2, 0x300 + MAD(x, 16, y), 1, 1);
				}
			}

			game_fg_draw_text(game, 14, 2, 1, "STAGE SELECT");
			game_fg_draw_text(game, 17, 25, 1, "STAGE");

			game_fadeset(game, 0);

			game->select_idx = 0;
			game->stage_idx = UINT32_MAX;

			if (game->title.cheat_stage_unlock)
			{
				game->select.stage_num = 5;
			}
			else
			{
				game->select.stage_num = MIN(game->save.clear_stage + 1, 5);
			}

			csvb_t csvb;

			csvb_init(&csvb, arc_read(game->arc, FILE_CSVB_STAGE_SELECT), arc_size(game->arc, FILE_CSVB_STAGE_SELECT));

			for (uint32 i = 0; i < csvb_row_count(&csvb); ++i)
			{
				game->select.marker_pos[i][0] = csvb_read_int(&csvb, i, 0);
				game->select.marker_pos[i][1] = csvb_read_int(&csvb, i, 1);
			}

			csvb_fini(&csvb);

			game->select.tora_pos[0] = 64;
			game->select.tora_pos[1] = 144;

			game->step = SELECT_STEP_FADEIN;

			break;
		}
		case SELECT_STEP_FADEIN:
		{
			if (game_fadein(game))
			{
				game->select_idx = 0;
				game->step = SELECT_STEP_SELECT;
			}

			break;
		}
		case SELECT_STEP_SELECT:
		{
			if (game->input_down & (INPUT_RIGHT | INPUT_LEFT | INPUT_DOWN | INPUT_UP))
			{
				int8 best_idx = -1;
				int32 best_dist = 999;

				for (uint8 i = 0; i < game->select.stage_num; ++i)
				{
					int32 dist = 0;

					if (game->input_down & INPUT_RIGHT)
					{
						dist = game->select.marker_pos[i][0] - game->select.marker_pos[game->select_idx][0];
					}
					else if (game->input_down & INPUT_LEFT)
					{
						dist = game->select.marker_pos[game->select_idx][0] - game->select.marker_pos[i][0];
					}
					else if (game->input_down & INPUT_DOWN)
					{
						dist = game->select.marker_pos[i][1] - game->select.marker_pos[game->select_idx][1];
					}
					else if (game->input_down & INPUT_UP)
					{
						dist = game->select.marker_pos[game->select_idx][1] - game->select.marker_pos[i][1];
					}

					if (game->select_idx != i && dist >= 0 && dist < best_dist)
					{
						best_idx = i;
						best_dist = dist;
					}
				}

				if (best_idx != -1)
				{
					game->select_idx = best_idx;

					game_se_play(game, 10);
				}
			}
			else if (game->input_down & INPUT_CONFIRM)
			{
				game_se_play(game, 11);

				game->step = SELECT_STEP_FADEOUT;
			}
			else if (game->input_down & INPUT_CANCEL)
			{
				game->phase = PHASE_TITLE;
				game->step = TITLE_STEP_INIT;
			}

			break;
		}
		case SELECT_STEP_FADEOUT:
		{
			if (game_fadeout(game))
			{
				game->stage_idx = game->select_idx;
				game->phase = PHASE_PLAY;
				game->step = GAME_STEP_INIT;
			}

			break;
		}
	}

	game_fg_draw_tile(game, 23, 25, 1, '1' + game->select_idx, 1, 1);

	gpu_obj_scroll_x(&game->gpu, 96);
	gpu_obj_scroll_y(&game->gpu, 56);

	game_gpu_obj_clear(game);

	for (uint32 i = 0; i < game->select.stage_num; ++i)
	{
		game_draw_sprite(game, FILE_IMAGE_STAGE_MARKER, 0, game->select.marker_pos[i][0], game->select.marker_pos[i][1], 0, GPU_SPRITE_PRIO_FG);
	}

	int16 move[2];

	move[0] = game->select.marker_pos[game->select_idx][0] - game->select.tora_pos[0];
	move[1] = game->select.marker_pos[game->select_idx][1] - 20 - game->select.tora_pos[1];

	if (abs(move[0]) > 2)
	{
		move[0] = SIGN(move[0]) * abs(move[0]) / 2;
	}

	if (abs(move[1]) > 2)
	{
		move[1] = SIGN(move[1]) * abs(move[1]) / 2;
	}

	game->select.tora_pos[0] += move[0];
	game->select.tora_pos[1] += move[1];

	game_draw_sprite(game, FILE_IMAGE_TORA, 2 + ((game->frame_num / 16) & 0x1), game->select.tora_pos[0], game->select.tora_pos[1], 0, GPU_SPRITE_PRIO_FG + 0x10);
}

void game_phase_play(game_t* game)
{
	switch (game->step)
	{
		case GAME_STEP_INIT:
		{
			gpu_reset(&game->gpu);
			gpu_fg_clear(&game->gpu);
			gpu_obj_pal(&game->gpu, 16);

			game->stage = malloc(0x10000);
			game->stage_img = malloc(0x100000);

			const arc_t* stage_arc = arc_read(game->arc, FILE_ARCHIVE_STAGE + game->stage_idx);
			csvb_t stage_csvb;

			csvb_init(&stage_csvb, arc_read(stage_arc, 0), arc_size(stage_arc, 0));

			uint32 stage_cursor = 0;
			uint32 stage_img_cursor = 0;

			game->stage->height = 0;
			game->stage_img->tile_num = 0;

			for (uint32 i = 0; i < csvb_row_count(&stage_csvb); ++i)
			{
				int8 chance = game_rand_max(game, 100);
				const char* chunk_name = NULL;

				for (uint32 j = 0; j < csvb_column_count(&stage_csvb, i); j += 2)
				{
					chance -= csvb_read_int(&stage_csvb, i, j + 1);

					if (chance < 0)
					{
						chunk_name = csvb_read_string(&stage_csvb, i, j);

						break;
					}
				}

				if (chunk_name)
				{
					uint32 chunk_file_idx = 1;

					while (1)
					{
						const stage_chunk_t* iter_chunk = arc_read(stage_arc, chunk_file_idx);
						char iter_chunk_name[16];

						strcpy(iter_chunk_name, iter_chunk->name);

						char* ext = strchr(iter_chunk_name, '.');

						if (ext)
						{
							*ext = '\0';
						}

						if (!strcmp(iter_chunk_name, chunk_name))
						{
							break;
						}

						chunk_file_idx += 2;
					}

					const stage_chunk_t* chunk = arc_read(stage_arc, chunk_file_idx);
					const stage_t* chunk_data = MEMOFFS(chunk, sizeof(stage_chunk_t));
					const image_t* chunk_img = arc_read(stage_arc, chunk_file_idx + 1);
					uint32 chunk_data_size = arc_size(stage_arc, chunk_file_idx) - sizeof(stage_chunk_t);
					uint32 chunk_img_size = arc_size(stage_arc, chunk_file_idx + 1);
					uint32 chunk_data_offs;
					uint32 chunk_img_offs;

					if (i == 0)
					{
						chunk_data_offs = 0;
						chunk_img_offs = 0;
					}
					else
					{
						chunk_data_offs = sizeof(stage_t);
						chunk_img_offs = sizeof(image_t) + chunk_img->pal_size;
					}

					memcpy(MEMOFFS(game->stage, stage_cursor), MEMOFFS(chunk_data, chunk_data_offs), chunk_data_size - chunk_data_offs);
					memcpy(MEMOFFS(game->stage_img, stage_img_cursor), MEMOFFS(chunk_img, chunk_img_offs), chunk_img_size - chunk_img_offs);

					stage_cursor += chunk_data_size - chunk_data_offs;
					stage_img_cursor += chunk_img_size - chunk_img_offs;

					if (i != 0)
					{
						game->stage->height += chunk_data->height;
						game->stage_img->tile_num += chunk_img->tile_num;
					}
				}
			}

			csvb_fini(&stage_csvb);

			const stage_info_t* stage_info = &stage_table[game->stage_idx];

			game->play.scroll_down_spd = GAME2SCROLL(DRAW2GAME(stage_info->scroll_down_spd) / 100);
			game->play.scroll_up_spd = GAME2SCROLL(-DRAW2GAME(stage_info->scroll_up_spd) / 100);
			game->play.scroll_up_accel = GAME2SCROLL(-DRAW2GAME(stage_info->scroll_up_accel * 4) / 1000) / 4;
			game->play.scroll_up_spd_max = GAME2SCROLL(-DRAW2GAME(stage_info->scroll_up_spd_max) / 100);
			game->play.scroll_threshold = stage_info->scroll_threshold;
			game->play.star_rate = stage_info->star_rate;
			game->play.o2_rate = stage_info->o2_rate;
			game->play.o2_val = stage_info->o2_val;
			game->play.bonus_time = stage_info->bonus_time;
			game->play.bonus_uni_rate = stage_info->bonus_uni_rate;
			game->play.bonus_ether_rate = stage_info->bonus_ether_rate;
			game->play.bonus_entry_time = stage_info->bous_entry_time;
			game->play.scroll_spd = game->play.scroll_down_spd;
			game->play.scroll_y = 0;
			game->play.scroll_y_max = DRAW2SCROLL((game->stage->height * 16) - GPU_SCR_H);
			game->play.tbox_num = 0;
			game->play.ether_num = 0;

			const image_t* bg_img = arc_read(game->arc, FILE_IMAGE_STAGE_BACKGROUND + (game->stage_idx % 5));

			game_load_img(game, 0x180, bg_img, 0, 64, 64);
			game_load_img_pal(game, 3, bg_img);

			uint16 tile_idx = 0x1E0;

			tile_idx += game_load_sprite_file(game, tile_idx, 4, FILE_IMAGE_UI);
			tile_idx += game_load_sprite_file(game, tile_idx, 16, FILE_IMAGE_TORA);
			tile_idx += game_load_sprite_file(game, tile_idx, 17, FILE_IMAGE_AIR_BUBBLE);
			tile_idx += game_load_sprite_file(game, tile_idx, 18, FILE_IMAGE_ITEMS);
			tile_idx += game_load_sprite_file(game, tile_idx, 19, FILE_IMAGE_ETHER);
			tile_idx += game_load_sprite_file(game, tile_idx, 20, FILE_IMAGE_SHARK);
			tile_idx += game_load_sprite_file(game, tile_idx, 21, FILE_IMAGE_SEA_URCHIN);
			tile_idx += game_load_sprite_file(game, tile_idx, 22, FILE_IMAGE_TURTLE);
			tile_idx += game_load_sprite_file(game, tile_idx, 23, FILE_IMAGE_JELLYFISH);
			tile_idx += game_load_sprite_file(game, tile_idx, 24, FILE_IMAGE_SEA_SLUG);
			tile_idx += game_load_sprite_file(game, tile_idx, 25, FILE_IMAGE_MONKFISH);
			tile_idx += game_load_sprite_file(game, tile_idx, 26, FILE_IMAGE_EEL);
			tile_idx += game_load_sprite_file(game, tile_idx, 27, FILE_IMAGE_CRAB);
			tile_idx += game_load_sprite_file(game, tile_idx, 28, FILE_IMAGE_TENTACLE);
			tile_idx += game_load_sprite_file(game, tile_idx, 29, FILE_IMAGE_BONUS);
			tile_idx += game_load_sprite_file(game, tile_idx, 30, FILE_IMAGE_SHARK_RED);

			gpu_pal_set(&game->gpu, 0, pal4);
			gpu_pal_set(&game->gpu, 1, pal2);
			gpu_pal_set(&game->gpu, 2, pal5);

			game_fadeset(game, UINT8_MAX);

			gpu_overlay_set(&game->gpu, MEMOFFS(game->stage_img, sizeof(image_t) + game->stage_img->pal_size), MEMOFFS(game->stage_img, sizeof(image_t)), PLAY_AREA_W);

			for (uint8 y = 0; y < GPU_BG_WH; ++y)
			{
				for (uint8 x = 0; x < GPU_BG_WH; ++x)
				{
					game_bg_draw_tile(game, x, y, 3, 0x180 + MAD(x % 8, 8, y % 8), 1, 1);
				}
			}

			game_ui_draw_tile(game, 0, 0, 0, 0x20, GPU_UI_W, GPU_UI_H);

			game_ui_draw_text(game, 33, 1, 1, "SCORE");
			game_ui_draw_text(game, 33, 4, 1, "ETHER");
			game_ui_draw_text(game, 33, 7, 1, "AIR");

			game_ui_draw_sprite(game, 32, 8, FILE_IMAGE_UI, 10, 1, 1);
			game_ui_draw_sprite(game, 33, 8, FILE_IMAGE_UI, 8, 6, 1);
			game_ui_draw_sprite(game, 39, 8, FILE_IMAGE_UI, 9, 1, 1);

			game_ui_draw_sprite(game, 33, 16, FILE_IMAGE_UI, 13, 6, 1);
			game_ui_draw_sprite(game, 33, 28, FILE_IMAGE_UI, 13 | GPU_UI_FLIP_Y, 6, 1);
			game_ui_draw_sprite(game, 32, 17, FILE_IMAGE_UI, 14, 1, 11);
			game_ui_draw_sprite(game, 39, 17, FILE_IMAGE_UI, 14 | GPU_UI_FLIP_X, 1, 11);
			game_ui_draw_sprite(game, 32, 16, FILE_IMAGE_UI, 15, 1, 1);
			game_ui_draw_sprite(game, 39, 16, FILE_IMAGE_UI, 15 | GPU_UI_FLIP_X, 1, 1);
			game_ui_draw_sprite(game, 32, 28, FILE_IMAGE_UI, 15 | GPU_UI_FLIP_Y, 1, 1);
			game_ui_draw_sprite(game, 39, 28, FILE_IMAGE_UI, 15 | GPU_UI_FLIP_X | GPU_UI_FLIP_Y, 1, 1);

			gpu_ui_size_x(&game->gpu, -8);
			gpu_ui_size_y(&game->gpu, 0);

			game_gpu_obj_clear(game);

			const player_info_t* player_info = &player_table[0];
			player_task_t* player = game_obj_create(game);

			player->obj.task.move_cb = game_move_player;
			player->obj.task.draw_cb = game_draw_player;
			player->obj.flip = 0;
			player->obj.x = DRAW2GAME(128);
			player->obj.y = DRAW2GAME(64);
			player->obj.hit_flag = HIT_FLAG_ALL;
			player->obj.hit_by_flag = HIT_FLAG_NONE;
			player->obj.hit_x = -player_info->hix_w;
			player->obj.hit_y = -player_info->hit_h;
			player->obj.hit_w = player_info->hix_w * 2;
			player->obj.hit_h = player_info->hit_h * 2;
			player->move_type = PLAYER_MOVE_TYPE_DIVE;
			player->dmg_counter = 0;
			player->sprite_idx = 2;
			player->counter = 0;
			player->scroll = 0;
			player->atk_counter = 0;
			player->atk_cancel = 0;
			player->invuln_counter = 0;
			player->move_dir = PLAYER_MOVE_DIR_S;
			player->spd_u = DRAW2GAME(player_info->spd_u) / 100;
			player->spd_lr = DRAW2GAME(player_info->spd_lr) / 100;
			player->spd_d = DRAW2GAME(player_info->spd_d) / 100;
			player->scroll_threshold = game->play.scroll_threshold;
			player->atk_x = 0;
			player->atk_y = 0;
			player->atk_w = player_info->atk_w;
			player->atk_h = player_info->atk_h;

			game->play.player = player;

			uint32 stage_obj_num = 0;

			for (uint16 y = 0; y < game->stage->height; ++y)
			{
				for (uint16 x = 0; x < game->stage->width; ++x)
				{
					uint8 tile = game_get_stage_tile(game, x, y);

					if (tile)
					{
						if (TILE_IS_ENEMY(tile) || TILE_IS_ITEM(tile) || TILE_IS_ETHER(tile))
						{
							stage_obj_t* stage_obj = &game->play.stage_objs[stage_obj_num++];

							stage_obj->tile = tile;
							stage_obj->x = (x * 16) + 8;
							stage_obj->y = (y * 16) + 8;

							if (TILE_IS_ITEM(tile))
							{
								if (item_table[TILE_ITEMID(tile)].type != ITEM_TYPE_WEAPON)
								{
									game->play.tbox_num++;
								}
							}
							else if (TILE_IS_ETHER(tile))
							{
								game->play.ether_num++;
							}
						}
					}
				}
			}

			game->play.stage_objs[stage_obj_num].tile = 0;
			game->play.stage_tbox = NULL;

			for (uint32 i = 0; i < STAGE_OBJ_MAX; ++i)
			{
				const stage_obj_t* stage_obj = &game->play.stage_objs[i];

				if (!stage_obj->tile)
				{
					break;
				}

				if (TILE_IS_ENEMY(stage_obj->tile))
				{
					const enemy_info_t* enemy_info = &enemy_table[TILE_ENEMYID(stage_obj->tile)];
					enemy_task_t* enemy = game_obj_create(game);

					enemy->obj.task.move_cb = game_move_enemy;
					enemy->obj.task.draw_cb = game_draw_enemy;
					enemy->obj.x = DRAW2GAME(1024);
					enemy->obj.y = DRAW2GAME(0);
					enemy->obj.flip = 0;
					enemy->obj.hit_flag = HIT_FLAG_ENEMY;
					enemy->obj.hit_by_flag = HIT_FLAG_NONE;
					enemy->obj.hit_x = -enemy_info->hit_w;
					enemy->obj.hit_y = -enemy_info->hit_h;
					enemy->obj.hit_w = enemy_info->hit_w * 2;
					enemy->obj.hit_h = enemy_info->hit_h * 2;
					enemy->tile = stage_obj->tile;
					enemy->counter = 0;
					enemy->move_state = ENEMY_MOVE_STATE_NONE;
					enemy->move_time = game_rand_max(game, UINT8_MAX);
					enemy->dmg_time = 0;
					enemy->abs_x = DRAW2GAME(stage_obj->x);
					enemy->abs_y = DRAW2GAME(stage_obj->y);
					enemy->spd_x = 0;
					enemy->spd_y = 0;
					enemy->start_x = enemy->abs_x;
					enemy->start_y = enemy->abs_y;
					enemy->spd = DRAW2GAME(enemy_info->spd) / 100;
					enemy->range = enemy_info->range;
					enemy->search = enemy_info->search;
					enemy->act_spd = DRAW2GAME(enemy_info->act_spd) / 100;
					enemy->hp = enemy_info->Hp;
					enemy->back_spd = DRAW2GAME(enemy_info->back_spd) / 100;
					enemy->back_time = enemy_info->back_time;
					enemy->atk = enemy_info->atk;
					enemy->score = enemy_info->score;
					enemy->move_type = enemy_info->move_type;
					enemy->hit_type = enemy_info->hit_type;

					static uint8 file_idx_table[] =
					{
						FILE_IMAGE_SHARK,
						FILE_IMAGE_SEA_URCHIN,
						FILE_IMAGE_TURTLE,
						FILE_IMAGE_JELLYFISH,
						FILE_IMAGE_SEA_SLUG,
						FILE_IMAGE_MONKFISH,
						FILE_IMAGE_EEL,
						FILE_IMAGE_CRAB,
						FILE_IMAGE_TENTACLE,
						FILE_IMAGE_BONUS,
						FILE_IMAGE_SHARK_RED,
						FILE_MAX,
					};

					enemy->file_idx = file_idx_table[enemy_info->res_idx];

					if (enemy->move_type == ENEMY_MOVE_TYPE_TENTACLE)
					{
						memset(&enemy->tentacle, 0, sizeof(enemy->tentacle));

						enemy->tentacle.seg[1].x = 16;
						enemy->tentacle.seg[2].x = 32;
						enemy->tentacle.seg[3].x = 48;
					}
				}
				else if (TILE_IS_ITEM(stage_obj->tile) || TILE_IS_ETHER(stage_obj->tile))
				{
					item_task_t* item = game_obj_create(game);

					item->obj.task.move_cb = game_move_item;
					item->obj.task.draw_cb = game_draw_item;
					item->obj.x = DRAW2GAME(1024);
					item->obj.y = DRAW2GAME(0);
					item->obj.flip = 0;
					item->obj.hit_flag = HIT_FLAG_ITEM;
					item->obj.hit_by_flag = HIT_FLAG_NONE;
					item->obj.hit_x = -10;
					item->obj.hit_y = -10;
					item->obj.hit_w = 20;
					item->obj.hit_h = 20;
					item->tile = stage_obj->tile;
					item->state = ITEM_STATE_SPAWN;
					item->counter = 0;
					item->abs_x = DRAW2GAME(stage_obj->x);
					item->abs_y = DRAW2GAME(stage_obj->y);

					if (TILE_IS_ITEM(stage_obj->tile))
					{
						item->score = 0;

						if (item_table[TILE_ITEMID(stage_obj->tile)].type == ITEM_TYPE_BIG_TBOX)
						{
							game->play.stage_tbox = item;

							item->obj.hit_w = 36;
							item->obj.hit_h = 36;
						}
					}
					else if (TILE_IS_ETHER(stage_obj->tile))
					{
						item->score = ether_table[TILE_ETHERID(stage_obj->tile)].score;
					}
				}
			}

			game->play.hp_anim_time = 0;
			game->play.score_anim_time = 0;
			game->play.hp_max = player_info->hp_max;
			game->play.hp = player_info->hp_max;
			game->play.hp_anim = player_info->hp_max;
			game->play.ether = 0;
			game->play.ether_bonus = 0;
			game->play.score = 0;
			game->play.score_anim = 0;

			for (uint32 i = 0; i < LOOT_MAX; ++i)
			{
				game->play.loot[i] = NULL;
			}

			game->play.weapon = NULL;
			game->play.counter = 60;
			game->step = GAME_STEP_DIVE_START;

			game_bgm_play(game, 1);

			break;
		}
		case GAME_STEP_DIVE:
		case GAME_STEP_SURFACE_START:
		case GAME_STEP_SURFACE:
		case GAME_STEP_DIVE_START:
		{
			if (!game->play.hp)
			{
				game->step = GAME_STEP_OVER;
				game->play.counter = 60;
				game->play.player->obj.hit_flag = HIT_FLAG_NONE;
			}
			else
			{
				switch (game->step)
				{
					case GAME_STEP_DIVE:
					{
						if (game->play.scroll_threshold)
						{
							if (game->play.player->scroll)
							{
								game->play.scroll_time = MIN(game->play.scroll_time + 1, 8);
							}
							else
							{
								game->play.scroll_time = MAX(game->play.scroll_time - 1, 0);
							}

							game->play.scroll_spd = ((game->play.scroll_time * game->play.player->spd_d) & 0xFFFFFF) << 5;
						}

						game->play.scroll_y += game->play.scroll_spd;

						if (game->play.scroll_y >= game->play.scroll_y_max)
						{
							game->play.player->scroll_threshold = 0;

							game->play.scroll_y = game->play.scroll_y_max;

							if (!game->play.stage_tbox || game->play.stage_tbox->state != ITEM_STATE_SPAWN)
							{
								game->play.player->sprite_idx = 8;
								game->play.player->obj.flip = 0;
								game->play.player->move_type = PLAYER_MOVE_TYPE_SURFACE;
								game->play.counter = 60;
								game->step = GAME_STEP_SURFACE_START;
							}
						}

						break;
					}
					case GAME_STEP_SURFACE_START:
					{
						game->play.counter--;

						if (game->play.counter == 1)
						{
							game->play.scroll_spd = game->play.scroll_up_spd;
							game->step = GAME_STEP_SURFACE;

							game_bgm_play(game, 2);
						}

						break;
					}
					case GAME_STEP_SURFACE:
					{
						game->play.scroll_y += game->play.scroll_spd;
						game->play.scroll_spd = MAX(game->play.scroll_spd + game->play.scroll_up_accel, game->play.scroll_up_spd_max);

						if (game->play.scroll_y < 0)
						{
							game_se_play(game, 15);

							game->play.player->obj.hit_flag = HIT_FLAG_NONE;
							game->play.player->sprite_idx = 8;
							game->play.player->obj.flip = 0;
							game->play.player->move_type = PLAYER_MOVE_TYPE_FLOAT;
							game->play.scroll_y = 0;
							game->play.counter = 60;
							game->step = GAME_STEP_CLEAR;
						}

						break;
					}
					case GAME_STEP_DIVE_START:
					{
						game->play.counter--;

						if (game->play.counter == 1)
						{
							game->play.scroll_spd = game->play.scroll_down_spd;
							game->step = GAME_STEP_DIVE;
						}

						break;
					}
				}

				gpu_fg_scroll_y(&game->gpu, SCROLL2DRAW(game->play.scroll_y));
				gpu_bg_scroll_y(&game->gpu, SCROLL2DRAW(game->play.scroll_y / 2));

				game_move(game);
			}

			break;
		}
		case GAME_STEP_OVER:
		case GAME_STEP_CLEAR:
		{
			if (game->step == GAME_STEP_CLEAR)
			{
				game->play.player->obj.y -= DRAW2GAME(4);
			}

			game->play.counter--;

			if (!game->play.counter)
			{
				game->step = GAME_STEP_FADEOUT;
			}

			game_move(game);

			break;
		}
		case GAME_STEP_FADEOUT:
		{
			game_move(game);

			if (game_fadeout(game))
			{
				task_clear(&game->task_sys);

				free(game->stage);
				free(game->stage_img);

				game->stage = NULL;
				game->stage_img = NULL;

				gpu_overlay_clear(&game->gpu);

				if (game->play.hp)
				{
					game->phase = PHASE_CLEAR;
					game->step = CLEAR_STEP_INIT;
					game->select_idx = game_rand_max(game, 4);
				}
				else
				{
					game->phase = PHASE_OVER;
					game->step = OVER_STEP_INIT;
					game->select_idx = game_rand_minmax(game, 4, 8);
				}
			}

			break;
		}
		case GAME_STEP_BONUS_START:
		{
			game_move(game);

			if (game_fadeout(game))
			{
				game->step = GAME_STEP_BONUS;
				game->bonus.time = game->play.bonus_time;
				game->bonus.end = 0;
				game->bonus.saved_x = GAME2DRAW(game->play.player->obj.x);
				game->bonus.saved_y = GAME2DRAW(game->play.player->obj.y);
				game->bonus.saved_scroll_threshold = game->play.player->scroll_threshold;
				game->play.player->obj.x = DRAW2GAME(BONUS_AREA_OFFS + (PLAY_AREA_W / 2));
				game->play.player->obj.y = DRAW2GAME(128);
				game->play.player->scroll_threshold = 0;
				game->play.player->invuln_counter = 0;

				gpu_obj_scroll_x(&game->gpu, -BONUS_AREA_OFFS);
				gpu_overlay_enable(&game->gpu, 0);

				enemy_task_t* bonus_enemy = game_obj_create(game);

				if (bonus_enemy)
				{
					bonus_enemy->obj.task.move_cb = game_move_enemy;
					bonus_enemy->obj.task.draw_cb = game_draw_enemy;
					bonus_enemy->obj.x = DRAW2GAME(BONUS_AREA_OFFS);
					bonus_enemy->obj.y = DRAW2GAME(0);
					bonus_enemy->obj.flip = 0;
					bonus_enemy->obj.hit_flag = HIT_FLAG_ENEMY;
					bonus_enemy->obj.hit_by_flag = HIT_FLAG_NONE;
					bonus_enemy->obj.hit_x = -8;
					bonus_enemy->obj.hit_y = -8;
					bonus_enemy->obj.hit_w = 16;
					bonus_enemy->obj.hit_h = 16;
					bonus_enemy->tile = TILE_ID_ENEMY_END - 1;
					bonus_enemy->counter = 0;
					bonus_enemy->move_type = ENEMY_MOVE_TYPE_NONE;
					bonus_enemy->move_state = ENEMY_MOVE_STATE_NONE;
					bonus_enemy->move_time = game_rand_max(game, UINT8_MAX);
					bonus_enemy->dmg_time = 0;
					bonus_enemy->abs_x = DRAW2GAME(BONUS_AREA_OFFS + (PLAY_AREA_W / 2));
					bonus_enemy->abs_y = SCROLL2GAME(game->play.scroll_y) + DRAW2GAME(32);
					bonus_enemy->spd_x = 0;
					bonus_enemy->spd_y = 0;
					bonus_enemy->start_x = bonus_enemy->abs_x;
					bonus_enemy->start_y = bonus_enemy->abs_y;
					bonus_enemy->spd = 0;
					bonus_enemy->range = 0;
					bonus_enemy->search = 0;
					bonus_enemy->act_spd = 0;
					bonus_enemy->hit_type = ENEMY_HIT_TYPE_NEVER;
					bonus_enemy->score = 0;
					bonus_enemy->file_idx = FILE_IMAGE_BONUS;
				}
			}

			break;
		}
		case GAME_STEP_BONUS:
		{
			if (game->bonus.time > 90 && !(game->bonus.time & 0x3))
			{
				enemy_task_t* bonus_enemy = game_obj_create(game);

				if (bonus_enemy)
				{
					bonus_enemy->obj.task.move_cb = game_move_enemy;
					bonus_enemy->obj.task.draw_cb = game_draw_enemy;
					bonus_enemy->obj.x = DRAW2GAME(BONUS_AREA_OFFS);
					bonus_enemy->obj.y = DRAW2GAME(0);
					bonus_enemy->obj.flip = 0;
					bonus_enemy->obj.hit_flag = HIT_FLAG_ENEMY;
					bonus_enemy->obj.hit_by_flag = HIT_FLAG_NONE;
					bonus_enemy->obj.hit_x = -8;
					bonus_enemy->obj.hit_y = -8;
					bonus_enemy->obj.hit_w = 16;
					bonus_enemy->obj.hit_h = 16;
					bonus_enemy->tile = TILE_ID_ENEMY_END - 1;
					bonus_enemy->counter = 0;
					bonus_enemy->move_type = ENEMY_MOVE_TYPE_BONUS_SPAWN;
					bonus_enemy->move_state = ENEMY_MOVE_STATE_BONUS_SPAWN;
					bonus_enemy->move_time = game_rand_max(game, UINT8_MAX);
					bonus_enemy->dmg_time = 0;
					bonus_enemy->abs_x = DRAW2GAME(BONUS_AREA_OFFS + (PLAY_AREA_W / 2));
					bonus_enemy->abs_y = SCROLL2GAME(game->play.scroll_y) + DRAW2GAME(32);
					bonus_enemy->spd_x = game_rand_minmax(game, -512, 512);
					bonus_enemy->spd_y = -1024;
					bonus_enemy->start_x = bonus_enemy->abs_x;
					bonus_enemy->start_y = bonus_enemy->abs_y;
					bonus_enemy->spd = 64;
					bonus_enemy->range = 0;
					bonus_enemy->search = 0;
					bonus_enemy->act_spd = 0;
					bonus_enemy->hit_type = ENEMY_HIT_TYPE_NEVER;

					if (game_rand_max(game, 10000) < game->play.bonus_uni_rate)
					{
						bonus_enemy->score = 0;
						bonus_enemy->file_idx = FILE_IMAGE_SEA_URCHIN;
					}
					else
					{
						bonus_enemy->score = game->play.bonus_ether_rate;
						bonus_enemy->file_idx = FILE_IMAGE_ETHER;
					}
				}
			}

			game->bonus.time--;

			if (!game->bonus.time || game->bonus.end)
			{
				game->step = GAME_STEP_BONUS_EXIT_START;
				game->play.player->obj.hit_flag = HIT_FLAG_NONE;
			}

			game_move(game);

			game_fadein(game);

			break;
		}
		case GAME_STEP_BONUS_EXIT_START:
		{
			game_move(game);

			if (game_fadeout(game))
			{
				game->step = GAME_STEP_BONUS_EXIT;
				game->play.player->obj.x = DRAW2GAME(game->bonus.saved_x);
				game->play.player->obj.y = DRAW2GAME(game->bonus.saved_y);
				game->play.player->scroll_threshold = game->bonus.saved_scroll_threshold;

				gpu_obj_scroll_x(&game->gpu, 0);
				gpu_overlay_enable(&game->gpu, 1);
			}

			break;
		}
		case GAME_STEP_BONUS_EXIT:
		{
			game_move(game);

			if (game_fadein(game))
			{
				game->step = game->bonus.saved_step;
				game->play.player->invuln_counter = 90;
				game->play.player->obj.hit_flag = HIT_FLAG_ALL;
			}

			break;
		}
	}

	if (game->play.score_anim_time)
	{
		game->play.score_anim += (game->play.score_anim_time + game->play.score - 1 - game->play.score_anim) / game->play.score_anim_time;
		game->play.score_anim_time--;
	}

	game->play.score_anim = MIN(game->play.score_anim, 999999);

	game_ui_draw_number(game, 33, 2, 1, game->play.score_anim, game->play.ez_mode);
	game_ui_draw_number(game, 33, 5, 1, game->play.ether + game->play.ether_bonus, 0);

	if (game->play.hp_anim_time)
	{
		game->play.hp_anim += (game->play.hp_anim_time + game->play.hp - 1 - game->play.hp_anim) / game->play.hp_anim_time;
		game->play.hp_anim_time--;
	}

	uint32 hp = MIN(game->play.hp, game->play.hp_max);
	uint32 hp_anim = CLAMP(game->play.hp_anim, game->play.hp, game->play.hp_max);
	int32 hp_ratio = 48 * hp / game->play.hp_max;
	int32 hp_anim_ratio = 48 * hp_anim / game->play.hp_max;

	if (hp > 0 && hp_ratio == 0)
	{
		hp_ratio++;
	}
	else if (hp < game->play.hp_max && hp_ratio == 48)
	{
		hp_ratio--;
	}

	if (hp_anim > 0 && hp_anim_ratio == 0)
	{
		hp_anim_ratio++;
	}
	else if (hp_anim < game->play.hp_max && hp_anim_ratio == 48)
	{
		hp_anim_ratio--;
	}

	for (uint8 seg = 0; seg < 6; ++seg)
	{
		int32 Row = CLAMP(hp_ratio, 0, 8);
		int32 Col = CLAMP(hp_anim_ratio - hp_ratio, 0, 8 - Row);

		static const uint8 sprite_table[][9] =
		{
			{  8, 23, 22, 21, 20, 19, 18, 17, 16 },
			{  7, 30, 29, 28, 27, 26, 25, 24,  0 },
			{  6, 36, 35, 34, 33, 32, 31,  0,  0 },
			{  5, 41, 40, 39, 38, 37,  0,  0,  0 },
			{  4, 45, 44, 43, 42,  0,  0,  0,  0 },
			{  3, 48, 47, 46,  0,  0,  0,  0,  0 },
			{  2, 50, 49,  0,  0,  0,  0,  0,  0 },
			{  1, 51,  0,  0,  0,  0,  0,  0,  0 },
			{  0,  0,  0,  0,  0,  0,  0,  0,  0 },
		};

		game_ui_draw_sprite(game, 33 + seg, 8, FILE_IMAGE_UI, sprite_table[Row][Col], 1, 1);

		hp_ratio = MAX(hp_ratio, 8) - 8;
		hp_anim_ratio = MAX(hp_anim_ratio, 8) - 8;
	}

	float pitch;

	if (game->play.hp && game->play.hp < (game->play.hp_max / 10))
	{
		pitch = 1.5f;
	}
	else
	{
		pitch = 1.0f;
	}

	game_bgm_pitch(game, pitch);
	game_gpu_obj_clear(game);

	task_draw(&game->task_sys, game);
}

void game_phase_clear(game_t* game)
{
	switch (game->step)
	{
		case CLEAR_STEP_INIT:
		{
			gpu_reset(&game->gpu);
			gpu_fg_clear(&game->gpu);
			gpu_bg_clear(&game->gpu);

			game_bgm_play(game, 3);

			game_load_sprite_file(game, 0x1E0, 7, FILE_IMAGE_REWARDS);

			gpu_pal_set(&game->gpu, 1, pal2);

			game_cell_anime_init(game, game->select_idx);
			game_load_img_pal(game, 2, game->cell_anime_img);
			game_load_sprite_file(game, 0x80, 0, FILE_IMAGE_TITLE);

			gpu_pal_col_set(&game->gpu, 0, 0, &col_black);

			for (uint8 y = 0; y < 5; ++y)
			{
				for (uint8 x = 0; x < 40; ++x)
				{
					game_fg_draw_tile(game, x, 1 + y, 0, 0x80 + clear_art[MAD(y, 40, x)], 1, 1);
				}
			}

			for (uint8 y = 0; y < 8; ++y)
			{
				for (uint8 x = 0; x < 8; ++x)
				{
					game_fg_draw_tile(game, 2 + x, 9 + y, 2, 0x140 + MAD(x, 8, y), 1, 1);
				}
			}

			game_fadeset(game, 0);

			memset(game->clear.badge_anim, 0, sizeof(game->clear.badge_anim));

			game->clear.badge_hp = game->play.hp == game->play.hp_max;
			game->clear.badge_eth = game->play.ether == game->play.ether_num;
			game->play.ether += game->play.ether_bonus;

			game->save.clear_stage = MAX(game->save.clear_stage, game->stage_idx + 1);

			uint32 tbox_num = 0;

			for (uint32 i = 0; i < LOOT_MAX; ++i)
			{
				if (game->play.loot[i])
				{
					tbox_num++;
				}
			}

			game->clear.badge_tbox = tbox_num == game->play.tbox_num;
			game->clear.badge_perfect = game->clear.badge_hp && game->clear.badge_eth && game->clear.badge_tbox;
			game->clear.badge_no_kill = !game->play.kills;
			game->clear.badge_crystal = 0;

			for (uint32 i = 0; i < LOOT_MAX; ++i)
			{
				game->clear.tbox_ids[i] = UINT8_MAX;
				game->clear.tbox_sprite_idxs[i] = 0;
			}

			uint32 tbox_idx = 1;

			for (uint32 i = 0; i < LOOT_MAX; ++i)
			{
				if (game->play.loot[i])
				{
					uint8 item_id = TILE_ITEMID(game->play.loot[i]->tile);

					if (item_table[item_id].type == ITEM_TYPE_SMALL_TBOX)
					{
						game->clear.tbox_ids[tbox_idx++] = item_id;
					}
					else
					{
						game->clear.tbox_ids[0] = item_id;
					}
				}
			}

			for (uint32 i = 0; i < LOOT_MAX; ++i)
			{
				if (game->clear.tbox_ids[i] != UINT8_MAX)
				{
					game->clear.tbox_sprite_idxs[i] = 1;
				}
			}

			game->step = CLEAR_STEP_FADEIN;

			break;
		}
		case CLEAR_STEP_FADEIN:
		{
			if (game_fadein(game))
			{
				game->clear.index = 0;
				game->clear.counter = 30;
				game->step = CLEAR_STEP_SCORE;
			}

			break;
		}
		case CLEAR_STEP_SCORE:
		{
			if (game->clear.counter)
			{
				game->clear.counter--;

				game_fg_draw_text(game, 14, 12, 1, "SCORE:");
				game_fg_draw_number(game, 20, 12, 1, game->play.score, game->play.ez_mode);
			}
			else
			{
				if (game->clear.badge_hp)
				{
					game_se_play(game, 6);

					game->clear.badge_anim[0] = 1;
					game->clear.score_anim = BADGE_HP_MULT * game->play.score / 100;
					game->clear.index = 0;
					game->clear.counter = 30;
					game->step = CLEAR_STEP_HP_BADGE;
				}
				else
				{
					game->clear.counter = 30;
					game->step = CLEAR_STEP_ETHER;
				}
			}

			break;
		}
		case CLEAR_STEP_HP_BADGE:
		{
			if (game->clear.index < 99)
			{
				if (game->clear.counter)
				{
					game->clear.counter--;
				}
				else
				{
					if (game->clear.index == 2)
					{
						game->clear.index = 99;
					}

					game->clear.index++;
					game->clear.counter = 30;
				}

				if (game->clear.index == 1)
				{
					uint32 score = (game->clear.score_anim + game->clear.counter) / (game->clear.counter + 1);

					game->play.score += score;
					game->clear.score_anim -= score;

					game_se_play(game, 10);
				}

				game_fg_draw_number(game, 20, 12, 1, game->play.score, game->play.ez_mode);
				game_fg_draw_text(game, 27, 12, 1, "+");
				game_fg_draw_number(game, 28, 12, 1, game->clear.score_anim, 0);
				game_fg_draw_text(game, 29, 13, 1, "NO DAMAGE");
			}
			else
			{
				game_fg_clear(game, 27, 12, 13, 2);

				game->clear.counter = 30;
				game->step = CLEAR_STEP_ETHER;
			}

			break;
		}
		case CLEAR_STEP_ETHER:
		{
			if (game->clear.counter)
			{
				game->clear.counter--;

				game_fg_draw_text(game, 14, 15, 1, "ETHER:");
				game_fg_draw_number(game, 20, 15, 1, game->play.ether, 0);
			}
			else
			{
				if (game->clear.badge_eth)
				{
					game_se_play(game, 6);

					game->clear.badge_anim[1] = 1;
					game->clear.ether_anim = BADGE_ETHER_MULT * game->play.ether / 100;
					game->clear.index = 0;
					game->clear.counter = 30;
					game->step = CLEAR_STEP_ETHER_BADGE;
				}
				else
				{
					game->clear.ether_anim = 0;
					game->clear.index = 0;
					game->clear.counter = 30;
					game->step = CLEAR_STEP_TBOX1;
				}
			}

			break;
		}
		case CLEAR_STEP_ETHER_BADGE:
		{
			if (game->clear.index < 99)
			{
				if (game->clear.counter)
				{
					game->clear.counter--;
				}
				else
				{
					if (game->clear.index == 2)
					{
						game->clear.index = 99;
					}

					game->clear.index++;
					game->clear.counter = 30;
				}

				if (game->clear.index == 1)
				{
					uint32 ether = (game->clear.ether_anim + game->clear.counter) / (game->clear.counter + 1);

					game->play.ether += ether;
					game->clear.ether_anim -= ether;

					game_se_play(game, 10);
				}

				game_fg_draw_number(game, 20, 15, 1, game->play.ether, 0);
				game_fg_draw_text(game, 27, 15, 1, "+");
				game_fg_draw_number(game, 28, 15, 1, game->clear.ether_anim, 0);
				game_fg_draw_text(game, 29, 16, 1, "COMPLETE");
			}
			else
			{
				game_fg_clear(game, 27, 15, 13, 2);

				game->clear.ether_anim = 0;
				game->clear.index = 0;
				game->clear.counter = 30;
				game->step = CLEAR_STEP_TBOX1;
			}

			break;
		}
		case CLEAR_STEP_TBOX1:
		{
			if (game->clear.counter)
			{
				game->clear.counter--;

				if (!game->clear.counter)
				{
					game->clear.counter = 6;

					if (game->clear.index == 0 && game->clear.tbox_ids[0] == UINT8_MAX && game->clear.tbox_ids[1] != UINT8_MAX)
					{
						game->clear.index = 1;
					}

					if (game->clear.tbox_ids[game->clear.index] != UINT8_MAX)
					{
						game_se_play(game, 8);

						game->clear.tbox_sprite_idxs[game->clear.index] = 2;
						game->clear.index++;
					}
					else
					{
						if (game->clear.badge_tbox)
						{
							game->clear.badge_anim[2] = 1;

							game_fg_draw_text(game, 10, 20, 1, "TREASURE COMPLETE !!");

							if (game->clear.tbox_ids[0] != UINT8_MAX)
							{
								game->clear.badge_crystal = 1;
								game->clear.ether_anim = item_table[game->clear.tbox_ids[0]].score;

								if (game->clear.badge_crystal)
								{
									game_fg_draw_text(game, 27, 15, 1, "+");
									game_fg_draw_number(game, 28, 15, 1, game->clear.ether_anim, 0);
									game_fg_draw_text(game, 29, 16, 1, "TREASURE");
								}
							}
							else
							{
								game->clear.badge_crystal = 0;
							}
						}

						game->clear.index = 0;
						game->clear.counter = 30;
						game->step = CLEAR_STEP_TBOX2;
					}
				}
			}

			break;
		}
		case CLEAR_STEP_TBOX2:
		{
			if (game->clear.counter)
			{
				game->clear.counter--;

				if (!game->clear.counter)
				{
					game->clear.counter = 10;

					if (game->clear.index == 0 && game->clear.tbox_ids[0] == UINT8_MAX && game->clear.tbox_ids[1] != UINT8_MAX)
					{
						game->clear.index = 1;
					}

					if (game->clear.tbox_ids[game->clear.index] != UINT8_MAX)
					{
						game_se_play(game, 6);

						game->clear.ether_anim += item_table[game->clear.tbox_ids[game->clear.index]].score;
						game->clear.tbox_sprite_idxs[game->clear.index] = 3;
						game->clear.index++;
					}
					else
					{
						game->clear.index = 0;
						game->clear.counter = 30;
						game->step = CLEAR_STEP_TBOX3;
					}
				}
			}

			game_fg_draw_number(game, 20, 15, 1, game->play.ether, 0);
			game_fg_draw_text(game, 27, 15, 1, "+");
			game_fg_draw_number(game, 28, 15, 1, game->clear.ether_anim, 0);
			game_fg_draw_text(game, 29, 16, 1, "TREASURE");

			break;
		}
		case CLEAR_STEP_TBOX3:
		{
			if (game->clear.index < 99)
			{
				if (game->clear.counter)
				{
					game->clear.counter--;
				}
				else
				{
					if (game->clear.index == 2)
					{
						game->clear.index = 99;
					}

					game->clear.index++;
					game->clear.counter = 30;
				}

				if (game->clear.index == 1)
				{
					uint32 ether = (game->clear.ether_anim + game->clear.counter) / (game->clear.counter + 1);

					game->play.ether += ether;
					game->clear.ether_anim -= ether;

					game_se_play(game, 10);
				}

				game_fg_draw_number(game, 20, 15, 1, game->play.ether, 0);
				game_fg_draw_text(game, 27, 15, 1, "+");
				game_fg_draw_number(game, 28, 15, 1, game->clear.ether_anim, 0);
				game_fg_draw_text(game, 29, 16, 1, "TREASURE");
			}
			else
			{
				game_fg_clear(game, 27, 15, 13, 2);

				if (game->clear.badge_perfect)
				{
					game_se_play(game, 6);

					game->clear.badge_anim[3] = 1;
					game->clear.score_anim = BADGE_PERFECT_MULT * game->play.score / 100;
					game->clear.ether_anim = BADGE_PERFECT_ETHER;
					game->clear.index = 0;
					game->clear.counter = 30;
					game->step = CLEAR_STEP_PERFECT_BADGE;
				}
				else
				{
					game->clear.index = 0;
					game->clear.counter = 30;
					game->step = CLEAR_STEP_NO_KILL_CHECK;
				}
			}

			break;
		}
		case CLEAR_STEP_PERFECT_BADGE:
		{
			if (game->clear.index < 99)
			{
				if (game->clear.counter)
				{
					game->clear.counter--;
				}
				else
				{
					if (game->clear.index == 2)
					{
						game->clear.index = 99;
					}

					game->clear.index++;
					game->clear.counter = 30;
				}

				if (game->clear.index == 1)
				{
					uint32 score = (game->clear.score_anim + game->clear.counter) / (game->clear.counter + 1);
					uint32 ether = (game->clear.ether_anim + game->clear.counter) / (game->clear.counter + 1);

					game->play.score += score;
					game->play.ether += ether;
					game->clear.score_anim -= score;
					game->clear.ether_anim -= ether;

					game_se_play(game, 10);
				}

				game_fg_draw_number(game, 20, 12, 1, game->play.score, game->play.ez_mode);
				game_fg_draw_text(game, 27, 12, 1, "+");
				game_fg_draw_number(game, 28, 12, 1, game->clear.score_anim, 0);
				game_fg_draw_text(game, 29, 13, 1, "PERFECT");
				game_fg_draw_number(game, 20, 15, 1, game->play.ether, 0);
				game_fg_draw_text(game, 27, 15, 1, "+");
				game_fg_draw_number(game, 28, 15, 1, game->clear.ether_anim, 0);
				game_fg_draw_text(game, 29, 16, 1, "PERFECT");
			}
			else
			{
				game_fg_clear(game, 27, 12, 13, 5);

				game->clear.index = 0;
				game->clear.counter = 30;
				game->step = CLEAR_STEP_NO_KILL_CHECK;
			}

			break;
		}
		case CLEAR_STEP_NO_KILL_CHECK:
		{
			if (game->clear.badge_no_kill)
			{
				game_se_play(game, 6);

				game->clear.badge_anim[4] = 1;
				game->clear.score_anim = BADGE_NO_KILL_MULT * game->play.score / 100;
				game->clear.index = 0;
				game->clear.counter = 30;
				game->step = CLEAR_STEP_NO_KILL_BADGE;
			}
			else
			{
				game->clear.index = 0;
				game->clear.counter = 30;
				game->step = CLEAR_STEP_INPUT_WAIT;
			}

			break;
		}
		case CLEAR_STEP_NO_KILL_BADGE:
		{
			if (game->clear.index < 99)
			{
				if (game->clear.counter)
				{
					game->clear.counter--;
				}
				else
				{
					if (game->clear.index == 2)
					{
						game->clear.index = 99;
					}

					game->clear.index++;
					game->clear.counter = 30;
				}

				if (game->clear.index == 1)
				{
					uint32 score = (game->clear.score_anim + game->clear.counter) / (game->clear.counter + 1);

					game->play.score += score;
					game->clear.score_anim -= score;

					game_se_play(game, 10);
				}

				game_fg_draw_number(game, 20, 12, 1, game->play.score, game->play.ez_mode);
				game_fg_draw_text(game, 27, 12, 1, "+");
				game_fg_draw_number(game, 28, 12, 1, game->clear.score_anim, 0);
				game_fg_draw_text(game, 29, 13, 1, "NO KILL");
			}
			else
			{
				game_fg_clear(game, 27, 12, 13, 2);

				game->clear.counter = 30;
				game->step = CLEAR_STEP_INPUT_WAIT;
			}

			break;
		}
		case CLEAR_STEP_INPUT_WAIT:
		{
			if (game->input_down & INPUT_CONFIRM)
			{
				game_se_play(game, 11);

				game->step = CLEAR_STEP_FADEOUT;
			}

			break;
		}
		case CLEAR_STEP_FADEOUT:
		{
			if (game_fadeout(game))
			{
				game->phase = PHASE_NAME;
				game->step = NAME_STEP_INIT;
			}

			break;
		}
	}

	game_cell_anime(game);
	game_gpu_obj_clear(game);

	for (uint32 i = 0; i < 4; ++i)
	{
		uint32 sprite_idx = 4;

		if (game->clear.badge_anim[i])
		{
			sprite_idx = 5 + i;
		}

		game_draw_sprite(game, FILE_IMAGE_REWARDS, sprite_idx, 112 + (48 * i), 72, 0, GPU_SPRITE_PRIO_FG);
	}

	if (game->clear.badge_anim[4])
	{
		game_draw_sprite(game, FILE_IMAGE_REWARDS, 9, 304, 72, 0, GPU_SPRITE_PRIO_FG);
	}

	for (uint32 i = 0; i < LOOT_MAX; ++i)
	{
		uint8 sprite_idx = game->clear.tbox_sprite_idxs[i];

		if (sprite_idx)
		{
			if (i == 0 && sprite_idx < 3)
			{
				sprite_idx += 10;
			}

			static const uint8 tbox_pos_table[][2] =
			{
				{  28, 102 },
				{  58,  94 },
				{  84,  94 },
				{ 110,  94 },
				{ 136,  94 },
				{  58, 110 },
				{  84, 110 },
				{ 110, 110 },
				{ 136, 110 },
				{  17,  94 },
				{  17,  94 },
				{  17,  94 },
				{  17,  94 },
				{  17,  94 },
				{  17,  94 },
				{  17,  94 },
			};

			game_draw_sprite(game, FILE_IMAGE_REWARDS, sprite_idx - 1, tbox_pos_table[i][0] * 2, tbox_pos_table[i][1] * 2, 0, GPU_SPRITE_PRIO_FG);
		}
	}

	if (game->clear.badge_crystal)
	{
		game_draw_sprite(game, FILE_IMAGE_REWARDS, 2, 256, 160, 0, GPU_SPRITE_PRIO_FG);
	}

	task_draw(&game->task_sys, game);
}

void game_phase_over(game_t* game)
{
	switch (game->step)
	{
		case OVER_STEP_INIT:
		{
			gpu_reset(&game->gpu);
			gpu_fg_clear(&game->gpu);
			gpu_bg_clear(&game->gpu);

			game_bgm_play(game, 3);

			gpu_pal_set(&game->gpu, 1, pal2);

			game_cell_anime_init(game, game->select_idx);
			game_load_img_pal(game, 2, game->cell_anime_img);
			game_load_sprite_file(game, 0x80, 0, FILE_IMAGE_TITLE);

			gpu_pal_col_set(&game->gpu, 0, 0, &col_black);

			for (uint8 y = 0; y < 5; ++y)
			{
				for (uint8 x = 0; x < 40; ++x)
				{
					game_fg_draw_tile(game, x, 1 + y, 0, 0x80 + OverArt[MAD(y, 40, x)], 1, 1);
				}
			}

			for (uint8 y = 0; y < 8; ++y)
			{
				for (uint8 x = 0; x < 8; ++x)
				{
					game_fg_draw_tile(game, 16 + x, 7 + y, 2, 0x140 + MAD(x, 8, y), 1, 1);
				}
			}

			game_fg_draw_text(game, 14, 16, 1, "SCORE:");
			game_fg_draw_number(game, 20, 16, 1, game->play.score, game->play.ez_mode);

			game->play.ether += game->play.ether_bonus;

			game_fg_draw_text(game, 14, 18, 1, "ETHER:");
			game_fg_draw_number(game, 20, 18, 1, game->play.ether, 0);

			game_fadeset(game, 0);

			game->step = OVER_STEP_FADEIN;

			break;
		}
		case OVER_STEP_FADEIN:
		{
			if (game_fadein(game))
			{
				game->step = OVER_STEP_INPUT_WAIT;
			}

			break;
		}
		case OVER_STEP_INPUT_WAIT:
		{
			if (game->input_down & INPUT_CONFIRM)
			{
				game_se_play(game, 11);

				game->step = OVER_STEP_FADEOUT;
			}

			break;
		}
		case OVER_STEP_FADEOUT:
		{
			if (game_fadeout(game))
			{
				game->phase = PHASE_NAME;
				game->step = NAME_STEP_INIT;
			}

			break;
		}
	}

	game_cell_anime(game);
	game_gpu_obj_clear(game);

	task_draw(&game->task_sys, game);
}

void game_phase_name(game_t* game)
{
	switch (game->step)
	{
		case NAME_STEP_INIT:
		{
			gpu_reset(&game->gpu);
			gpu_fg_clear(&game->gpu);
			gpu_bg_clear(&game->gpu);

			game->name.high_score = game->play.score >= (game->save.leaderboard[game->stage_idx].score & ~EZ_MODE_FLAG);

			if (game->name.high_score)
			{
				game->save.leaderboard[game->stage_idx].score = game->play.score;

				if (game->play.ez_mode)
				{
					game->save.leaderboard[game->stage_idx].score |= EZ_MODE_FLAG;
				}

				strcpy(game->save.leaderboard[game->stage_idx].name, "   ");

				game_fg_draw_text(game, 10, 4, 0, "ENTER YOUR INITIALS!");
				game_fg_draw_text(game, 12, 7, 0, "SCORE       NAME");
				game_fg_draw_number(game, 11, 9, 0, game->play.score, game->play.ez_mode);
			}
			else
			{
				game_fg_draw_text(game, 15, 4, 0, "YOUR SCORE");
				game_fg_draw_number(game, 16, 6, 0, game->play.score, game->play.ez_mode);
				game_fg_draw_text(game, 15, 12, 0, "TOP PLAYERS");
			}

			game_fg_draw_text(game, 17, 15, 0, "SCORE      NAME");

			for (uint8 i = 0; i < ARRAYNUM(game->save.leaderboard); ++i)
			{
				uint8 Y = 17 + (2 * i);
				uint8 pal_idx = i == game->stage_idx ? 2 : 0;

				game_fg_draw_text(game, 7, Y, pal_idx, "STAGE");
				game_fg_draw_tile(game, 13, Y, pal_idx, '1' + i, 1, 1);
				game_fg_draw_number(game, 16, Y, pal_idx, game->save.leaderboard[i].score, 0);
				game_fg_draw_text(game, 29, Y, pal_idx, game->save.leaderboard[i].name);
			}

			gpu_pal_col_set(&game->gpu, 2, 0, &col_black);
			gpu_pal_col_set(&game->gpu, 2, 1, &col_black);
			gpu_pal_col_set(&game->gpu, 2, 2, &col_red);
			
			game_fadeset(game, 0);

			game->step = NAME_STEP_FADEIN;

			break;
		}
		case NAME_STEP_FADEIN:
		{
			if (game_fadein(game))
			{
				if (game->name.high_score)
				{
					game->name.counter = 0;
					game->name.char_idx = 0;
					game->name.char_select_idx = 0;
					strcpy(game->name.buffer, "A  ");

					game->step = NAME_STEP_INPUT;
				}
				else
				{
					game->step = NAME_STEP_INPUT_WAIT;
				}
			}

			break;
		}
		case NAME_STEP_INPUT:
		{
			static const char char_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!?&-. ";
			int8 change = 0;

			if (game->input_down & INPUT_DOWN)
			{
				change = -1;
			}
			else if (game->input_down & INPUT_UP)
			{
				change = 1;
			}

			if (change)
			{
				game->name.counter = 0;

				game_se_play(game, 10);

				if (game->name.char_select_idx == 0 && change < 0)
				{
					game->name.char_select_idx = ARRAYNUM(char_table) - 2;
				}
				else if (game->name.char_select_idx == ARRAYNUM(char_table) - 2 && change > 0)
				{
					game->name.char_select_idx = 0;
				}
				else
				{
					game->name.char_select_idx += change;
				}
			}

			if (game->input_down & INPUT_CONFIRM)
			{
				game->name.buffer[game->name.char_idx] = char_table[game->name.char_select_idx];
				game->name.counter = 0;

				game_se_play(game, 11);

				if (game->name.char_idx == 2)
				{
					strncpy(game->save.leaderboard[game->stage_idx].name, game->name.buffer, 4);

					file_write("save.bin", &game->save, sizeof(save_t));

					game->step = NAME_STEP_WAIT;
				}
				else
				{
					game->name.char_idx++;
				}
			}
			else if (game->input_down & INPUT_CANCEL)
			{
				if (game->name.char_idx > 0)
				{
					game->name.buffer[game->name.char_idx] = ' ';
					game->name.counter = 0;
					game->name.char_idx--;

					for (uint32 i = 0; i < ARRAYNUM(char_table); ++i)
					{
						if (char_table[i] == game->name.buffer[game->name.char_idx])
						{
							game->name.char_select_idx = i;

							break;
						}
					}
				}
			}

			game->name.counter++;

			if ((game->name.counter / 16) & 0x1)
			{
				game->name.buffer[game->name.char_idx] = ' ';
			}
			else
			{
				game->name.buffer[game->name.char_idx] = char_table[game->name.char_select_idx];
			}

			game_fg_draw_text(game, 25, 9, 2, game->name.buffer);

			break;
		}
		case NAME_STEP_INPUT_WAIT:
		{
			if (game->input_down & INPUT_CONFIRM)
			{
				game_se_play(game, 11);

				game->step = NAME_STEP_FADEOUT;
			}

			break;
		}
		case NAME_STEP_WAIT:
		{
			game->name.counter++;

			if (game->name.counter > 32)
			{
				game->step = NAME_STEP_FADEOUT;
			}

			break;
		}
		case NAME_STEP_FADEOUT:
		{
			if (game_fadeout(game))
			{
				game->phase = PHASE_TITLE;
				game->step = TITLE_STEP_INIT;

				game_bgm_play(game, 0);
			}

			break;
		}
	}

	game_gpu_obj_clear(game);

	task_draw(&game->task_sys, game);
}

void game_move(game_t* game)
{
	if (!(game->frame_num & 0xF))
	{
		effect_task_t* bubble = game_air_bubble(game);

		bubble->obj.x = game_rand_max(game, UINT16_MAX);
		bubble->obj.y = DRAW2GAME(GPU_SCR_H);
	}

	task_move(&game->task_sys, game);
}

void game_move_player(game_t* game, player_task_t* player)
{
	player->counter++;

	if (player->invuln_counter)
	{
		player->invuln_counter--;
	}

	uint8 stage_hit_flag = 0;
	object_t* hit_obj = game_coli_check(game, &stage_hit_flag);

	if (player->dmg_counter)
	{
		player->dmg_counter--;

		if (player->obj.hit_by_flag & HIT_FLAG_ENEMY)
		{
			enemy_task_t* hit_enemy = hit_obj;

			if (hit_enemy)
			{
				if (hit_enemy->hit_type == ENEMY_HIT_TYPE_TOUCH)
				{
					game_atk_dmg(game, hit_enemy, player->move_dir);
				}
			}
		}
	}
	else
	{
		int32 hp_dmg = 0;

		if (player->obj.hit_by_flag & (HIT_FLAG_ENEMY | HIT_FLAG_STAGE))
		{
			enemy_task_t* hit_enemy = hit_obj;

			if (hit_enemy)
			{
				if (hit_enemy->hit_type == ENEMY_HIT_TYPE_TOUCH)
				{
					game_atk_dmg(game, hit_enemy, player->move_dir);
				}
				else
				{
					hp_dmg = hit_enemy->atk;
				}
			}
			else
			{
				if (!game->play.ez_mode)
				{
					hp_dmg = game->play.hp_max / 4;
				}
			}
		}

		if (hp_dmg && !player->invuln_counter && !game->title.cheat_god_mode)
		{
			player->dmg_counter = 32;

			game_se_play(game, 1);
			game_add_hp(game, -hp_dmg);

			if (game->play.weapon)
			{
				game->play.weapon->state = ITEM_STATE_LOST;
				game->play.weapon = NULL;
			}
			else
			{
				uint8 big_tbox_idx = UINT8_MAX;
				uint8 lose_item_idx = UINT8_MAX;

				for (uint8 i = 0; i < LOOT_MAX; ++i)
				{
					if (game->play.loot[i])
					{
						uint8 tbox_id = TILE_ITEMID(game->play.loot[i]->tile);

						if (item_table[tbox_id].type == ITEM_TYPE_BIG_TBOX)
						{
							big_tbox_idx = i;
						}
						else
						{
							lose_item_idx = i;
						}
					}
				}

				if (lose_item_idx == UINT8_MAX)
				{
					lose_item_idx = big_tbox_idx;
				}

				if (lose_item_idx != UINT8_MAX)
				{
					game->play.loot[lose_item_idx]->state = ITEM_STATE_LOST;
					game->play.loot[lose_item_idx] = NULL;
				}
			}
		}
	}

	player->obj.hit_by_flag = HIT_FLAG_NONE;

	if (player->move_type == PLAYER_MOVE_TYPE_DIVE && !player->atk_cancel)
	{
		uint8 cancel = 0;
		enemy_task_t* enemy = game_atk_check(game);

		if (enemy)
		{
			if (enemy->hit_type == ENEMY_HIT_TYPE_ALWAYS)
			{
			}
			else if (enemy->hit_type == ENEMY_HIT_TYPE_TOP && (player->move_dir == PLAYER_MOVE_DIR_S || player->move_dir == PLAYER_MOVE_DIR_SW || player->move_dir == PLAYER_MOVE_DIR_SE))
			{
			}
			else if (enemy->hit_type == ENEMY_HIT_TYPE_BOTTOM && (player->move_dir == PLAYER_MOVE_DIR_N || player->move_dir == PLAYER_MOVE_DIR_NW || player->move_dir == PLAYER_MOVE_DIR_NE))
			{
			}
			else
			{
				cancel = 1;
			}

			if (!cancel)
			{
				game_atk_dmg(game, enemy, player->move_dir);
			}
		}
		else if (player->atk_w)
		{
			int16 atk_x = GAME2DRAW(player->obj.x) + player->atk_x;
			int16 atk_y = GAME2DRAW(player->obj.y) + SCROLL2DRAW(game->play.scroll_y) + player->atk_y;

			if (ISIN(atk_x, 0, PLAY_AREA_W - 1) && ISIN(atk_y, 0, PLAY_AREA_H + SCROLL2DRAW(game->play.scroll_y_max) - 1))
			{
				uint8 tile = game_get_stage_tile(game, atk_x / 16, atk_y / 16);

				if (TILE_IS_WALL(tile))
				{
					cancel = 1;
				}
			}
			else
			{
				cancel = 1;
			}
		}

		if (cancel)
		{
			game_se_play(game, 17);

			player->atk_counter = MAX(player->atk_counter, 15 - player->atk_counter);
			player->atk_cancel = 1;
		}
	}

	if (player->dmg_counter > 24)
	{
		effect_task_t* bubble = game_air_bubble(game);

		bubble->life = game_rand_minmax(game, 16, 48);
		bubble->spd_x = game_rand_minmax(game, -256, 256);
		bubble->obj.x = player->obj.x + DRAW2GAME(game_rand_minmax(game, -2, 2));
		bubble->obj.y = player->obj.y + DRAW2GAME(game_rand_minmax(game, -2, 2));
	}

	int32 new_x = player->obj.x;
	int32 new_y = player->obj.y;

	if (player->move_type != PLAYER_MOVE_TYPE_FLOAT)
	{
		static const uint8 stage_depen_table[] =
		{
			0x0, 0xA, 0x9, 0x8, 0x6, 0x2, 0x0, 0xA, 0x5, 0x0, 0x1, 0x9, 0x4, 0x6, 0x5, 0x0,
		};

		uint8 stage_depen_flag = stage_depen_table[stage_hit_flag];

		if (stage_depen_flag & 0x1)
		{
			new_x -= DRAW2GAME(8);
		}

		if (stage_depen_flag & 0x2)
		{
			new_x += DRAW2GAME(8);
		}

		if (stage_depen_flag & 0x4)
		{
			new_y -= DRAW2GAME(8);
		}

		if (stage_depen_flag & 0x8)
		{
			new_y += DRAW2GAME(8);
		}
	}

	if (player->obj.hit_flag)
	{
		uint32 input_dir = 0;

		if (game->input_hold & INPUT_RIGHT)
		{
			input_dir |= 0x8;
		}

		if (game->input_hold & INPUT_LEFT)
		{
			input_dir |= 0x4;
		}

		if (game->input_hold & INPUT_DOWN)
		{
			input_dir |= 0x2;
		}

		if (game->input_hold & INPUT_UP)
		{
			input_dir |= 0x1;
		}

		static const int8 move_spd_table[][2] =
		{
			{   0,   0 },
			{   0, -64 },
			{   0,  64 },
			{   0,   0 },
			{ -64,   0 },
			{ -45, -45 },
			{ -45,  45 },
			{   0,   0 },
			{  64,   0 },
			{  45, -45 },
			{  45,  45 },
			{   0,   0 },
			{   0,   0 },
			{   0,   0 },
			{   0,   0 },
			{   0,   0 },
		};

		new_x += (player->spd_lr * move_spd_table[input_dir][0]) / 64;
		new_y += (((input_dir & 0x1) ? player->spd_u : player->spd_d) * move_spd_table[input_dir][1]) / 64;

		if (player->move_type == PLAYER_MOVE_TYPE_DIVE && !player->atk_counter)
		{
			if (input_dir)
			{
				if (game->play.weapon)
				{
					static const uint8 MoveDirTable[] =
					{
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_N,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_W,
						PLAYER_MOVE_DIR_NW,
						PLAYER_MOVE_DIR_SW,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_E,
						PLAYER_MOVE_DIR_NE,
						PLAYER_MOVE_DIR_SE,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
					};

					player->move_dir = MoveDirTable[input_dir];
				}
				else
				{
					static const uint8 move_dir_table[] =
					{
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_N,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_W,
						PLAYER_MOVE_DIR_N,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_E,
						PLAYER_MOVE_DIR_N,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
						PLAYER_MOVE_DIR_S,
					};

					player->move_dir = move_dir_table[input_dir];
				}

				static const uint8 move_sprite_table[][2] =
				{
					{ 0, 0 },
					{ 2, 2 },
					{ 2, 0 },
					{ 6, 0 },
					{ 4, 2 },
					{ 4, 0 },
					{ 6, 1 },
					{ 4, 3 },
					{ 4, 1 },
				};

				player->sprite_idx = move_sprite_table[player->move_dir][0];
				player->obj.flip = move_sprite_table[player->move_dir][1];
			}

			if (new_x < DRAW2GAME(512))
			{
				if (game->input_down & INPUT_CONFIRM)
				{
					game_se_play(game, 16);

					player->atk_counter = 1;
					player->atk_cancel = 0;
				}
			}
		}

		if (new_x < DRAW2GAME(512))
		{
			new_x = CLAMP(new_x, DRAW2GAME(8), DRAW2GAME(PLAY_AREA_W - 8));
		}
		else
		{
			new_x = CLAMP(new_x, DRAW2GAME(BONUS_AREA_OFFS + 8), DRAW2GAME(BONUS_AREA_OFFS + PLAY_AREA_W - 8));
		}

		new_y = CLAMP(new_y, DRAW2GAME(8), DRAW2GAME(PLAY_AREA_H - 8));
	}

	if (player->move_type == PLAYER_MOVE_TYPE_DIVE)
	{
		player->scroll = 0;

		if (player->scroll_threshold)
		{
			int32 scroll_y = DRAW2GAME(player->scroll_threshold);

			if (new_y > scroll_y)
			{
				new_y = scroll_y;

				player->scroll = 1;
			}
		}

		if (player->atk_counter)
		{
			if (player->atk_counter < 15)
			{
				player->atk_counter++;
			}
			else
			{
				player->atk_counter = 0;
			}
		}
	}

	player->obj.x = new_x;
	player->obj.y = new_y;
}

void game_move_enemy(game_t* game, enemy_task_t* enemy)
{
	uint8 stage_hit_check = 1;

	if (enemy->dmg_time)
	{
		enemy->dmg_time--;

		if (enemy->dmg_time)
		{
			if (enemy->hp > 1)
			{
				int8 back_spd_table[][2] =
				{
					{   0,   0, },
					{   0, -16, },
					{   0,  16, },
					{ -16,   0, },
					{ -11, -11, },
					{ -11,  11, },
					{  16,   0, },
					{  11, -11, },
					{  11,  11, },
				};

				enemy->abs_x += (enemy->back_spd * back_spd_table[enemy->dmg_dir][0]) / 16;
				enemy->abs_y += (enemy->back_spd * back_spd_table[enemy->dmg_dir][1]) / 16;
			}
		}
		else
		{
			if (enemy->hp > 1)
			{
				enemy->hp--;
			}
			else
			{
				game_obj_remove(game, enemy);

				return;
			}
		}
	}

	if (!enemy->dmg_time)
	{
		switch (enemy->move_state)
		{
			case ENEMY_MOVE_STATE_NONE:
			{
				enemy->spd_x = 0;
				enemy->spd_y = 0;

				break;
			}
			case ENEMY_MOVE_STATE_RIGHT:
			{
				enemy->spd_x = enemy->spd;
				enemy->abs_x += enemy->spd_x;
				enemy->obj.flip = GPU_SPRITE_FLIP_X;

				if ((enemy->abs_x - enemy->start_x) >= DRAW2GAME(enemy->range))
				{
					enemy->move_state = ENEMY_MOVE_STATE_LEFT;
				}

				break;
			}
			case ENEMY_MOVE_STATE_LEFT:
			{
				enemy->spd_x = -enemy->spd;
				enemy->abs_x += enemy->spd_x;
				enemy->obj.flip = 0;

				if ((enemy->start_x - enemy->abs_x) >= DRAW2GAME(enemy->range))
				{
					enemy->move_state = ENEMY_MOVE_STATE_RIGHT;
				}

				break;
			}
			case ENEMY_MOVE_STATE_DOWN:
			{
				enemy->spd_y = enemy->spd;
				enemy->abs_y += enemy->spd_y;

				if ((enemy->abs_y - enemy->start_y) >= DRAW2GAME(enemy->range))
				{
					enemy->move_state = ENEMY_MOVE_STATE_UP;
				}

				break;
			}
			case ENEMY_MOVE_STATE_UP:
			{
				enemy->spd_y = -enemy->spd;
				enemy->abs_y += enemy->spd_y;

				if ((enemy->start_y - enemy->abs_y) >= DRAW2GAME(enemy->range))
				{
					enemy->move_state = ENEMY_MOVE_STATE_DOWN;
				}

				break;
			}
			case ENEMY_MOVE_STATE_CHASE:
			{
				game_get_player_vxvy(game, enemy->abs_x, enemy->abs_y, enemy->act_spd, &enemy->spd_x, &enemy->spd_y);

				enemy->abs_x += enemy->spd_x;
				enemy->abs_y += enemy->spd_y;
				enemy->obj.flip = (enemy->spd_x >= 0) ? GPU_SPRITE_FLIP_X : 0;

				break;
			}
			case ENEMY_MOVE_STATE_DASH:
			{
				enemy->abs_x += enemy->spd_x;
				enemy->abs_y += enemy->spd_y;
				enemy->obj.flip = (enemy->spd_x >= 0) ? GPU_SPRITE_FLIP_X : 0;

				if (!ISIN(enemy->obj.x, DRAW2GAME(-64), DRAW2GAME(GPU_SCR_W + 64)) || !ISIN(enemy->obj.y, DRAW2GAME(-64), DRAW2GAME(GPU_SCR_H + 64)))
				{
					game_obj_remove(game, enemy);

					return;
				}

				break;
			}
			case ENEMY_MOVE_STATE_RETURN:
			{
				if (enemy->move_time)
				{
					enemy->move_time--;
					enemy->spd_x = 0;
					enemy->spd_y = 0;
				}
				else
				{
					if (game_get_vxvy(game, enemy->abs_x - enemy->start_x, enemy->abs_y - enemy->start_y, enemy->spd, &enemy->spd_x, &enemy->spd_y))
					{
						enemy->abs_x += enemy->spd_x;
						enemy->abs_y += enemy->spd_y;
						enemy->obj.flip = (enemy->spd_x >= 0) ? GPU_SPRITE_FLIP_X : 0;
					}
					else
					{
						enemy->move_state = ENEMY_MOVE_STATE_NONE;
						enemy->abs_x = enemy->start_x;
						enemy->abs_y = enemy->start_y;
					}
				}

				break;
			}
			case ENEMY_MOVE_STATE_BONUS_SPAWN:
			{
				if (game->bonus.end)
				{
					enemy->spd_x = 0;
					enemy->spd_y = 0;

					if (game->step == GAME_STEP_BONUS_EXIT)
					{
						enemy->obj.y = DRAW2GAME(384);
					}
				}
				else
				{
					enemy->spd_y += enemy->spd;
					enemy->abs_x += enemy->spd_x;
					enemy->abs_y += enemy->spd_y;
					enemy->obj.flip = (enemy->spd_x >= 0) ? GPU_SPRITE_FLIP_X : 0;
				}

				if (enemy->obj.y > DRAW2GAME(320))
				{
					game_obj_remove(game, enemy);

					return;
				}

				stage_hit_check = 0;

				break;
			}
		}

		switch (enemy->move_type)
		{
			case ENEMY_MOVE_TYPE_HOR:
			{
				if (enemy->move_state == ENEMY_MOVE_STATE_NONE)
				{
					enemy->move_state = ENEMY_MOVE_STATE_RIGHT;
				}
				else if (enemy->move_state == ENEMY_MOVE_STATE_RIGHT)
				{
					if ((enemy->abs_x - enemy->start_x) >= DRAW2GAME(enemy->range))
					{
						enemy->move_state = ENEMY_MOVE_STATE_LEFT;
					}
				}
				else if (enemy->move_state == ENEMY_MOVE_STATE_LEFT)
				{
					if ((enemy->start_x - enemy->abs_x) >= DRAW2GAME(enemy->range))
					{
						enemy->move_state = ENEMY_MOVE_STATE_RIGHT;
					}
				}

				break;
			}
			case ENEMY_MOVE_TYPE_VERT:
			{
				if (enemy->move_state == ENEMY_MOVE_STATE_NONE)
				{
					enemy->move_state = ENEMY_MOVE_STATE_DOWN;
				}
				else if (enemy->move_state == ENEMY_MOVE_STATE_DOWN)
				{
					if ((enemy->abs_y - enemy->start_y) >= DRAW2GAME(enemy->range))
					{
						enemy->move_state = ENEMY_MOVE_STATE_UP;
					}
				}
				else if (enemy->move_state == ENEMY_MOVE_STATE_UP)
				{
					if ((enemy->start_y - enemy->abs_y) >= DRAW2GAME(enemy->range))
					{
						enemy->move_state = ENEMY_MOVE_STATE_DOWN;
					}
				}

				break;
			}
			case ENEMY_MOVE_TYPE_CHASE:
			case ENEMY_MOVE_TYPE_ESCAPE:
			{
				if (enemy->move_state == ENEMY_MOVE_STATE_NONE)
				{
					enemy->start_x = enemy->abs_x;
					enemy->start_y = enemy->abs_y;
				}

				if (game_get_player_dist(game, enemy->abs_x, enemy->abs_y) < enemy->search)
				{
					if (enemy->move_type == ENEMY_MOVE_TYPE_CHASE)
					{
						enemy->move_state = ENEMY_MOVE_STATE_CHASE;
					}
					else
					{
						if (enemy->move_state != ENEMY_MOVE_STATE_DASH)
						{
							game_get_player_vxvy(game, enemy->abs_x, enemy->abs_y, -enemy->act_spd, &enemy->spd_x, &enemy->spd_y);
						}

						enemy->move_state = ENEMY_MOVE_STATE_DASH;
					}
				}
				else
				{
					if (enemy->move_state == ENEMY_MOVE_STATE_CHASE || enemy->move_state == ENEMY_MOVE_STATE_DASH)
					{
						enemy->move_state = ENEMY_MOVE_STATE_RETURN;
						enemy->move_time = 30;
					}
					else
					{
						if (enemy->move_state == ENEMY_MOVE_STATE_NONE)
						{
							enemy->move_state = ENEMY_MOVE_STATE_RIGHT;
						}
						else if (enemy->move_state == ENEMY_MOVE_STATE_RIGHT)
						{
							if ((enemy->abs_x - enemy->start_x) >= DRAW2GAME(enemy->range))
							{
								enemy->move_state = ENEMY_MOVE_STATE_LEFT;
							}
						}
						else if (enemy->move_state == ENEMY_MOVE_STATE_LEFT)
						{
							if ((enemy->start_x - enemy->abs_x) >= DRAW2GAME(enemy->range))
							{
								enemy->move_state = ENEMY_MOVE_STATE_RIGHT;
							}
						}
					}
				}

				break;
			}
			case ENEMY_MOVE_TYPE_AMBUSH_HOR:
			{
				int32 player_dx;
				int32 player_dy;

				game_get_player_dxdy(game, enemy->abs_x, enemy->abs_y, &player_dx, &player_dy);

				if (enemy->move_state == ENEMY_MOVE_STATE_NONE)
				{
					enemy->obj.flip = (player_dx < 0) ? GPU_SPRITE_FLIP_X : 0;
				}

				if (game_get_player_dist(game, enemy->start_x, enemy->start_y) < enemy->search)
				{
					if (enemy->move_state == ENEMY_MOVE_STATE_DASH)
					{
						if (enemy->spd_x < 0)
						{
							if (GAME2DRAW(player_dx) < 8)
							{
								enemy->move_state = ENEMY_MOVE_STATE_RETURN;
								enemy->move_time = 30;
							}
						}
						else
						{
							if (GAME2DRAW(player_dx) > -8)
							{
								enemy->move_state = ENEMY_MOVE_STATE_RETURN;
								enemy->move_time = 30;
							}
						}
					}
					else if (enemy->move_state == ENEMY_MOVE_STATE_NONE)
					{
						enemy->spd_x = -enemy->act_spd * SIGN(player_dx);
						enemy->move_state = ENEMY_MOVE_STATE_DASH;

						game_se_play(game, 14);
					}
				}
				else
				{
					if (enemy->move_state == ENEMY_MOVE_STATE_DASH)
					{
						enemy->move_state = ENEMY_MOVE_STATE_RETURN;
						enemy->move_time = 30;
					}
				}

				stage_hit_check = 0;

				break;
			}
			case ENEMY_MOVE_TYPE_AMBUSH_UP:
			case ENEMY_MOVE_TYPE_AMBUSH_DOWN:
			case ENEMY_MOVE_TYPE_AMBUSH_VERT:
			{
				if (game_get_player_dist(game, enemy->abs_x, enemy->abs_y) < enemy->search)
				{
					int32 player_dx;
					int32 player_dy;

					game_get_player_dxdy(game, enemy->abs_x, enemy->abs_y, &player_dx, &player_dy);

					if (enemy->move_state == ENEMY_MOVE_STATE_DASH)
					{
						if (enemy->spd_y < 0)
						{
							if (GAME2DRAW(player_dy) < 8)
							{
								enemy->move_state = ENEMY_MOVE_STATE_RETURN;
								enemy->move_time = 30;
							}
						}
						else
						{
							if (GAME2DRAW(player_dy) >= -8)
							{
								enemy->move_state = ENEMY_MOVE_STATE_RETURN;
								enemy->move_time = 30;
							}
						}
					}
					else if (enemy->move_state == ENEMY_MOVE_STATE_NONE)
					{
						if (abs(GAME2DRAW(player_dx)) < 8)
						{
							if (player_dy < 0)
							{
								if (enemy->move_type == ENEMY_MOVE_TYPE_AMBUSH_DOWN || enemy->move_type == ENEMY_MOVE_TYPE_AMBUSH_VERT)
								{
									enemy->spd_y = enemy->act_spd;
									enemy->move_state = ENEMY_MOVE_STATE_DASH;
								}
							}
							else
							{
								if (enemy->move_type == ENEMY_MOVE_TYPE_AMBUSH_UP || enemy->move_type == ENEMY_MOVE_TYPE_AMBUSH_VERT)
								{
									enemy->spd_y = -enemy->act_spd;
									enemy->move_state = ENEMY_MOVE_STATE_DASH;
								}
							}
						}
					}
				}
				else
				{
					if (enemy->move_state == ENEMY_MOVE_STATE_DASH)
					{
						enemy->move_state = ENEMY_MOVE_STATE_RETURN;
						enemy->move_time = 30;
					}
				}

				stage_hit_check = 0;

				break;
			}
			case ENEMY_MOVE_TYPE_TENTACLE:
			{
				uint8 seg_idx = enemy->tentacle.seg_idx;

				if (seg_idx > 0)
				{
					enemy_task_t* parent = enemy->tentacle.parent;

					if (parent)
					{
						enemy->abs_x = parent->abs_x + DRAW2GAME(parent->tentacle.seg[seg_idx - 1].x);
						enemy->abs_y = parent->abs_y + DRAW2GAME(parent->tentacle.seg[seg_idx - 1].y);
					}
				}
				else
				{
					if (!enemy->tentacle.active)
					{
						for (uint32 i = 0; i < 4; ++i)
						{
							static const int8 hit_table[][4] =
							{
								{ -6, -6, 12, 12 },
								{ -6, -6, 12, 12 },
								{ -4, -4, 8, 8 },
								{ -3, -3, 6, 6 },
							};

							enemy_task_t* child = game_obj_create(game);

							child->obj.task.move_cb = game_move_enemy;
							child->obj.task.draw_cb = game_draw_enemy;
							child->obj.x = enemy->obj.x;
							child->obj.y = enemy->obj.y;
							child->obj.flip = enemy->obj.flip;
							child->obj.hit_flag = enemy->obj.hit_flag;
							child->obj.hit_by_flag = enemy->obj.hit_by_flag;
							child->obj.hit_x = hit_table[i][0];
							child->obj.hit_y = hit_table[i][1];
							child->obj.hit_w = hit_table[i][2];
							child->obj.hit_h = hit_table[i][3];
							child->tile = enemy->tile;
							child->counter = enemy->counter;
							child->move_state = enemy->move_state;
							child->move_time = enemy->move_time;
							child->dmg_time = enemy->dmg_time;
							child->abs_x = enemy->abs_x;
							child->abs_y = enemy->abs_y;
							child->spd_x = enemy->spd_x;
							child->spd_y = enemy->spd_y;
							child->start_x = enemy->start_x;
							child->start_y = enemy->start_y;
							child->spd = enemy->spd;
							child->range = enemy->range;
							child->search = enemy->search;
							child->act_spd = enemy->act_spd;
							child->hp = enemy->hp;
							child->back_spd = enemy->back_spd;
							child->back_time = enemy->back_time;
							child->atk = enemy->atk;
							child->score = enemy->score;
							child->move_type = enemy->move_type;
							child->hit_type = enemy->hit_type;
							child->file_idx = enemy->file_idx;
							child->tentacle.seg_idx = i + 1;
							child->tentacle.parent = enemy;
						}

						enemy->tentacle.active = 1;
					}

					enemy->move_time++;

					for (uint32 i = 0; i < TENTACLE_SEG_MAX; ++i)
					{
						uint32 child_idx = TENTACLE_SEG_MAX - i - 1;
						int8 parent_x;
						int8 parent_y;

						if (child_idx != 0)
						{
							enemy->tentacle.seg[child_idx].rot = enemy->tentacle.seg[child_idx - 1].delay[TENTACLE_DELAY_MAX - 1];

							for (uint32 j = 0; j < TENTACLE_DELAY_MAX - 1; ++j)
							{
								enemy->tentacle.seg[child_idx - 1].delay[TENTACLE_DELAY_MAX - j - 1] = enemy->tentacle.seg[child_idx - 1].delay[TENTACLE_DELAY_MAX - j - 2];
							}

							enemy->tentacle.seg[child_idx - 1].delay[0] = enemy->tentacle.seg[child_idx - 1].rot;

							parent_x = enemy->tentacle.seg[child_idx - 1].x;
							parent_y = enemy->tentacle.seg[child_idx - 1].y;
						}
						else
						{
							int32 player_dx;
							int32 player_dy;

							game_get_player_dxdy(game, enemy->start_x, enemy->start_y, &player_dx, &player_dy);

							int32 rot = (int32)((atan2f(player_dy, player_dx) + sinf(enemy->move_time * 0.05f) * 0.7f) / (PI * 2.0f) * 256.0f);

							rot += 128;
							rot -= enemy->tentacle.seg[child_idx].rot;

							if (rot < -128)
							{
								rot += 256;
							}
							else if (rot > 127)
							{
								rot -= 256;
							}

							enemy->tentacle.seg[child_idx].rot += SIGNZ(rot);

							parent_x = 0;
							parent_y = 0;
						}

						float angle = (enemy->tentacle.seg[child_idx].rot * PI * 2.0f) / 256.0f;

						enemy->tentacle.seg[child_idx].x = parent_x + (int8)(cosf(angle) * 16.0f);
						enemy->tentacle.seg[child_idx].y = parent_y + (int8)(sinf(angle) * 16.0f);
					}
				}

				stage_hit_check = 0;

				break;
			}
			case ENEMY_MOVE_TYPE_BONUS:
			{
				if (enemy->obj.hit_by_flag && (game->input_hold & (INPUT_LEFT | INPUT_RIGHT)))
				{
					enemy->move_time++;

					if (enemy->move_time > game->play.bonus_entry_time)
					{
						game->bonus.saved_step = game->step;
						game->step = GAME_STEP_BONUS_START;

						game_obj_remove(game, enemy);

						return;
					}
				}
				else
				{
					enemy->move_time = 0;

					if (enemy->abs_x > DRAW2GAME(PLAY_AREA_W / 2))
					{
						enemy->abs_x = DRAW2GAME(PLAY_AREA_W - 8);
					}
					else
					{
						enemy->abs_x = DRAW2GAME(8);
					}
				}

				stage_hit_check = 0;

				break;
			}
			case ENEMY_MOVE_TYPE_BONUS_SPAWN:
			{
				if (enemy->obj.hit_by_flag)
				{
					if (enemy->score)
					{
						game_add_bonus_ether(game, enemy->score);
						game_se_play(game, 3);

						game_obj_remove(game, enemy);

						return;
					}
					else
					{
						game->bonus.end = 1;

						game_se_play(game, 1);
					}
				}

				stage_hit_check = 0;

				break;
			}
		}

		enemy->counter++;
		enemy->obj.hit_by_flag = HIT_FLAG_NONE;
	}

	if (stage_hit_check)
	{
		int32 hit_min_x = GAME2DRAW(enemy->abs_x) + enemy->obj.hit_x;
		int32 hit_min_y = GAME2DRAW(enemy->abs_y) + enemy->obj.hit_y;
		int32 hit_max_x = hit_min_x + enemy->obj.hit_w - 1;
		int32 hit_max_y = hit_min_y + enemy->obj.hit_h - 1;

		hit_min_x = CLAMP(hit_min_x, 0, PLAY_AREA_W - 1);
		hit_min_y = CLAMP(hit_min_y, 0, PLAY_AREA_H + SCROLL2DRAW(game->play.scroll_y_max) - 1);
		hit_max_x = CLAMP(hit_max_x, 0, PLAY_AREA_W - 1);
		hit_max_y = CLAMP(hit_max_y, 0, PLAY_AREA_H + SCROLL2DRAW(game->play.scroll_y_max) - 1);

		int32 tile_min_x = hit_min_x / 16;
		int32 tile_min_y = hit_min_y / 16;
		int32 tile_max_x = hit_max_x / 16;
		int32 tile_max_y = hit_max_y / 16;
		int32 sub_min_x = -hit_min_x & 0xF;
		int32 sub_min_y = -hit_min_y & 0xF;
		int32 sub_max_x = -hit_max_x & 0xF;
		int32 sub_max_y = -hit_max_y & 0xF;
		int32 depen_x = 0;
		int32 depen_y = 0;

		if (TILE_IS_WALL(game_get_stage_tile(game, tile_min_x, tile_min_y)))
		{
			if (sub_min_x >= sub_min_y)
			{
				depen_y = sub_min_y;
			}
			else
			{
				depen_x = sub_min_x;
			}
		}

		if (TILE_IS_WALL(game_get_stage_tile(game, tile_max_x, tile_min_y)))
		{
			if (sub_max_x >= sub_min_y)
			{
				depen_y = sub_min_y;
			}
			else
			{
				depen_x = -sub_max_x;
			}
		}

		if (TILE_IS_WALL(game_get_stage_tile(game, tile_min_x, tile_max_y)))
		{
			if (sub_min_x >= sub_max_y)
			{
				depen_y = -sub_max_y;
			}
			else
			{
				depen_x = sub_min_x;
			}
		}

		if (TILE_IS_WALL(game_get_stage_tile(game, tile_max_x, tile_max_y)))
		{
			if (sub_max_x >= sub_max_y)
			{
				depen_y = -sub_max_y;
			}
			else
			{
				depen_x = -sub_max_x;
			}
		}

		enemy->abs_x += DRAW2GAME(depen_x);
		enemy->abs_y += DRAW2GAME(depen_y);
	}

	enemy->obj.x = enemy->abs_x;
	enemy->obj.y = enemy->abs_y - SCROLL2GAME(game->play.scroll_y);
}

void game_move_item(game_t* game, item_task_t* item)
{
	switch (item->state)
	{
		case ITEM_STATE_SPAWN:
		{
			item->counter++;
			item->obj.x = item->abs_x;
			item->obj.y = item->abs_y - SCROLL2GAME(game->play.scroll_y);

			if (!item->obj.hit_by_flag)
			{
				break;
			}

			item->loot_x = item->obj.x;
			item->loot_y = item->obj.y;
			item->state = ITEM_STATE_LOOTING;

			if (TILE_IS_ITEM(item->tile))
			{
				uint8 loot_slot_idx = game_loot_item(game, TILE_ITEMID(item->tile), item);

				if (loot_slot_idx == UINT8_MAX)
				{
					game_obj_remove(game, item);

					return;
				}

				item->anim_time = 30;

				switch (item_table[TILE_ITEMID(item->tile)].type)
				{
					case ITEM_TYPE_SMALL_TBOX:
					{
						static const uint16 loot_anim_table[][2] =
						{
							{ 276, 144 },
							{ 300, 144 },
							{ 276, 168 },
							{ 300, 168 },
							{ 276, 192 },
							{ 300, 192 },
							{ 276, 216 },
							{ 300, 216 },
							{ 276, 144 },
							{ 276, 144 },
							{ 276, 144 },
							{ 276, 144 },
							{ 276, 144 },
							{ 276, 144 },
							{ 276, 144 },
							{ 276, 144 },
						};

						item->loot_anim_x = DRAW2GAME(loot_anim_table[loot_slot_idx][0]);
						item->loot_anim_y = DRAW2GAME(loot_anim_table[loot_slot_idx][1]);

						break;
					}
					case ITEM_TYPE_BIG_TBOX:
					{
						item->loot_anim_x = DRAW2GAME(268);
						item->loot_anim_y = DRAW2GAME(92);

						break;
					}
					case ITEM_TYPE_WEAPON:
					{
						item->loot_anim_x = DRAW2GAME(308);
						item->loot_anim_y = DRAW2GAME(100);

						break;
					}
				}

				game_se_play(game, 4);
			}
			else if (TILE_IS_ETHER(item->tile))
			{
				item->anim_time = 30;
				item->loot_anim_x = DRAW2GAME(304);
				item->loot_anim_y = DRAW2GAME(40);

				game_se_play(game, 3);
			}

			item->counter = 0;
			item->loot_anim_x -= item->loot_x;
			item->loot_anim_y -= item->loot_y;

			break;
		}
		case ITEM_STATE_LOOTING:
		{
			if (item->counter < item->anim_time)
			{
				item->counter++;
				item->obj.x = item->loot_x + ((item->loot_anim_x * item->counter) / item->anim_time);
				item->obj.y = item->loot_y + ((item->loot_anim_y * item->counter) / item->anim_time);
				item->obj.y += (int32)(DRAW2GAME(sinf((item->counter * PI) / item->anim_time) * 64.0f));
			}
			else
			{
				game_add_score(game, item->score);

				if (TILE_IS_ITEM(item->tile))
				{
					item->obj.x = item->loot_x + item->loot_anim_x;
					item->obj.y = item->loot_y + item->loot_anim_y;
					item->state = ITEM_STATE_LOOT;
				}
				else if (TILE_IS_ETHER(item->tile))
				{
					game_add_ether(game, 1);

					game_obj_remove(game, item);
				}
			}

			break;
		}
		case ITEM_STATE_LOST:
		{
			item->obj.y += DRAW2GAME(4);

			if (item->obj.y > DRAW2GAME(GPU_SCR_H))
			{
				game_obj_remove(game, item);
			}

			break;
		}
		case ITEM_STATE_DROP:
		{
			if (item->counter)
			{
				item->counter--;
				item->obj.x = item->abs_x;
				item->obj.y = item->abs_y - SCROLL2GAME(game->play.scroll_y);

				if (item->obj.hit_by_flag)
				{
					game_se_play(game, 4);

					if (item->tile)
					{
						game_add_hp(game, game->play.o2_val);
					}
					else
					{
						game->play.player->invuln_counter = 360;
					}

					game_obj_remove(game, item);
				}
			}
			else
			{
				game_obj_remove(game, item);
			}

			break;
		}
	}
}

void game_move_effect(game_t* game, effect_task_t* effect)
{
	if (effect->pause)
	{
		return;
	}

	effect->counter++;
	effect->obj.x += effect->spd_x + (int32)(DRAW2GAME(sinf(effect->counter * 0.2f)));
	effect->obj.y += effect->spd_y - SCROLL2GAME(game->play.scroll_spd);

	if (effect->obj.y < 0)
	{
		game_obj_remove(game, effect);
	}
	else if (effect->life)
	{
		effect->life--;

		if (!effect->life)
		{
			game_obj_remove(game, effect);
		}
	}
}

void game_draw_player(game_t* game, player_task_t* player)
{
	if (player->dmg_counter)
	{
		if (player->dmg_counter & 0x1)
		{
			return;
		}
	}
	else if (player->invuln_counter)
	{
		if (player->invuln_counter > 90)
		{
			if (player->invuln_counter & 0x1)
			{
				return;
			}
		}
		else
		{
			if ((player->invuln_counter & 0x3) == 3)
			{
				return;
			}
		}
	}

	uint8 sprite_idx = player->sprite_idx;

	if (!((player->counter / 16) & 0x1))
	{
		sprite_idx++;
	}

	int16 offs_x = 0;
	int16 offs_y = 0;

	switch (player->sprite_idx)
	{
		case 2:
			offs_y = -4;
			break;
		case 4:
			offs_x = 5;
			offs_y = -2;
			break;
		case 6:
			offs_x = 8;
			break;
	}

	if (offs_x)
	{
		if (player->obj.flip & GPU_SPRITE_FLIP_X)
		{
			offs_x = -offs_x;
		}
	}

	if (offs_y)
	{
		if (player->obj.flip & GPU_SPRITE_FLIP_Y)
		{
			offs_y = -offs_y;
		}
	}

	game_draw_obj(game, FILE_IMAGE_TORA, sprite_idx, &player->obj, offs_x, offs_y, GPU_SPRITE_PRIO_BG + 0x20);

	if (player->move_type == PLAYER_MOVE_TYPE_DIVE && player->move_dir != PLAYER_MOVE_DIR_NONE)
	{
		int8 atk_time;

		if (player->atk_counter < 8)
		{
			atk_time = player->atk_counter;
		}
		else
		{
			atk_time = 15 - player->atk_counter;
		}

		typedef struct atk_info_t
		{
			int8 x;
			int8 y;
			int8 w;
			int8 h;
			uint8 sprite_idx;
			uint8 flip;
			uint8 prio;
		} atk_info_t;

		static const atk_info_t atk_info_table[] =
		{
			{   0,   0,   0,   0, 0, 0, 0 },
			{  -8, -16,   0, -16, 10, 2, 1 },
			{  -8,   8,   0,  16, 10, 0, 1 },
			{ -20,   0, -16,   0, 16, 0, 1 },
			{ -15, -12, -11, -11, 13, 2, 1 },
			{ -15,  12, -11,  11, 13, 0, 1 },
			{  12,   0,  16,   0, 16, 1, 1 },
			{  15, -12,  11, -11, 13, 3, 1 },
			{  15,  12,  11,  11, 13, 1, 1 },
		};

		const atk_info_t* atk_info = &atk_info_table[player->move_dir];
		int16 x = GAME2DRAW(player->obj.x) + atk_info->x + offs_x;
		int16 y = GAME2DRAW(player->obj.y) + atk_info->y + offs_y;
		uint8 prio = GPU_SPRITE_PRIO_BG + 0x20 + atk_info->prio;

		if (atk_time)
		{
			player->atk_x = atk_info->w * 8 * atk_time / 16;
			player->atk_y = atk_info->h * 8 * atk_time / 16;
			player->atk_w = 12;
			player->atk_h = 12;

			game_draw_sprite(game, FILE_IMAGE_TORA, atk_info->sprite_idx + 1, x + player->atk_x, y + player->atk_y, atk_info->flip, prio);

			for (int8 i = 0; i < atk_time; ++i)
			{
				game_draw_sprite(game, FILE_IMAGE_TORA, atk_info->sprite_idx + 2, x + (i * atk_info->w / 2), y + (i * atk_info->h / 2), atk_info->flip, prio);
			}
		}
		else
		{
			game_draw_sprite(game, FILE_IMAGE_TORA, atk_info->sprite_idx, x, y, atk_info->flip, prio);

			player->atk_w = 0;
			player->atk_h = 0;
		}
	}
}

void game_draw_enemy(game_t* game, enemy_task_t* enemy)
{
	if (enemy->dmg_time & 0x1)
	{
		return;
	}

	uint32 file_idx = enemy->file_idx;
	uint32 sprite_idx = 0;
	uint8 prio = GPU_SPRITE_PRIO_BG + 0x10;

	switch (enemy->file_idx)
	{
		case FILE_IMAGE_SHARK:
		case FILE_IMAGE_SEA_URCHIN:
		case FILE_IMAGE_TURTLE:
		case FILE_IMAGE_JELLYFISH:
		case FILE_IMAGE_SEA_SLUG:
		case FILE_IMAGE_MONKFISH:
		case FILE_IMAGE_EEL:
		case FILE_IMAGE_CRAB:
		case FILE_IMAGE_SHARK_RED:
		{
			sprite_idx = ((enemy->counter / 4) & 0x1) ? 1 : 0;

			break;
		}
		case FILE_IMAGE_TENTACLE:
		{
			static const uint8 SpriteTable[] = { 0, 0, 0, 1, 2, 0, };

			sprite_idx = SpriteTable[enemy->tentacle.seg_idx];

			break;
		}
		case FILE_IMAGE_BONUS:
		{
			enemy->obj.flip = ((enemy->counter / 32) & 0x1) ? GPU_SPRITE_FLIP_X : 0;

			prio = GPU_SPRITE_PRIO_BG;

			break;
		}
		case FILE_MAX:
		{
			return;
		}
	}

	game_draw_obj(game, file_idx, sprite_idx, &enemy->obj, 0, 0, prio);
}

void game_draw_item(game_t* game, item_task_t* item)
{
	uint32 file_idx = 0;
	uint32 sprite_idx = 0;
	uint8 prio = GPU_SPRITE_PRIO_BG + 0x10;

	if (item->state == ITEM_STATE_DROP)
	{
		if (item->counter < 30)
		{
			return;
		}

		if (item->counter & 0x1)
		{
			return;
		}

		file_idx = FILE_IMAGE_ITEMS;
		sprite_idx = 3 + item->tile;
	}
	else
	{
		if (item->state != ITEM_STATE_SPAWN)
		{
			prio = GPU_SPRITE_PRIO_FG;
		}

		if (TILE_IS_ITEM(item->tile))
		{
			file_idx = FILE_IMAGE_ITEMS;
			sprite_idx = item_table[TILE_ITEMID(item->tile)].type;
		}
		else if (TILE_IS_ETHER(item->tile))
		{
			file_idx = FILE_IMAGE_ETHER;

			if ((item->counter / 16) & 0x3)
			{
				sprite_idx = 0;
			}
			else
			{
				sprite_idx = (item->counter / 4) & 0x3;
			}
		}
	}

	game_draw_obj(game, file_idx, sprite_idx, &item->obj, 0, 0, prio);
}

void game_draw_effect(game_t* game, effect_task_t* effect)
{
	if (effect->pause)
	{
		return;
	}

	game_draw_obj(game, FILE_IMAGE_AIR_BUBBLE, 0, &effect->obj, 0, 0, GPU_SPRITE_PRIO_FG);
}

void game_draw_sprite(game_t* game, uint32 file_idx, uint32 sprite_idx, int16 x, int16 y, uint8 flip, uint8 prio)
{
	sprite_file_t* sprite_file = &game->sprite_files[file_idx];

	if (sprite_file->sprite_num)
	{
		uint8 obj_idx = game_gpu_obj_add(game);

		if (obj_idx != UINT8_MAX)
		{
			const sprite_t* sprite = &sprite_file->sprites[sprite_idx];

			gpu_obj_draw(&game->gpu, obj_idx, flip ^ sprite->flip, prio, x + sprite->offs_x, y + sprite->offs_y, sprite_file->tile_idx + sprite->tile_offs, sprite_file->pal_idx, sprite->w, sprite->h);
		}
	}
}

void game_draw_obj(game_t* game, uint32 file_idx, uint32 sprite_idx, const object_t* obj, int16 offs_x, int16 offs_y, uint8 prio)
{
	sprite_file_t* sprite_file = &game->sprite_files[file_idx];

	if (sprite_file->sprite_num)
	{
		const sprite_t* sprite = &sprite_file->sprites[sprite_idx];

		int16 x = GAME2DRAW(obj->x) + offs_x;
		int16 y = GAME2DRAW(obj->y) + offs_y;

		if (obj->flip & GPU_SPRITE_FLIP_X)
		{
			x -= sprite->offs_x + sprite->w;
		}
		else
		{
			x += sprite->offs_x;
		}

		if (obj->flip & GPU_SPRITE_FLIP_Y)
		{
			y -= sprite->offs_y + sprite->h;
		}
		else
		{
			y += sprite->offs_y;
		}

		if (ISIN(y, -sprite->h, GPU_SCR_H))
		{
			uint8 obj_idx = game_gpu_obj_add(game);

			if (obj_idx != UINT8_MAX)
			{
				gpu_obj_draw(&game->gpu, obj_idx, obj->flip ^ sprite->flip, prio, x, y, sprite_file->tile_idx + sprite->tile_offs, sprite_file->pal_idx, sprite->w, sprite->h);
			}
		}
	}
}

void game_fg_draw_tile(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint16 tile_idx, uint8 w, uint8 h)
{
	for (uint8 offs_y = 0; offs_y < h; ++offs_y)
	{
		for (uint8 offs_x = 0; offs_x < w; ++offs_x)
		{
			gpu_fg_draw(&game->gpu, x + offs_x, y + offs_y, tile_idx, pal_idx);
		}
	}
}

void game_fg_draw_text(game_t* game, uint8 x, uint8 y, uint8 pal_idx, const char* text)
{
	uint8 cursor = 0;
	char c;

	while ((c = text[cursor]))
	{
		gpu_fg_draw(&game->gpu, x + cursor, y, c, pal_idx);

		cursor++;
	}
}

void game_fg_draw_number(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint32 val, uint8 ez_mode)
{
	char text[8];

	number_to_text(text, val, ez_mode);
	game_fg_draw_text(game, x, y, pal_idx, text);
}

void game_fg_clear(game_t* game, uint8 x, uint8 y, uint8 w, uint8 h)
{
	game_fg_draw_tile(game, x, y, 0, 0, w, h);
}

void game_bg_draw_tile(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint16 tile_idx, uint8 w, uint8 h)
{
	for (uint8 offs_y = 0; offs_y < h; ++offs_y)
	{
		for (uint8 offs_x = 0; offs_x < w; ++offs_x)
		{
			gpu_bg_draw(&game->gpu, x + offs_x, y + offs_y, tile_idx, pal_idx);
		}
	}
}

void game_bg_draw_text(game_t* game, uint8 x, uint8 y, uint8 pal_idx, const char* text)
{
	uint8 cursor = 0;
	char c;

	while ((c = text[cursor]))
	{
		gpu_bg_draw(&game->gpu, x + cursor, y, c, pal_idx);

		cursor++;
	}
}

void game_bg_draw_number(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint32 val, uint8 ez_mode)
{
	char text[8];

	number_to_text(text, val, ez_mode);
	game_bg_draw_text(game, x, y, pal_idx, text);
}

void game_bg_clear(game_t* game, uint8 x, uint8 y, uint8 w, uint8 h)
{
	game_bg_draw_tile(game, x, y, 0, 0, w, h);
}

void game_ui_draw_tile(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint16 tile_idx, uint8 w, uint8 h)
{
	for (uint8 offs_y = 0; offs_y < h; ++offs_y)
	{
		for (uint8 offs_x = 0; offs_x < w; ++offs_x)
		{
			gpu_ui_draw(&game->gpu, x + offs_x, y + offs_y, tile_idx, pal_idx);
		}
	}
}

void game_ui_draw_text(game_t* game, uint8 x, uint8 y, uint8 pal_idx, const char* text)
{
	uint32 cursor = 0;
	char c;

	while ((c = text[cursor]))
	{
		gpu_ui_draw(&game->gpu, x + cursor, y, c, pal_idx);

		cursor++;
	}
}

void game_ui_draw_number(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint32 val, uint8 ez_mode)
{
	char text[8];

	number_to_text(text, val, ez_mode);
	game_ui_draw_text(game, x, y, pal_idx, text);
}

void game_ui_draw_sprite(game_t* game, uint8 x, uint8 y, uint8 file_idx, uint16 tile_idx, uint8 w, uint8 h)
{
	game_ui_draw_tile(game, x, y, game->sprite_files[file_idx].pal_idx, game->sprite_files[file_idx].tile_idx + tile_idx, w, h);
}

void game_ui_clear(game_t* game, uint8 x, uint8 y, uint8 w, uint8 h)
{
	game_ui_draw_tile(game, x, y, 0, 0, w, h);
}

uint8 game_fadein(game_t* game)
{
	if (game->fade < UINT8_MAX - 16)
	{
		game->fade += 16;
	}
	else
	{
		game->fade = UINT8_MAX;
	}

	gpu_fade(&game->gpu, game->fade);

	return game->fade == UINT8_MAX;
}

uint8 game_fadeout(game_t* game)
{
	if (game->fade > 16)
	{
		game->fade -= 16;
	}
	else
	{
		game->fade = 0;
	}

	gpu_fade(&game->gpu, game->fade);

	return game->fade == 0;
}

void game_fadeset(game_t* game, uint8 val)
{
	game->fade = val;

	gpu_fade(&game->gpu, game->fade);
}

uint8 game_gpu_obj_add(game_t* game)
{
	if (game->obj_num >= GPU_OBJ_MAX)
	{
		return UINT8_MAX;
	}

	return game->obj_num++;
}

void game_gpu_obj_clear(game_t* game)
{
	game->obj_num = 0;

	for (uint8 i = 0; i < GPU_OBJ_MAX; ++i)
	{
		gpu_obj_clear(&game->gpu, i);
	}
}

void* game_obj_create(game_t* game)
{
	task_t* task = task_create(&game->task_sys);

	if (task)
	{
		return task;
	}

	task = task_enum(&game->task_sys, NULL);

	while (task)
	{
		if (task->move_cb == game_move_effect)
		{
			game_obj_remove(game, task);

			return task_create(&game->task_sys);
		}

		task = task_enum(&game->task_sys, task);
	}

	return NULL;
}

void game_obj_remove(game_t* game, void* obj)
{
	task_remove(&game->task_sys, obj);
}

uint32 game_load_sprite_file(game_t* game, uint16 tile_idx, uint8 pal_idx, uint8 file_idx)
{
	sprite_file_t* sprite_file = &game->sprite_files[file_idx];

	sprite_file->tile_idx = tile_idx;
	sprite_file->pal_idx = pal_idx;

	const image_t* img = arc_read(game->arc, file_idx);

	sprite_file->sprite_num = img->sprite_num;

	if (sprite_file->sprite_num)
	{
		uint32 tile_num;

		if (img->width)
		{
			tile_num = (img->width * img->height) / (GPU_TILE_WH * GPU_TILE_WH);
		}
		else
		{
			tile_num = img->tile_num;
		}

		sprite_file->sprites = MEMOFFS(img, sizeof(image_t) + img->pal_size + tile_num * GPU_TILE_WH * sizeof(uint32));
	}
	else
	{
		sprite_file->sprites = NULL;
	}

	game_load_img_pal(game, pal_idx, img);

	return game_load_sprite_img(game, tile_idx, img);
}

uint32 game_load_sprite_img(game_t* game, uint16 tile_idx, const image_t* img)
{
	if (!img)
	{
		return 0;
	}

	const uint32* data = MEMOFFS(img, sizeof(image_t) + img->pal_size);

	if (img->width)
	{
		uint32 stride = ((img->width - 1) / GPU_TILE_WH) + 1;

		for (uint16 y = 0; y < img->height; ++y)
		{
			for (uint16 x = 0; x < img->width; x += GPU_TILE_WH)
			{
				gpu_tile_set_row(&game->gpu, tile_idx, MAD(y, stride, x / GPU_TILE_WH), _byteswap_ulong(data[MAD(y, stride, x / GPU_TILE_WH)]));
			}
		}

		return (img->width * img->height) / (GPU_TILE_WH * GPU_TILE_WH);
	}
	else
	{
		gpu_tile_copy(&game->gpu, tile_idx, data, img->tile_num * GPU_TILE_WH * GPU_BPP);

		return img->tile_num;
	}
}

void game_load_img(game_t* game, uint16 tile_idx, const image_t* img, uint16 offs, uint8 w, uint8 h)
{
	const uint32* data = MEMOFFS(img, sizeof(image_t) + img->pal_size + offs);
	uint32 stride = ((w - 1) / GPU_TILE_WH) + 1;

	for (uint16 y = 0; y < h; ++y)
	{
		for (uint16 x = 0; x < w; x += GPU_TILE_WH)
		{
			gpu_tile_set_row(&game->gpu, tile_idx, MAD(x / GPU_TILE_WH, h, y), _byteswap_ulong(data[MAD(y, stride, x / GPU_TILE_WH)]));
		}
	}
}

void game_load_img_pal(game_t* game, uint8 pal_idx, const image_t* img)
{
	if (img->pal_size)
	{
		gpu_pal_set(&game->gpu, pal_idx, MEMOFFS(img, sizeof(image_t)));
	}
}

void game_vm_update(game_t* game)
{
	while (game->title.vm_stream)
	{
		uint8 op;

		do
		{
			op = game_vm_stream_u8(game);
		}
		while (!op);

		if (ISIN(op, 16, 63))
		{
			uint8 reg = game_vm_stream_u8(game);
			uint8 val = game_vm_stream_u8(game);

			if (!(op & VM_OP_CONSTANT))
			{
				val = game->title.vm_regs[val];
			}

			op &= ~VM_OP_CONSTANT;

			switch (op)
			{
				case VM_OP_SET:
				{
					game->title.vm_regs[reg] = val;

					break;
				}
				case VM_OP_ADD:
				{
					game->title.vm_regs[reg] += val;

					break;
				}
				case VM_OP_AND:
				{
					game->title.vm_regs[reg] &= val;

					break;
				}
				case VM_OP_OR:
				{
					game->title.vm_regs[reg] |= val;

					break;
				}
				case VM_OP_XOR:
				{
					game->title.vm_regs[reg] ^= val;

					break;
				}
				case VM_OP_LSH:
				{
					game->title.vm_regs[reg] <<= val;

					break;
				}
				case VM_OP_RSH:
				{
					game->title.vm_regs[reg] >>= val;

					break;
				}
				case VM_OP_MUL:
				{
					game->title.vm_regs[reg] *= val;

					break;
				}
				case VM_OP_DIV:
				{
					game->title.vm_regs[reg] /= val;

					break;
				}
				case VM_OP_MOD:
				{
					game->title.vm_regs[reg] %= val;

					break;
				}
				case VM_OP_JEQ:
				{
					uint16 offs = game_vm_stream_u16(game);

					if (game->title.vm_regs[reg] == val)
					{
						game->title.vm_stream = MEMOFFS(game->title.vm_data, offs);
					}

					break;
				}
				case VM_OP_JNE:
				{
					uint16 offs = game_vm_stream_u16(game);

					if (game->title.vm_regs[reg] != val)
					{
						game->title.vm_stream = MEMOFFS(game->title.vm_data, offs);
					}

					break;
				}
				case VM_OP_JL:
				{
					uint16 offs = game_vm_stream_u16(game);

					if (game->title.vm_regs[reg] < val)
					{
						game->title.vm_stream = MEMOFFS(game->title.vm_data, offs);
					}

					break;
				}
			}
		}
		else if (ISIN(op, VM_OP_OBJ_INIT, 78))
		{
			vm_obj_t* obj = &game->title.vm_objs[game_vm_stream_u8(game)];

			switch (op)
			{
				case VM_OP_OBJ_INIT:
				{
					obj->pack_idx = game_vm_stream_u8(game);
					obj->sprite_idx = game_vm_stream_u8(game);
					obj->x = game_vm_stream_i8(game) << 8;
					obj->y = game_vm_stream_i8(game) << 8;
					obj->flip = game_vm_stream_u8(game);
					obj->prio = game_vm_stream_u8(game);

					uint16 anim_offs = game_vm_stream_u16(game);

					if (anim_offs)
					{
						obj->anim_data = MEMOFFS(game->title.vm_data, anim_offs);
						obj->anim_wait = 0;
						obj->anim_cursor = 0;
					}
					else
					{
						obj->anim_data = NULL;
					}

					break;
				}
				case VM_OP_OBJ_CLEAR:
				{
					obj->pack_idx = 0;

					break;
				}
				case VM_OP_OBJ_SET:
				{
					uint8 Flags = game_vm_stream_u8(game);

					if (Flags & 0x1)
					{
						obj->pack_idx = game_vm_stream_u8(game);
					}

					if (Flags & 0x2)
					{
						obj->sprite_idx = game_vm_stream_u8(game);
					}

					if (Flags & 0x4)
					{
						obj->x = game_vm_stream_i8(game) << 8;
					}

					if (Flags & 0x8)
					{
						obj->y = game_vm_stream_i8(game) << 8;
					}

					if (Flags & 0x10)
					{
						obj->flip = game_vm_stream_u8(game);
					}

					if (Flags & 0x20)
					{
						obj->prio = game_vm_stream_u8(game);
					}

					if (Flags & 0x40)
					{
						uint16 anim_offs = game_vm_stream_u16(game);

						if (anim_offs)
						{
							obj->anim_data = MEMOFFS(game->title.vm_data, anim_offs);
							obj->anim_wait = 0;
							obj->anim_cursor = 0;
						}
						else
						{
							obj->anim_data = NULL;
						}
					}

					break;
				}
				case VM_OP_OBJ_ADD:
				{
					uint8 flags = game_vm_stream_u8(game);

					if (flags & 0x1)
					{
						obj->pack_idx += game_vm_stream_i8(game);
					}

					if (flags & 0x2)
					{
						obj->sprite_idx += game_vm_stream_i8(game);
					}

					if (flags & 0x4)
					{
						obj->x += game_vm_stream_i8(game) << 6;
					}

					if (flags & 0x8)
					{
						obj->y += game_vm_stream_i8(game) << 6;
					}

					if (flags & 0x10)
					{
						obj->flip += game_vm_stream_i8(game);
					}

					if (flags & 0x20)
					{
						obj->prio += game_vm_stream_i8(game);
					}

					break;
				}
				case VM_OP_OBJ_ANIM:
				{
					uint16 anim_offs = game_vm_stream_u16(game);

					if (anim_offs)
					{
						obj->anim_data = MEMOFFS(game->title.vm_data, anim_offs);
						obj->anim_wait = 0;
						obj->anim_cursor = 0;
					}
					else
					{
						obj->anim_data = NULL;
					}

					break;
				}
			}
		}
		else if (op == VM_OP_JUMP)
		{
			game->title.vm_stream = MEMOFFS(game->title.vm_data, game_vm_stream_u16(game));
		}
		else if (op == VM_OP_WAIT)
		{
			break;
		}
		else if (op == VM_OP_CLEAR)
		{
			game->title.vm_stream = NULL;

			for (uint32 i = 0; i < ARRAYNUM(game->title.vm_objs); ++i)
			{
				game->title.vm_objs[i].pack_idx = 0;
			}
		}
		else
		{
			game->title.vm_stream = NULL;
		}
	}

	for (uint32 i = 0; i < ARRAYNUM(game->title.vm_objs); ++i)
	{
		vm_obj_t* obj = &game->title.vm_objs[i];

		if (obj->pack_idx != 0)
		{
			if (obj->anim_data)
			{
				if (obj->anim_wait)
				{
					obj->anim_wait--;
				}
				else
				{
					obj->anim_wait = obj->anim_data[obj->anim_cursor] - 1;
					obj->sprite_idx = obj->anim_data[obj->anim_cursor + 1];

					if (obj->anim_data[obj->anim_cursor + 2])
					{
						obj->anim_cursor += 2;
					}
					else
					{
						obj->anim_cursor = 0;
					}
				}
			}
		}
	}
}

int8 game_vm_stream_i8(game_t* game)
{
	const uint8 val = *(const uint8*)game->title.vm_stream;

	game->title.vm_stream = MEMOFFS(game->title.vm_stream, sizeof(uint8));

	return val;
}

int8 game_vm_stream_u8(game_t* game)
{
	const int8 val = *(const int8*)game->title.vm_stream;

	game->title.vm_stream = MEMOFFS(game->title.vm_stream, sizeof(int8));

	return val;
}

uint16 game_vm_stream_u16(game_t* game)
{
	const uint16 val = *(const uint16*)game->title.vm_stream;

	game->title.vm_stream = MEMOFFS(game->title.vm_stream, sizeof(uint16));

	return val;
}

void game_cell_anime_init(game_t* game, uint32 idx)
{
	game->cell_anime_img = arc_read(game->arc, FILE_IMAGE_CELL_ANIME + idx);
	game->cell_anime_idx = 0;
	game->cell_anime_wait = 1;

	csvb_t csvb;

	csvb_init(&csvb, arc_read(game->arc, FILE_CSVB_CELL_ANIME), arc_size(game->arc, FILE_CSVB_CELL_ANIME));

	game->cell_anime_time[0] = csvb_read_int(&csvb, idx, 0);
	game->cell_anime_time[1] = csvb_read_int(&csvb, idx, 1);

	csvb_fini(&csvb);
}

void game_cell_anime(game_t* game)
{
	game->cell_anime_wait--;

	if (game->cell_anime_wait)
	{
		return;
	}

	game->cell_anime_idx = 1 - game->cell_anime_idx;
	game->cell_anime_wait = game->cell_anime_time[game->cell_anime_idx];

	game_load_img(game, 0x140, game->cell_anime_img, game->cell_anime_idx << 11, 64, 64);
}

void game_bgm_play(game_t* game, uint8 idx)
{
	sound_bgm_play(&game->sound_sys, idx);
}

void game_se_play(game_t* game, uint8 idx)
{
	sound_se_play(&game->sound_sys, idx);
}

void game_bgm_pitch(game_t* game, float pitch)
{
	sound_bgm_pitch(&game->sound_sys, pitch);
}

void game_get_player_xy(game_t* game, int32* x, int32* y)
{
	*x = game->play.player->obj.x;
	*y = game->play.player->obj.y + SCROLL2GAME(game->play.scroll_y);
}

void game_get_player_dxdy(game_t* game, int32 x, int32 y, int32* dx, int32* dy)
{
	int32 player_x;
	int32 player_y;

	game_get_player_xy(game, &player_x, &player_y);

	*dx = x - player_x;
	*dy = y - player_y;
}

uint32 game_get_player_dist(game_t* game, int32 x, int32 y)
{
	int32 dx;
	int32 dy;

	game_get_player_dxdy(game, x, y, &dx, &dy);

	float fdx = GAME2DRAW(dx);
	float fdy = GAME2DRAW(dy);

	return (uint32)sqrtf((fdx * fdx) + (fdy * fdy));
}

uint8 game_get_player_vxvy(game_t* game, int32 x, int32 y, int32 spd, int32* vx, int32* vy)
{
	int32 dx;
	int32 dy;

	game_get_player_dxdy(game, x, y, &dx, &dy);

	return game_get_vxvy(game, dx, dy, spd, vx, vy);
}

uint8 game_get_vxvy(game_t* game, int32 dx, int32 dy, int32 spd, int32* vx, int32* vy)
{
	float fdx = GAME2DRAW(dx);
	float fdy = GAME2DRAW(dy);
	float dist = sqrtf((fdx * fdx) + (fdy * fdy));

	if (dist >= GAME2DRAW(spd))
	{
		float ratio = -spd / dist;

		*vx = (int32)(fdx * ratio);
		*vy = (int32)(fdy * ratio);

		return 1;
	}
	else
	{
		*vx = 0;
		*vy = 0;

		return 0;
	}
}

effect_task_t* game_air_bubble(game_t* game)
{
	effect_task_t* effect = task_create(&game->task_sys);

	if (effect)
	{
		effect->obj.task.move_cb = game_move_effect;
		effect->obj.task.draw_cb = game_draw_effect;
		effect->obj.flip = 0;
		effect->obj.x = DRAW2GAME(1024);
		effect->obj.y = DRAW2GAME(0);
		effect->obj.hit_flag = HIT_FLAG_NONE;
		effect->pause = 0;
		effect->counter = game_rand_max(game, UINT8_MAX);
		effect->life = 0;
		effect->spd_x = 0;
		effect->spd_y = game_rand_max(game, UINT8_MAX) - 640;
	}

	return effect;
}

void game_add_hp(game_t* game, int32 val)
{
	if (val)
	{
		game->play.hp = CLAMP(game->play.hp + val, 0, game->play.hp_max);
		game->play.hp_anim_time = 7;
	}
}

void game_add_score(game_t* game, uint32 val)
{
	game->play.score += val;
	game->play.score_anim_time = 30;
}

void game_add_ether(game_t* game, uint32 val)
{
	game->play.ether += val;
}

void game_add_bonus_ether(game_t* game, uint32 val)
{
	game->play.ether_bonus += val;
}

void game_add_kill(game_t* game)
{
	game->play.kills += 1;
}

uint8 game_loot_item(game_t* game, uint8 id, item_task_t* item)
{
	if (item_table[id].type == ITEM_TYPE_WEAPON)
	{
		if (game->play.weapon)
		{
			game_add_score(game, item_table[id].score);

			return UINT8_MAX;
		}
		else
		{
			game->play.weapon = item;

			return 0;
		}
	}
	else
	{
		uint8 slot_idx = 0;

		for (uint32 i = 0; i < LOOT_MAX; ++i)
		{
			if (!game->play.loot[i])
			{
				game->play.loot[i] = item;

				return slot_idx;
			}
			else if (item_table[TILE_ITEMID(game->play.loot[i]->tile)].type == ITEM_TYPE_SMALL_TBOX)
			{
				slot_idx++;
			}
		}

		return UINT8_MAX;
	}
}

uint8 game_get_stage_tile(game_t* game, uint32 x, uint32 y)
{
	const uint8* tiles = MEMOFFS(game->stage, sizeof(stage_t));

	return tiles[MAD(y, game->stage->width, x)];
}

object_t* game_coli_check(game_t* game, uint8* out_stage_hit_flag)
{
	player_task_t* player = game->play.player;

	if (!player->obj.hit_flag || player->obj.hit_by_flag)
	{
		return NULL;
	}

	uint8 stage_hit_flag = game_coli_check_stage(game);

	if (stage_hit_flag && !game->title.cheat_no_clip)
	{
		*out_stage_hit_flag = stage_hit_flag;

		player->obj.hit_by_flag |= HIT_FLAG_STAGE;
	}
	else
	{
		object_t* obj = task_enum(&game->task_sys, NULL);

		while (obj)
		{
			int32 player_min_x = GAME2DRAW(player->obj.x) + player->obj.hit_x;
			int32 player_min_y = GAME2DRAW(player->obj.y) + player->obj.hit_y;
			int32 player_max_x = player_min_x + player->obj.hit_w;
			int32 player_max_y = player_min_y + player->obj.hit_h;

			if (obj != &player->obj && !obj->hit_by_flag)
			{
				uint32 obj_hit_flag = obj->hit_flag & player->obj.hit_flag;

				if (obj_hit_flag)
				{
					int32 obj_min_x = GAME2DRAW(obj->x) + obj->hit_x;
					int32 obj_min_y = GAME2DRAW(obj->y) + obj->hit_y;
					int32 obj_max_x = obj_min_x + obj->hit_w;
					int32 obj_max_y = obj_min_y + obj->hit_h;

					if (obj_min_x < player_max_x && obj_min_y < player_max_y && obj_max_x > player_min_x && obj_max_y > player_min_y)
					{
						player->obj.hit_by_flag |= obj_hit_flag;
						obj->hit_by_flag |= obj_hit_flag;

						return obj;
					}
				}
			}

			obj = task_enum(&game->task_sys, &obj->task);
		}
	}

	return NULL;
}

uint8 game_coli_check_stage(game_t* game)
{
	player_task_t* player = game->play.player;
	int32 hit_min_x = GAME2DRAW(player->obj.x) + player->obj.hit_x;

	if (hit_min_x > 512)
	{
		return 0;
	}

	int32 hit_min_y = GAME2DRAW(player->obj.y) + SCROLL2DRAW(game->play.scroll_y) + player->obj.hit_y;

	if (hit_min_y < 0)
	{
		return 3;
	}

	int32 hit_max_x = hit_min_x + player->obj.hit_w - 1;
	int32 hit_max_y = hit_min_y + player->obj.hit_h - 1;

	hit_min_x = CLAMP(hit_min_x, 0, PLAY_AREA_W - 1);
	hit_min_y = CLAMP(hit_min_y, 0, PLAY_AREA_H + SCROLL2DRAW(game->play.scroll_y_max) - 1);
	hit_max_x = CLAMP(hit_max_x, 0, PLAY_AREA_W - 1);
	hit_max_y = CLAMP(hit_max_y, 0, PLAY_AREA_H + SCROLL2DRAW(game->play.scroll_y_max) - 1);

	int32 tile_min_x = hit_min_x / 16;
	int32 tile_min_y = hit_min_y / 16;
	int32 tile_max_x = hit_max_x / 16;
	int32 tile_max_y = hit_max_y / 16;
	uint8 hit_flag = 0;

	if (TILE_IS_WALL(game_get_stage_tile(game, tile_min_x, tile_min_y)))
	{
		hit_flag |= 0x1;
	}

	if (TILE_IS_WALL(game_get_stage_tile(game, tile_max_x, tile_min_y)))
	{
		hit_flag |= 0x2;
	}

	if (TILE_IS_WALL(game_get_stage_tile(game, tile_min_x, tile_max_y)))
	{
		hit_flag |= 0x4;
	}

	if (TILE_IS_WALL(game_get_stage_tile(game, tile_max_x, tile_max_y)))
	{
		hit_flag |= 0x8;
	}

	return hit_flag;
}

enemy_task_t* game_atk_check(game_t* game)
{
	player_task_t* player = game->play.player;

	if (!player->obj.hit_flag || player->obj.hit_by_flag)
	{
		return NULL;
	}

	if (!player->atk_w)
	{
		return NULL;
	}

	object_t* obj = task_enum(&game->task_sys, NULL);

	while (obj)
	{
		int32 atk_min_x = GAME2DRAW(player->obj.x) + player->atk_x - player->atk_w;
		int32 atk_min_y = GAME2DRAW(player->obj.y) + player->atk_y - player->atk_h;
		int32 atk_max_x = atk_min_x + (player->atk_w * 2);
		int32 atk_max_y = atk_min_y + (player->atk_h * 2);

		if (obj != &player->obj && !obj->hit_by_flag)
		{
			uint32 obj_hit_flag = obj->hit_flag & HIT_FLAG_ENEMY;

			if (obj_hit_flag)
			{
				int32 obj_min_x = GAME2DRAW(obj->x) + obj->hit_x;
				int32 obj_min_y = GAME2DRAW(obj->y) + obj->hit_y;
				int32 obj_max_x = obj_min_x + obj->hit_w;
				int32 obj_max_y = obj_min_y + obj->hit_h;

				if (obj_min_x < atk_max_x && obj_min_y < atk_max_y && obj_max_x > atk_min_x && obj_max_y > atk_min_y)
				{
					obj->hit_by_flag |= obj_hit_flag;

					if (obj->hit_flag & HIT_FLAG_ENEMY)
					{
						return obj;
					}
					else
					{
						return NULL;
					}
				}
			}
		}

		obj = task_enum(&game->task_sys, &obj->task);
	}

	return NULL;
}

void game_atk_dmg(game_t* game, enemy_task_t* enemy, uint8 move_dir)
{
	game_se_play(game, 2);

	if (!enemy->dmg_time)
	{
		enemy->dmg_time = enemy->back_time;
		enemy->dmg_dir = move_dir;
	}

	if (enemy->hp > 1)
	{
		return;
	}

	game_add_score(game, enemy->score);
	game_add_kill(game);

	int32 rand_drop = game_rand_max(game, 10000);

	if (rand_drop < (game->play.star_rate + game->play.o2_rate))
	{
		item_task_t* itm = game_obj_create(game);

		itm->obj.task.move_cb = game_move_item;
		itm->obj.task.draw_cb = game_draw_item;
		itm->obj.x = enemy->obj.x;
		itm->obj.y = enemy->obj.y;
		itm->obj.flip = 0;
		itm->obj.hit_x = -10;
		itm->obj.hit_y = -10;
		itm->obj.hit_w = 20;
		itm->obj.hit_h = 20;
		itm->tile = rand_drop < game->play.star_rate ? 0 : 1;
		itm->state = ITEM_STATE_DROP;
		itm->counter = 150;
		itm->anim_time = 0;
		itm->abs_x = enemy->abs_x;
		itm->abs_y = enemy->abs_y;
		itm->obj.hit_flag = HIT_FLAG_ITEM;
		itm->score = 0;
	}
}

uint32 game_rand(game_t* game)
{
	game->rand_seed = 1103515245 * game->rand_seed + 12345;

	return game->rand_seed;
}

int32 game_rand_max(game_t* game, int32 max)
{
	return game_rand(game) % max;
}

int32 game_rand_minmax(game_t* game, int32 min, int32 max)
{
	return min + (game_rand(game) % (max - min));
}

rgba_t* game_get_framebuffer(game_t* game)
{
	return gpu_get_framebuffer(&game->gpu);
}
