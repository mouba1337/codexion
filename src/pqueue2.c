/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pqueue2.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	pq_sift_down(t_pq *q, int i)
{
	int	child;

	child = 2 * i + 1;
	while (child < q->size)
	{
		if (child + 1 < q->size && pq_less(q, child + 1, child))
			child++;
		if (!pq_less(q, child, i))
			break ;
		pq_swap(q, i, child);
		i = child;
		child = 2 * i + 1;
	}
}

void	pq_push(t_pq *q, t_req req)
{
	if (q->size >= q->cap)
		return ;
	q->items[q->size] = req;
	q->size++;
	pq_sift_up(q, q->size - 1);
}

void	pq_pop(t_pq *q)
{
	if (q->size == 0)
		return ;
	pq_swap(q, 0, q->size - 1);
	q->size--;
	pq_sift_down(q, 0);
}

void	pq_remove(t_pq *q, int coder)
{
	int	i;

	i = 0;
	while (i < q->size && q->items[i].coder != coder)
		i++;
	if (i == q->size)
		return ;
	pq_swap(q, i, q->size - 1);
	q->size--;
	if (i < q->size)
	{
		pq_sift_up(q, i);
		pq_sift_down(q, i);
	}
}
