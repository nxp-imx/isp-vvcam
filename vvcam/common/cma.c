/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/
#include "cma.h"

#ifdef __KERNEL__

struct block_list {
	struct block_list *next;
	u64 base_addr;
	u64 size;
};

struct cma_mem_context {
	u64 base;
	u64 size;
	u64 align;
	struct block_list free_blocks;
	struct block_list used_blocks;
};

static struct cma_mem_context *g_cma_mem_ctx;
static inline void add_free_blocks(struct block_list *free_item);
static inline void add_used_blocks(struct block_list *used_item);
struct mutex viv_cma_mutex;

int cma_init(u64 base, u64 size, u64 align)
{
	struct block_list *item = NULL;

	pr_info("enter %s\n", __func__);
	if (!size || (size <= align))
		return -EINVAL;
	g_cma_mem_ctx = kzalloc(sizeof(struct cma_mem_context), GFP_KERNEL);
	memset(g_cma_mem_ctx, 0, sizeof(*g_cma_mem_ctx));

	g_cma_mem_ctx->base = (base + align - 1) & ~(align - 1);
	g_cma_mem_ctx->size = size - (g_cma_mem_ctx->base - base);
	g_cma_mem_ctx->align = align;

	pr_info("addr:0x%llx, size:0x%llx, alignment:0x%llx.\n",
		g_cma_mem_ctx->base, g_cma_mem_ctx->size, g_cma_mem_ctx->align);

	item = kzalloc(sizeof(struct block_list), GFP_KERNEL);
	if (!item) {
		kzfree(g_cma_mem_ctx);
		g_cma_mem_ctx = NULL;
		return -ENOMEM;
	}

	item->next = NULL;
	item->base_addr = g_cma_mem_ctx->base;
	item->size = g_cma_mem_ctx->size;

	g_cma_mem_ctx->free_blocks.next = item;
	g_cma_mem_ctx->used_blocks.next = NULL;

	mutex_init(&viv_cma_mutex);
	return 0;
}

int cma_release(void)
{
	int result = 0;
	struct block_list *item, *pFree;

	if (!g_cma_mem_ctx->free_blocks.next
	    || g_cma_mem_ctx->free_blocks.next->next
	    || g_cma_mem_ctx->used_blocks.next) {
		pr_err("Warning memory is not free.\n");
	}

	item = g_cma_mem_ctx->free_blocks.next;
	while (item) {
		pFree = item;
		item = item->next;
		/*      kzfree(pFree); */
	}
	kzfree(g_cma_mem_ctx);
	g_cma_mem_ctx = NULL;

	mutex_destroy(&viv_cma_mutex);
	return result;
}

u64 cma_alloc(u64 size)
{
	u64 addr = ~0U;
	struct block_list *item;
	struct block_list *found;
	mutex_lock(&viv_cma_mutex);
	if (!size || (size > g_cma_mem_ctx->size)) {
		mutex_unlock(&viv_cma_mutex);
		return ~0U;
	}

	pr_info("enter %s\n", __func__);
	pr_info("addr:0x%llx, size:0x%llx, alignment:0x%llx, reqsize:0x%llx.\n",
		g_cma_mem_ctx->base, g_cma_mem_ctx->size,
		g_cma_mem_ctx->align, size);

	size += g_cma_mem_ctx->align - 1;
	size &= ~(g_cma_mem_ctx->align - 1);
	/*TODO: need to lock this block */
	item = &g_cma_mem_ctx->free_blocks;
	while (item->next && (item->next->size < size))
		item = item->next;

	found = item->next;
	if (found) {
		item->next = found->next;
		if ((found->size - size) >= g_cma_mem_ctx->align) {
			item = kzalloc(sizeof(struct block_list), GFP_KERNEL);
			if (item) {
				item->base_addr = found->base_addr + size;
				item->size = found->size - size;
				found->size = size;
				pr_info("new free block: base_addr=0x%llx,\n",
					item->base_addr);
				add_free_blocks(item);
			}
		}

		pr_info("new used block: base_addr=0x%llx, size=0x%llx \n",
			found->base_addr, found->size);
		add_used_blocks(found);
		addr = found->base_addr;
	}

	pr_info("block allocated: base_addr=0x%llx\n", addr);
	mutex_unlock(&viv_cma_mutex);
	return addr;
}

void cma_free(u64 addr)
{
	pr_info("enter %s\n", __func__);
	pr_info("block to free: base_addr=0x%llx\n", addr);
	mutex_lock(&viv_cma_mutex);
	if (addr) {
		/*TODO: need to lock this block */
		struct block_list *item, *free_item;

		item = &g_cma_mem_ctx->used_blocks;
		while (item->next && (item->next->base_addr != addr))
			item = item->next;

		free_item = item->next;
		item->next = free_item->next;
		if (((item->base_addr + item->size) != free_item->base_addr) ||
		    ((free_item->base_addr + item->size) != ((free_item->next) ?
							     free_item->next->
							     base_addr
							     : (g_cma_mem_ctx->
								base +
								g_cma_mem_ctx->
								size)))) {
			struct block_list *loc_item;
			struct block_list *pre_item = NULL;
			struct block_list *success_item = NULL;
			loc_item = &g_cma_mem_ctx->free_blocks;
			while (loc_item->next) {
				if ((loc_item->next->base_addr +
				     loc_item->next->size) ==
				    free_item->base_addr) {
					pre_item = loc_item;
				}
				if ((free_item->base_addr + free_item->size) ==
				    loc_item->next->base_addr) {
					success_item = loc_item;
				}
				loc_item = loc_item->next;
			}

			if (success_item) {
				loc_item = success_item->next;
				free_item->size += loc_item->size;
				success_item->next = loc_item->next;
				kzfree(loc_item);
			}

			if (pre_item) {
				loc_item = pre_item->next;
				free_item->base_addr = loc_item->base_addr;
				free_item->size += loc_item->size;
				pre_item->next = loc_item->next;
				kzfree(loc_item);
			}
		} else {
			pr_info("no adjacent block free\n");
		}
		add_free_blocks(free_item);
	}
	mutex_unlock(&viv_cma_mutex);
}

static inline void add_free_blocks(struct block_list *free_item)
{
	struct block_list *item;

	item = &g_cma_mem_ctx->free_blocks;
	while (item->next && (item->next->size < free_item->size))
		item = item->next;

	free_item->next = item->next;
	item->next = free_item;

	item = &g_cma_mem_ctx->free_blocks;
	while (item->next)
		item = item->next;
}

static inline void add_used_blocks(struct block_list *used_item)
{
	struct block_list *item;

	item = &g_cma_mem_ctx->used_blocks;
	while (item->next && (item->next->base_addr < used_item->base_addr))
		item = item->next;

	used_item->next = item->next;
	item->next = used_item;
	item = &g_cma_mem_ctx->used_blocks;
	while (item->next)
		item = item->next;
}

#endif
