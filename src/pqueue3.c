/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pqueue3.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	pq_index(t_pq *q, int coder)
{
	int	i;

	i = 0;
	while (i < q->size)
	{
		if (q->items[i].coder == coder)
			return (i);
		i++;
	}
	return (-1);
}

/*
** Charges one bypass to every request this coder overtakes on that dongle.
*/

void	pq_mark_bypassed(t_pq *q, int coder)
{
	int	self;
	int	i;

	self = pq_index(q, coder);
	if (self < 0)
		return ;
	i = 0;
	while (i < q->size)
	{
		if (i != self && pq_less(q, i, self))
			q->items[i].bypass++;
		i++;
	}
}
