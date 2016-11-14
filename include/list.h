#ifndef _LIST_H_
  #define _LIST_H_

#include "stddef.h"

#define LIST_POISON1 ((void *)0x00)
#define LIST_POISON2 ((void *)0x00)

/*******************************************************************************
* 此头文件实现简单的双向链表
********************************************************************************/
/*******************************************************************************
* 用于预取以提高遍历速度
* prefetch告诉CPU哪些元素可能马上就要用到，让CPU预取一下，这样可以提高速度
********************************************************************************/
#ifndef ARCH_HAS_PREFETCH
#define ARCH_HAS_PREFETCH
static inline void prefetch(const void *x) {;}
#endif


/* 链表节点 */
struct list_head
{
	struct list_head *next, *prev;
};


/* 初始化链表名字 */
#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)


/********************************************************************************
* 函数: static inline void INIT_LIST_HEAD(__in struct list_head *list)
* 描述: 初始化新双向链表的链表头，使链表头指向自己
* 输入: list: 指向链表头的指针
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void INIT_LIST_HEAD(__in struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

/********************************************************************************
* 函数: static inline void __list_add(__in struct list_head *new,
                                     __in struct list_head *prev,
                                     __in struct list_head *next)
* 描述: 在指定的位置添加一个新节点，此函数仅供内部使用
* 输入: new: 需要插入的新节点
       prev: 前向节点
       next: 后向节点
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void __list_add(__in struct list_head *new,
                                __in struct list_head *prev,
                                __in struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/********************************************************************************
* 函数: static inline void list_add(__in struct list_head *new,
                                   __in struct list_head *head)
* 描述: 在链表头插入新的节点
* 输入: new: 新插入的节点
       head: 链表头
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_add(__in struct list_head *new, __in struct list_head *head)
{
	__list_add(new, head, head->next);
}

/********************************************************************************
* 函数: static inline void list_add_tail(__in struct list_head *new,
                                        __in struct list_head *head)
* 描述: 在链表尾插入新的节点
* 输入: new: 新插入的节点
       head: 链表头
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_add_tail(__in struct list_head *new, __in struct list_head *head)
{
	__list_add(new, head->prev, head);
}

/********************************************************************************
* 函数: static inline void __list_del(__in struct list_head *prev,
                                     __in struct list_head *next)
* 描述: 删除列表，此函数仅供内部调用
* 输入: prev: 前向节点
       next: 后向节点
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void __list_del(__in struct list_head *prev, __in struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/********************************************************************************
* 函数: static inline void list_del(__in struct list_head *entry)
* 描述: 删除链表节点
* 输入: entry: 指向节点的指针
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_del(__in struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}

/********************************************************************************
* 函数: static inline void list_replace(__in struct list_head *old,
				                         __in struct list_head *new)
* 描述: 替换节点数据
* 输入: old: 需要替换的旧节点
       new: 替换成的新节点
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_replace(__in struct list_head *old,
				                  __in struct list_head *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

/********************************************************************************
* 函数: static inline void list_replace_init(__in struct list_head *old,
					                           __in struct list_head *new)
* 描述: 替换节点数据并把替换下来的节点初始换新的链表头
* 输入: old: 需要替换的旧节点
       new: 替换成的新节点
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_replace_init(__in struct list_head *old,
					                   __in struct list_head *new)
{
	list_replace(old, new);
	INIT_LIST_HEAD(old);
}

/********************************************************************************
* 函数: static inline void list_del_init(__in struct list_head *entry)
* 描述: 删除链表的节点，并把删除的节点初始化为新的链表头
* 输入: entry: 需要删除的节点
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_del_init(__in struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

/********************************************************************************
* 函数: static inline void list_move(__in struct list_head *list,
                                    __in struct list_head *head)
* 描述: 把节点list移动到链表头之后
* 输入: list: 需要移动的节点
       head: 链表头
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_move(__in struct list_head *list,
                               __in struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}

/********************************************************************************
* 函数: static inline void list_move_tail(__in struct list_head *list,
				                           __in struct list_head *head)
* 描述: 移动节点list到链表尾
* 输入: list: 需要移动的节点
       head: 链表头
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_move_tail(__in struct list_head *list,
                                    __in struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

/********************************************************************************
* 函数: static inline int list_is_last(__in const struct list_head *list,
				                        __in const struct list_head *head)
* 描述: 检测节点是否为链表的最后一个节点
* 输入: list: 需要检测的节点
       head: 链表头
* 输出: none
* 返回: 0: 节点是最后一个节点
       非0: 节点不是最后一个节点
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline int list_is_last(__in const struct list_head *list,
				                 __in const struct list_head *head)
{
	return (list->next == head);
}

/********************************************************************************
* 函数: static inline int list_empty(__in const struct list_head *head)
* 描述: 检测链表是否为空
* 输入: head: 链表头
* 输出: none
* 返回: 0: 链表为空
       非0: 链表不为空
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline int list_empty(__in const struct list_head *head)
{
	return (head->next == head);
}

/********************************************************************************
* 函数: static inline int list_empty_careful(__in const struct list_head *head)
* 描述: 检测链表是否为空，多线程安全
* 输入: head: 链表头
* 输出: none
* 返回: 0: 链表为空
       非0: 链表不为空
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline int list_empty_careful(__in const struct list_head *head)
{
	struct list_head *next = head->next;
	return (next == head) && (next == head->prev);
}

/********************************************************************************
* 函数: static inline int list_is_singular(__in const struct list_head *head)
* 描述: 检测链表是否只有一个节点
* 输入: head: 链表头
* 输出: none
* 返回: 0: 链表为空或链表不止一个节点
       非0: 链表只有一个节点
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline int list_is_singular(__in const struct list_head *head)
{
	return !list_empty(head) && (head->next == head->prev);
}



/********************************************************************************
* 函数: static inline void __list_cut_position(__in struct list_head *list,
                                              __in struct list_head *head,
                                              __in struct list_head *entry)
* 描述: 把head为头的链表从entry节点处分成两个链表
       一个链表以list为头包含head到entry节点之间的所有节点
       一个链表以head为头包含剩下的节点
       此函数仅供内部使用
* 输入: list: 一个链表中的某个节点
       head: 链表头
       entry: 分隔的节点
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void __list_cut_position(__in struct list_head *list,
                                         __in struct list_head *head,
                                         __in struct list_head *entry)
{
	struct list_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

/********************************************************************************
* 函数: static inline void list_cut_position(__in struct list_head *list,
		                                     __in struct list_head *head,
		                                     __in struct list_head *entry)
* 描述: 把head为头的链表从entry节点处分成两个链表
       一个链表以list为头包含head到entry节点之间的所有节点
       一个链表以head为头包含剩下的节点
* 输入: list: 一个链表中的某个节点
       head: 链表头
       entry: 分隔的节点
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_cut_position(__in struct list_head *list,
		                               __in struct list_head *head,
		                               __in struct list_head *entry)
{
	if (list_empty(head))
		return;
	if (list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		INIT_LIST_HEAD(list);
	else
		__list_cut_position(list, head, entry);
}


/********************************************************************************
* 函数: static inline void __list_splice(__in const struct list_head *list,
				                          __in struct list_head *prev,
				                          __in struct list_head *next)
* 描述: 把两个链表合成一个链表
       此函数仅供内部使用
* 输入: list: 一个链表，带有n个节点
       prev: 前向节点
       next: 后向节点
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void __list_splice(__in const struct list_head *list,
				                   __in struct list_head *prev,
				                   __in struct list_head *next)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/********************************************************************************
* 函数: static inline void list_splice(__in const struct list_head *list,
				                        __in struct list_head *head)
* 描述: 把一个链表插入到链表头之后
* 输入: list: 一个链表，带有n个节点
       head: 链表头
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_splice(__in const struct list_head *list,
				                 __in struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head, head->next);
}

/********************************************************************************
* 函数: static inline void list_splice_tail(__in struct list_head *list,
				                             __in struct list_head *head)
* 描述: 把一个链表插入到链表尾之后
* 输入: list: 一个链表头，带有n个节点
       head: 链表头
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_splice_tail(__in struct list_head *list,
				                      __in struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head->prev, head);
}

/********************************************************************************
* 函数: static inline void list_splice_init(__in struct list_head *list,
				                             __in struct list_head *head)
* 描述: 把一个链表插入到链表头之后，并重新初始化list为空链表头
* 输入: list: 一个链表中的某个节点
       head: 链表头
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_splice_init(__in struct list_head *list,
				                      __in struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head, head->next);
		INIT_LIST_HEAD(list);
	}
}

/********************************************************************************
* 函数: static inline void list_splice_tail_init(__in struct list_head *list,
					                       __in struct list_head *head)
* 描述: 把一个链表插入到链表尾之后，并重新初始化list为空链表头
* 输入: list: 一个链表中的某个节点
       head: 链表头
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void list_splice_tail_init(__in struct list_head *list,
					                       __in struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head->prev, head);
		INIT_LIST_HEAD(list);
	}
}

/********************************************************************************
* 函数: list_entry(ptr, type, member)
* 描述: 取得链表节点的地址
* 输入: ptr: 节点中数据的地址
       type: 节点的数据类型
       member: 节点中某个变量的名字
* 输出: none
* 返回: 链表节点的地址
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/********************************************************************************
* 函数: list_first_entry(ptr, type, member)
* 描述: 取得链表中第一个节点的地址
* 输入: ptr: 指向链表头的指针
       type: 节点的数据类型
       member: 节点中变量的名字，需要和ptr中变量名字一样
* 输出: none
* 返回: 第一个节点的地址
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/********************************************************************************
* 函数: list_for_each(pos, head)
* 描述: 向后遍历链表中的所有节点，使用prefetch加速
* 输入: pos: 指向节点的指针
       head: 链表头指针
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each(pos, head) \
	for (pos = (head)->next; prefetch(pos->next), pos != (head); \
		pos = pos->next)

/********************************************************************************
* 函数: list_for_each(pos, head)
* 描述: 向后遍历链表中的所有节点，使用prefetch加速
* 输入: pos: 指向节点的指针
       head: 链表头指针
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define __list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/********************************************************************************
* 函数: list_for_each_prev(pos, head)
* 描述: 向前遍历链表中的所有节点，使用prefetch加速
* 输入: pos: 指向节点的指针
       head: 链表头指针
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; prefetch(pos->prev), pos != (head); \
		pos = pos->prev)

/********************************************************************************
* 函数: list_for_each_safe(pos, n, head)
* 描述: 向后遍历链表中的所有节点
* 输入: pos: 指向节点的指针
       n: 缓存下一节点地址的指针
       head: 链表头指针
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/********************************************************************************
* 函数: list_for_each_prev_safe(pos, n, head)
* 描述: 向前遍历链表中的所有节点
* 输入: pos: 指向节点的指针
       n: 缓存下一节点地址的指针
       head: 链表头指针
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     prefetch(pos->prev), pos != (head); \
	     pos = n, n = pos->prev)

/********************************************************************************
* 函数: list_for_each_entry(pos, n, head)
* 描述: 向后遍历链表中的所有节点
* 输入: pos: 指向节点的指针
       head: 链表头指针
       member: 结构体中的数据名称
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     prefetch(pos->member.next), &pos->member != (head);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/********************************************************************************
* 函数: list_for_each_entry(pos, n, head)
* 描述: 向前遍历链表中的所有节点
* 输入: pos: 指向节点的指针
       head: 链表头指针
       member: 结构体中的数据名称
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_entry((head)->prev, typeof(*pos), member);	\
	     prefetch(pos->member.prev), &pos->member != (head);	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/********************************************************************************
* 函数: list_prepare_entry(pos, head, member)
* 描述: 准备链表节点
* 输入: pos: 指向节点的指针
       head: 链表头指针
       member: 结构体中的数据名称
* 输出: none
* 返回: 指向节点的指针或空
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_prepare_entry(pos, head, member) \
	((pos) ? : list_entry(head, typeof(*pos), member))

/********************************************************************************
* 函数: list_for_each_entry_continue(pos, head, member)
* 描述: 向后遍历链表节点
* 输入: pos: 指向节点的指针
       head: 链表头指针
       member: 结构体中的数据名称
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_entry_continue(pos, head, member) 		\
	for (pos = list_entry(pos->member.next, typeof(*pos), member);	\
	     prefetch(pos->member.next), &pos->member != (head);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/********************************************************************************
* 函数: list_for_each_entry_continue(pos, head, member)
* 描述: 向前遍历链表节点
* 输入: pos: 指向节点的指针
       head: 链表头指针
       member: 结构体中的数据名称
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = list_entry(pos->member.prev, typeof(*pos), member);	\
	     prefetch(pos->member.prev), &pos->member != (head);	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/********************************************************************************
* 函数: list_for_each_entry_from(pos, head, member)
* 描述: 从当前位置向后遍历链表节点
* 输入: pos: 指向节点的指针
       head: 链表头指针
       member: 结构体中的数据名称
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_entry_from(pos, head, member)			\
	for (; prefetch(pos->member.next), &pos->member != (head);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/********************************************************************************
* 函数: list_for_each_entry_safe(pos, n, head, member)
* 描述: 向后遍历链表节点
* 输入: pos: 指向节点的指针
       n: 缓存下一节点的指针
       head: 链表头指针
       member: 结构体中的数据名称
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/********************************************************************************
* 函数: list_for_each_entry_safe_continue(pos, n, head, member)
* 描述: 从当前位置向后遍历链表节点
* 输入: pos: 指向节点的指针
       n: 缓存下一节点的指针
       head: 链表头指针
       member: 结构体中的数据名称
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_entry_safe_continue(pos, n, head, member) 		\
	for (pos = list_entry(pos->member.next, typeof(*pos), member),		\
		n = list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/********************************************************************************
* 函数: list_for_each_entry_safe_from(pos, n, head, member)
* 描述: 从当前位置向后遍历链表节点
* 输入: pos: 指向节点的指针
       n: 缓存下一节点的指针
       head: 链表头指针
       member: 结构体中的数据名称
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_entry_safe_from(pos, n, head, member)			\
	for (n = list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/********************************************************************************
* 函数: list_for_each_entry_safe_reverse(pos, n, head, member)
* 描述: 向前遍历链表节点
* 输入: pos: 指向节点的指针
       n: 缓存下一节点的指针
       head: 链表头指针
       member: 结构体中的数据名称
* 输出: none
* 返回: 指向节点的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_entry((head)->prev, typeof(*pos), member),	\
		n = list_entry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.prev, typeof(*n), member))



#endif /* _LIST_H_ */
