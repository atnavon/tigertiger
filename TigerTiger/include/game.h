#pragma once

#include "common.h"
#include "gpu.h"
#include "sound.h"
#include "task.h"

#define PLAY_AREA_W 256
#define PLAY_AREA_H 240
#define BONUS_AREA_OFFS 1024
#define STAGE_MAX 8
#define STAGE_OBJ_MAX 256
#define LOOT_MAX 16
#define TASK_MAX 256

#define FILE_MAX 64
#define FILE_FONT 0
#define FILE_IMAGE_UI 1
#define FILE_IMAGE_TITLE 2
#define FILE_TITLE_VM 6
#define FILE_CSVB_CELL_ANIME 7
#define FILE_IMAGE_CELL_ANIME 8
#define FILE_IMAGE_STAGE_MAP 16
#define FILE_IMAGE_STAGE_MARKER 17
#define FILE_CSVB_STAGE_SELECT 18
#define FILE_IMAGE_STAGE_BACKGROUND 24
#define FILE_ARCHIVE_STAGE 32
#define FILE_IMAGE_ETHER 48
#define FILE_IMAGE_ITEMS 49
#define FILE_IMAGE_AIR_BUBBLE 50
#define FILE_IMAGE_TORA 51
#define FILE_IMAGE_SHARK 52
#define FILE_IMAGE_SEA_URCHIN 53
#define FILE_IMAGE_TURTLE 54
#define FILE_IMAGE_JELLYFISH 55
#define FILE_IMAGE_REWARDS 56
#define FILE_IMAGE_SEA_SLUG 57
#define FILE_IMAGE_MONKFISH 58
#define FILE_IMAGE_EEL 59
#define FILE_IMAGE_CRAB 60
#define FILE_IMAGE_TENTACLE 61
#define FILE_IMAGE_BONUS 62
#define FILE_IMAGE_SHARK_RED 63

#define TENTACLE_SEG_MAX 4
#define TENTACLE_DELAY_MAX 12

#define EZ_MODE_FLAG 0x80000000

#define CHEAT_INPUT_MAX 5
#define CHEAT_STAGE_UNLOCK 0
#define CHEAT_GODMODE 1
#define CHEAT_NOCLIP 2

enum input
{
	INPUT_CONFIRM = 0x1,
	INPUT_CANCEL = 0x2,
	INPUT_UP = 0x4,
	INPUT_RIGHT = 0x8,
	INPUT_DOWN = 0x10,
	INPUT_LEFT = 0x20,
};

enum phase
{
	PHASE_BOOT,
	PHASE_TITLE,
	PHASE_SELECT,
	PHASE_PLAY,
	PHASE_CLEAR,
	PHASE_OVER,
	PHASE_NAME,
	PHASE_EXIT,
};

enum boot_step
{
	BOOT_STEP_INIT,
	BOOT_STEP_ROM_CHECK,
	BOOT_STEP_RAM_CHECK,
	BOOT_STEP_BACKUP,
	BOOT_STEP_READY,
};

enum title_step
{
	TITLE_STEP_INIT,
	TITLE_STEP_FADEIN,
	TITLE_STEP_TITLE,
	TITLE_STEP_FADEOUT,
};

enum select_step
{
	SELECT_STEP_INIT,
	SELECT_STEP_FADEIN,
	SELECT_STEP_SELECT,
	SELECT_STEP_FADEOUT,
};

enum game_step
{
	GAME_STEP_INIT,
	GAME_STEP_DIVE_START,
	GAME_STEP_DIVE,
	GAME_STEP_SURFACE_START,
	GAME_STEP_SURFACE,
	GAME_STEP_OVER,
	GAME_STEP_CLEAR,
	GAME_STEP_FADEOUT,
	GAME_STEP_BONUS_START,
	GAME_STEP_BONUS,
	GAME_STEP_BONUS_EXIT_START,
	GAME_STEP_BONUS_EXIT,
};

enum over_step
{
	OVER_STEP_INIT,
	OVER_STEP_FADEIN,
	OVER_STEP_INPUT_WAIT,
	OVER_STEP_FADEOUT,
};

enum clear_step
{
	CLEAR_STEP_INIT,
	CLEAR_STEP_FADEIN,
	CLEAR_STEP_SCORE,
	CLEAR_STEP_HP_BADGE,
	CLEAR_STEP_ETHER,
	CLEAR_STEP_ETHER_BADGE,
	CLEAR_STEP_TBOX1,
	CLEAR_STEP_TBOX2,
	CLEAR_STEP_TBOX3,
	CLEAR_STEP_PERFECT_BADGE,
	CLEAR_STEP_NO_KILL_CHECK,
	CLEAR_STEP_NO_KILL_BADGE,
	CLEAR_STEP_INPUT_WAIT,
	CLEAR_STEP_FADEOUT,
};

enum name_step
{
	NAME_STEP_INIT,
	NAME_STEP_FADEIN,
	NAME_STEP_INPUT,
	NAME_STEP_INPUT_WAIT,
	NAME_STEP_WAIT,
	NAME_STEP_FADEOUT,
};

enum vm_op
{
	VM_OP_CONSTANT = 0x1,
	VM_OP_SET = 16,
	VM_OP_ADD = 18,
	VM_OP_AND = 20,
	VM_OP_OR = 22,
	VM_OP_XOR = 24,
	VM_OP_LSH = 26,
	VM_OP_RSH = 28,
	VM_OP_MUL = 32,
	VM_OP_DIV = 34,
	VM_OP_MOD = 36,
	VM_OP_JEQ = 48,
	VM_OP_JNE = 50,
	VM_OP_JL = 52,
	VM_OP_OBJ_INIT = 64,
	VM_OP_OBJ_CLEAR = 65,
	VM_OP_OBJ_SET = 66,
	VM_OP_OBJ_ADD = 67,
	VM_OP_OBJ_ANIM = 68,
	VM_OP_JUMP = 253,
	VM_OP_WAIT = 254,
	VM_OP_CLEAR = 255,
};

typedef struct vm_obj_t
{
	uint8 pack_idx;
	uint8 sprite_idx;
	int16 x;
	int16 y;
	uint8 flip;
	uint8 prio;
	uint8 anim_wait;
	uint8 anim_cursor;
	const uint8* anim_data;
} vm_obj_t;

enum hit_flag
{
	HIT_FLAG_NONE = 0x0,
	HIT_FLAG_ENEMY = 0x2,
	HIT_FLAG_ITEM = 0x4,
	HIT_FLAG_STAGE = 0x80,
	HIT_FLAG_ALL = 0xFF,
};

typedef struct object_t
{
	task_t task;
	uint8 flip;
	int32 x;
	int32 y;
	uint8 hit_flag;
	uint8 hit_by_flag;
	int16 hit_x;
	int16 hit_y;
	uint8 hit_w;
	uint8 hit_h;
} object_t;

enum player_move_type
{
	PLAYER_MOVE_TYPE_DIVE,
	PLAYER_MOVE_TYPE_SURFACE,
	PLAYER_MOVE_TYPE_FLOAT,
};

enum player_move_dir
{
	PLAYER_MOVE_DIR_NONE,
	PLAYER_MOVE_DIR_N,
	PLAYER_MOVE_DIR_S,
	PLAYER_MOVE_DIR_W,
	PLAYER_MOVE_DIR_NW,
	PLAYER_MOVE_DIR_SW,
	PLAYER_MOVE_DIR_E,
	PLAYER_MOVE_DIR_NE,
	PLAYER_MOVE_DIR_SE,
};

typedef struct player_t
{
	object_t obj;
	uint8 move_type;
	uint8 dmg_counter;
	uint8 sprite_idx;
	uint8 counter;
	uint8 atk_counter;
	uint8 move_dir;
	uint8 scroll;
	uint8 atk_cancel;
	uint16 invuln_counter;
	uint16 spd_lr;
	uint16 spd_d;
	uint16 spd_u;
	uint16 scroll_threshold;
	int16 atk_x;
	int16 atk_y;
	uint8 atk_w;
	uint8 atk_h;
} player_task_t;

enum enemy_move_state
{
	ENEMY_MOVE_STATE_NONE,
	ENEMY_MOVE_STATE_RIGHT,
	ENEMY_MOVE_STATE_LEFT,
	ENEMY_MOVE_STATE_DOWN,
	ENEMY_MOVE_STATE_UP,
	ENEMY_MOVE_STATE_CHASE,
	ENEMY_MOVE_STATE_DASH,
	ENEMY_MOVE_STATE_RETURN,
	ENEMY_MOVE_STATE_BONUS_SPAWN,
};

enum enemy_move_type
{
	ENEMY_MOVE_TYPE_NONE,
	ENEMY_MOVE_TYPE_HOR,
	ENEMY_MOVE_TYPE_VERT,
	ENEMY_MOVE_TYPE_CHASE,
	ENEMY_MOVE_TYPE_ESCAPE,
	ENEMY_MOVE_TYPE_AMBUSH_HOR,
	ENEMY_MOVE_TYPE_AMBUSH_UP,
	ENEMY_MOVE_TYPE_AMBUSH_DOWN,
	ENEMY_MOVE_TYPE_AMBUSH_VERT,
	ENEMY_MOVE_TYPE_TENTACLE,
	ENEMY_MOVE_TYPE_BONUS,
	ENEMY_MOVE_TYPE_BONUS_SPAWN,
};

enum enemy_hit_type
{
	ENEMY_HIT_TYPE_ALWAYS,
	ENEMY_HIT_TYPE_TOP,
	ENEMY_HIT_TYPE_BOTTOM,
	ENEMY_HIT_TYPE_NEVER,
	ENEMY_HIT_TYPE_TOUCH,
};

typedef struct enemy_t
{
	object_t obj;
	uint8 tile;
	uint8 counter;
	uint8 move_state;
	uint8 move_time;
	uint8 dmg_time;
	uint8 dmg_dir;
	int32 abs_x;
	int32 abs_y;
	int32 spd_x;
	int32 spd_y;
	int32 start_x;
	int32 start_y;
	uint16 spd;
	uint16 range;
	uint16 search;
	uint16 act_spd;
	uint16 back_spd;
	uint8 back_time;
	uint8 hp;
	uint8 atk;
	uint16 score;
	uint8 move_type;
	uint8 hit_type;
	uint8 file_idx;
	struct
	{
		struct enemy_t* parent;
		uint8 seg_idx;
		uint8 active;
		struct
		{
			int8 x;
			int8 y;
			int8 rot;
			int8 delay[TENTACLE_DELAY_MAX];
		} seg[TENTACLE_SEG_MAX];
	} tentacle;
} enemy_task_t;

enum item_state
{
	ITEM_STATE_SPAWN,
	ITEM_STATE_LOOTING,
	ITEM_STATE_LOOT,
	ITEM_STATE_LOST,
	ITEM_STATE_DROP,
};

enum item_type
{
	ITEM_TYPE_SMALL_TBOX,
	ITEM_TYPE_BIG_TBOX,
	ITEM_TYPE_WEAPON,
};

typedef struct item_t
{
	object_t obj;
	uint8 tile;
	uint8 state;
	uint8 counter;
	uint8 anim_time;
	int32 abs_x;
	int32 abs_y;
	int32 loot_x;
	int32 loot_y;
	int32 loot_anim_x;
	int32 loot_anim_y;
	uint16 score;
} item_task_t;

typedef struct effect_t
{
	object_t obj;
	uint8 pause;
	uint8 counter;
	uint8 life;
	int32 spd_x;
	int32 spd_y;
} effect_task_t;

typedef struct stage_obj_t
{
	uint8 tile;
	uint16 x;
	uint16 y;
} stage_obj_t;

typedef struct save_data_t
{
	struct
	{
		uint32 score;
		char name[4];
	} leaderboard[5];
	uint32 clear_stage;
} save_t;

typedef struct arc_t
{
	uint32 data_start;
	uint32 toc[];
} arc_t;

typedef struct image_t
{
	uint16 bpp;
	uint16 pal_size;
	union
	{
		struct
		{
			uint16 _pad1;
			uint16 tile_num;
		};
		struct
		{
			uint16 width;
			uint16 height;
		};
	};
	uint8 sprite_num;
	uint32 _pad2;
} image_t;

typedef struct sprite_t
{
	uint8 flip;
	uint16 tile_offs;
	uint8 w;
	uint8 h;
	int8 offs_x;
	int8 offs_y;
} sprite_t;

typedef struct sprite_file_t
{
	uint16 sprite_num;
	uint16 tile_idx;
	uint8 pal_idx;
	const sprite_t* sprites;
} sprite_file_t;

typedef struct stage_chunk_t
{
	char name[16];
} stage_chunk_t;

typedef struct stage_t
{
	uint16 width;
	uint16 height;
	uint32 _pad[3];
} stage_t;

typedef struct game_t
{
	arc_t* arc;
	sprite_file_t sprite_files[FILE_MAX];
	stage_t* stage;
	image_t* stage_img;
	const image_t* cell_anime_img;
	uint8 cell_anime_idx;
	uint8 cell_anime_wait;
	uint8 cell_anime_time[2];
	uint32 input_hold;
	uint32 input_down;
	uint32 rand_seed;
	uint32 frame_num;
	uint8 phase;
	uint8 step;
	uint8 obj_num;
	uint32 stage_idx;
	uint8 fade;
	int8 select_idx;
	struct
	{
		uint32 credit;
		const void* vm_data;
		const void* vm_stream;
		uint8 vm_stream_idx;
		uint8 vm_regs[8];
		vm_obj_t vm_objs[8];
		uint8 cheat_hist[CHEAT_INPUT_MAX];
		uint8 cheat_hist_idx;
		char cheat_name[16];
		uint8 cheat_name_counter;
		uint8 cheat_stage_unlock;
		uint8 cheat_god_mode;
		uint8 cheat_no_clip;
	} title;
	struct 
	{
		uint8 stage_num;
		uint8 marker_pos[STAGE_MAX][2];
		int16 tora_pos[2];
	} select;
	struct
	{
		int8 counter;
		uint8 ez_mode;
		uint8 hp_anim_time;
		uint8 score_anim_time;
		uint8 kills;
		uint16 hp_max;
		uint16 hp;
		uint16 hp_anim;
		uint32 score;
		uint32 score_anim;
		uint16 ether;
		uint16 ether_bonus;
		player_task_t* player;
		item_task_t* weapon;
		item_task_t* loot[LOOT_MAX];
		item_task_t* stage_tbox;
		uint16 star_rate;
		uint16 o2_rate;
		uint16 o2_val;
		uint16 bonus_time;
		uint16 bonus_uni_rate;
		uint8 bonus_ether_rate;
		uint8 bonus_entry_time;
		int16 scroll_threshold;
		int32 scroll_down_spd;
		int32 scroll_up_accel;
		int32 scroll_up_spd;
		int32 scroll_up_spd_max;
		int32 scroll_spd;
		int32 scroll_y;
		int32 scroll_y_max;
		uint16 tbox_num;
		uint16 ether_num;
		int8 scroll_time;
		stage_obj_t stage_objs[STAGE_OBJ_MAX];
	} play;
	struct
	{
		uint16 time;
		uint8 end;
		uint8 saved_step;
		int16 saved_x;
		int16 saved_y;
		uint16 saved_scroll_threshold;
	} bonus;
	struct
	{
		uint8 index;
		uint8 counter;
		uint32 score_anim;
		uint32 ether_anim;
		uint8 badge_anim[5];
		uint8 badge_hp;
		uint8 badge_eth;
		uint8 badge_tbox;
		uint8 badge_perfect;
		uint8 badge_no_kill;
		uint8 badge_crystal;
		uint8 tbox_ids[LOOT_MAX];
		uint8 tbox_sprite_idxs[LOOT_MAX];
	} clear;
	struct
	{
		uint8 counter;
		uint8 char_idx;
		uint8 char_select_idx;
		uint8 high_score;
		char buffer[4];
	} name;
	save_t save;
	gpu_t gpu;
	sound_sys_t sound_sys;
	task_sys_t task_sys;
} game_t;

uint8 game_init(game_t* game, const char* path);
void game_fini(game_t* game);
uint8 game_update(game_t* game, uint32 input);
void game_phase_boot(game_t* game);
void game_phase_title(game_t* game);
void game_phase_select(game_t* game);
void game_phase_play(game_t* game);
void game_phase_clear(game_t* game);
void game_phase_over(game_t* game);
void game_phase_name(game_t* game);
void game_move(game_t* game);
void game_move_player(game_t* game, player_task_t* player);
void game_move_enemy(game_t* game, enemy_task_t* enemy);
void game_move_item(game_t* game, item_task_t* item);
void game_move_effect(game_t* game, effect_task_t* effect);
void game_draw_player(game_t* game, player_task_t* player);
void game_draw_enemy(game_t* game, enemy_task_t* enemy);
void game_draw_item(game_t* game, item_task_t* item);
void game_draw_effect(game_t* game, effect_task_t* effect);
void game_draw_sprite(game_t* game, uint32 file_idx, uint32 sprite_idx, int16 x, int16 y, uint8 flip, uint8 prio);
void game_draw_obj(game_t* game, uint32 file_idx, uint32 sprite_idx, const object_t* obj, int16 offs_x, int16 offs_y, uint8 prio);
void game_fg_draw_tile(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint16 tile_idx, uint8 w, uint8 h);
void game_fg_draw_text(game_t* game, uint8 x, uint8 y, uint8 pal_idx, const char* text);
void game_fg_draw_number(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint32 val, uint8 ez_mode);
void game_fg_clear(game_t* game, uint8 x, uint8 y, uint8 w, uint8 h);
void game_bg_draw_tile(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint16 tile_idx, uint8 w, uint8 h);
void game_bg_draw_text(game_t* game, uint8 x, uint8 y, uint8 pal_idx, const char* text);
void game_bg_draw_number(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint32 val, uint8 ez_mode);
void game_bg_clear(game_t* game, uint8 x, uint8 y, uint8 w, uint8 h);
void game_ui_draw_tile(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint16 tile_idx, uint8 w, uint8 h);
void game_ui_draw_text(game_t* game, uint8 x, uint8 y, uint8 pal_idx, const char* text);
void game_ui_draw_number(game_t* game, uint8 x, uint8 y, uint8 pal_idx, uint32 val, uint8 ez_mode);
void game_ui_draw_sprite(game_t* game, uint8 x, uint8 y, uint8 file_idx, uint16 tile_idx, uint8 w, uint8 h);
void game_ui_clear(game_t* game, uint8 x, uint8 y, uint8 w, uint8 h);
uint8 game_fadein(game_t* game);
uint8 game_fadeout(game_t* game);
void game_fadeset(game_t* game, uint8 val);
uint8 game_gpu_obj_add(game_t* game);
void game_gpu_obj_clear(game_t* game);
void* game_obj_create(game_t* game);
void game_obj_remove(game_t* game, void* obj);
uint32 game_load_sprite_file(game_t* game, uint16 tile_idx, uint8 pal_idx, uint8 file_idx);
uint32 game_load_sprite_img(game_t* game, uint16 tile_idx, const image_t* img);
void game_load_img(game_t* game, uint16 tile_idx, const image_t* img, uint16 offs, uint8 w, uint8 h);
void game_load_img_pal(game_t* game, uint8 pal_idx, const image_t* img);
void game_vm_update(game_t* game);
int8 game_vm_stream_i8(game_t* game);
int8 game_vm_stream_u8(game_t* game);
uint16 game_vm_stream_u16(game_t* game);
void game_cell_anime_init(game_t* game, uint32 idx);
void game_cell_anime(game_t* game);
void game_bgm_play(game_t* game, uint8 idx);
void game_se_play(game_t* game, uint8 idx);
void game_bgm_pitch(game_t* game, float pitch);
void game_get_player_xy(game_t* game, int32* x, int32* y);
void game_get_player_dxdy(game_t* game, int32 x, int32 y, int32* dx, int32* dy);
uint32 game_get_player_dist(game_t* game, int32 x, int32 y);
uint8 game_get_player_vxvy(game_t* game, int32 x, int32 y, int32 spd, int32* vx, int32* vy);
uint8 game_get_vxvy(game_t* game, int32 dx, int32 dy, int32 spd, int32* vx, int32* vy);
effect_task_t* game_air_bubble(game_t* game);
void game_add_hp(game_t* game, int32 val);
void game_add_score(game_t* game, uint32 val);
void game_add_ether(game_t* game, uint32 val);
void game_add_bonus_ether(game_t* game, uint32 val);
void game_add_kill(game_t* game);
uint8 game_loot_item(game_t* game, uint8 id, item_task_t* item);
uint8 game_get_stage_tile(game_t* game, uint32 x, uint32 y);
object_t* game_coli_check(game_t* game, uint8* out_stage_hit_flag);
uint8 game_coli_check_stage(game_t* game);
enemy_task_t* game_atk_check(game_t* game);
void game_atk_dmg(game_t* game, enemy_task_t* enemy, uint8 move_dir);
uint32 game_rand(game_t* game);
int32 game_rand_max(game_t* game, int32 max);
int32 game_rand_minmax(game_t* game, int32 min, int32 max);
rgba_t* game_get_framebuffer(game_t* game);
