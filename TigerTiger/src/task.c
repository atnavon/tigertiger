#include "task.h"

static void llist_add(task_t** head, task_t** tail, task_t* task)
{
	task->prev = *tail;
	task->next = NULL;

	if (*tail)
	{
		(*tail)->next = task;
	}
	else
	{
		*head = task;
	}

	*tail = task;
}

static void llist_remove(task_t** head, task_t** tail, task_t* task)
{
	if (task == *head)
	{
		*head = (*head)->next;
	}
	else
	{
		task->prev->next = task->next;
	}

	if (task == *tail)
	{
		*tail = (*tail)->prev;
	}
	else
	{
		task->next->prev = task->prev;
	}

	task->prev = NULL;
	task->next = NULL;
}

void task_init(task_sys_t* sys, uint32 size, uint32 count)
{
	memset(sys, 0, sizeof(task_sys_t));

	sys->arena = calloc(count, size);
	sys->task_size = size;
	sys->task_count = count;

	task_clear(sys);
}

void task_fini(task_sys_t* sys)
{
	free(sys->arena);

	memset(sys, 0, sizeof(task_sys_t));
}

void task_clear(task_sys_t* sys)
{
	sys->active_head = NULL;
	sys->active_tail = NULL;
	sys->free_head = NULL;
	sys->free_tail = NULL;

	for (uint32 i = 0; i < sys->task_count; ++i)
	{
		task_t* task = MEMOFFS(sys->arena, i * sys->task_size);

		llist_add(&sys->free_head, &sys->free_tail, task);
	}
}

void task_move(task_sys_t* sys, void* ctx)
{
	task_t* task = sys->active_head;

	while (task)
	{
		sys->exec_task = task->next;

		task->move_cb(ctx, task);

		task = sys->exec_task;
	}
}

void task_draw(task_sys_t* sys, void* ctx)
{
	task_t* task = sys->active_head;

	while (task)
	{
		sys->exec_task = task->next;

		task->draw_cb(ctx, task);

		task = sys->exec_task;
	}
}

task_t* task_create(task_sys_t* sys)
{
	task_t* task = sys->free_head;

	if (task)
	{
		llist_remove(&sys->free_head, &sys->free_tail, task);
		llist_add(&sys->active_head, &sys->active_tail, task);
	}

	return task;
}

task_t* task_enum(task_sys_t* sys, task_t* task)
{
	if (!task)
	{
		return sys->active_head;
	}
	else
	{
		return task->next;
	}
}

void task_remove(task_sys_t* sys, task_t* task)
{
	if (sys->exec_task == task)
	{
		sys->exec_task = task->next;
	}

	llist_remove(&sys->active_head, &sys->active_tail, task);
	llist_add(&sys->free_head, &sys->free_tail, task);

	task->move_cb = NULL;
	task->draw_cb = NULL;
}
