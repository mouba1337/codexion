/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle2.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Hands the dongle to a coder: every pending request it overtakes is charged
** one bypass, then its own request leaves the queue.
*/

void	dongle_hold(t_sim *sim, int index, int id)
{
	t_dongle	*d;

	d = &sim->dongles[index];
	pthread_mutex_lock(&d->lock);
	pq_mark_bypassed(&d->queue, id);
	pq_remove(&d->queue, id);
	d->holder = id;
	pthread_mutex_unlock(&d->lock);
}

/*
** Puts the dongle back on the table: it stays unavailable until its cooldown
** has elapsed.
*/

void	dongle_release(t_sim *sim, int index)
{
	t_dongle	*d;

	d = &sim->dongles[index];
	pthread_mutex_lock(&d->lock);
	d->holder = 0;
	d->free_at = now_ms() + sim->cfg.cooldown;
	pthread_mutex_unlock(&d->lock);
}
