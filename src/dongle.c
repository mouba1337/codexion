/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** One request per compile attempt: the same sequence number and the same
** deadline are stored in both dongle queues, so the scheduler order is total
** and the arbiter can never see two contradictory priorities.
*/

void	dongle_request(t_sim *sim, t_coder *c, t_req *req)
{
	pthread_mutex_lock(&sim->state_lock);
	req->coder = c->id;
	req->bypass = 0;
	req->seq = sim->seq;
	sim->seq++;
	req->deadline = c->last_start + sim->cfg.burnout;
	pthread_mutex_unlock(&sim->state_lock);
}

void	dongle_enqueue(t_sim *sim, t_req *req, int index)
{
	t_dongle	*d;

	d = &sim->dongles[index];
	pthread_mutex_lock(&d->lock);
	pq_push(&d->queue, *req);
	pthread_mutex_unlock(&d->lock);
}

void	dongle_dequeue(t_sim *sim, int id, int index)
{
	t_dongle	*d;

	d = &sim->dongles[index];
	pthread_mutex_lock(&d->lock);
	pq_remove(&d->queue, id);
	pthread_mutex_unlock(&d->lock);
}

int	dongle_is_free(t_sim *sim, int index, long now)
{
	t_dongle	*d;
	int			ready;

	d = &sim->dongles[index];
	pthread_mutex_lock(&d->lock);
	ready = (d->holder == 0 && now >= d->free_at);
	pthread_mutex_unlock(&d->lock);
	return (ready);
}
