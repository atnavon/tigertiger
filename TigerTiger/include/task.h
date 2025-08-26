#pragma once

#include "common.h"

typedef struct task_t
{
	struct task_t* next;
	struct task_t* prev;

	void(*move_cb)(void*, void*);
	void(*draw_cb)(void*, void*);
} task_t;

typedef struct
{
	task_t* active_head;
	task_t* active_tail;
	task_t* free_head;
	task_t* free_tail;
	uint32 task_size;
	uint32 task_count;
	task_t* exec_task;
	void* arena;
} task_sys_t;

void task_init(task_sys_t* sys, uint32 size, uint32 count);
void task_fini(task_sys_t* sys);
void task_clear(task_sys_t* sys);
void task_move(task_sys_t* sys, void* ctx);
void task_draw(task_sys_t* sys, void* ctx);
task_t* task_create(task_sys_t* sys);
task_t* task_enum(task_sys_t* sys, task_t* task);
void task_remove(task_sys_t* sys, task_t* task);
