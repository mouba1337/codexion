/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pqueue.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	pq_init(t_pq *q, int cap, int policy)
{
	q->items = malloc(sizeof(t_req) * cap);
	if (!q->items)
		return (0);
	memset(q->items, 0, sizeof(t_req) * cap);
	q->size = 0;
	q->cap = cap;
	q->policy = policy;
	return (1);
}

void	pq_destroy(t_pq *q)
{
	free(q->items);
	q->items = NULL;
	q->size = 0;
	q->cap = 0;
}

void	pq_swap(t_pq *q, int i, int j)
{
	t_req	tmp;

	tmp = q->items[i];
	q->items[i] = q->items[j];
	q->items[j] = tmp;
}

int	pq_less(t_pq *q, int i, int j)
{
	if (q->policy == EDF)
	{
		if (q->items[i].deadline != q->items[j].deadline)
			return (q->items[i].deadline < q->items[j].deadline);
		return (q->items[i].seq < q->items[j].seq);
	}
	return (q->items[i].seq < q->items[j].seq);
}

void	pq_sift_up(t_pq *q, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!pq_less(q, i, parent))
			break ;
		pq_swap(q, i, parent);
		i = parent;
	}
}
