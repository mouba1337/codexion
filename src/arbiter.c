/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arbiter.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** it is skipped when already collected.
*/

static void	collect(t_sim *sim)
{
	t_req	*r;
	int		i;
	int		j;

	sim->scratch.size = 0;
	i = 0;
	while (i < sim->cfg.coders)
	{
		pthread_mutex_lock(&sim->dongles[i].lock);
		j = 0;
		while (j < sim->dongles[i].queue.size)
		{
			r = &sim->dongles[i].queue.items[j];
			if (pq_index(&sim->scratch, r->coder) < 0)
				pq_push(&sim->scratch, *r);
			j++;
		}
		pthread_mutex_unlock(&sim->dongles[i].lock);
		i++;
	}
}

/*
** All or nothing: a coder either receives both of its dongles in this very
** critical section, or it receives none and keeps waiting.
*/

static int	try_serve(t_sim *sim, t_coder *c, long now)
{
	if (sim->blocked[c->first] || sim->blocked[c->second])
		return (0);
	if (!dongle_is_free(sim, c->first, now))
		return (0);
	if (c->second != c->first && !dongle_is_free(sim, c->second, now))
		return (0);
	dongle_hold(sim, c->first, c->id);
	if (c->second != c->first)
		dongle_hold(sim, c->second, c->id);
	wake_grant(c);
	return (1);
}

/*
** A blocked request closes its two dongles for every lower priority request
** when it has spent its bypass budget, or when letting somebody else compile
*/

static int	must_block(t_sim *sim, t_req *req, long now)
{
	if (req->bypass >= BYPASS_LIMIT)
		return (1);
	if (now + sim->cfg.compile_t + sim->cfg.cooldown > req->deadline)
		return (1);
	return (0);
}

/*
** One arbitration pass, run with the arbiter mutex 
held each one granted or blocked.
*/

void	arbitrate_pass(t_sim *sim)
{
	t_coder	*c;
	t_req	req;
	long	now;

	now = now_ms();
	collect(sim);
	memset(sim->blocked, 0, sizeof(int) * sim->cfg.coders);
	while (sim->scratch.size > 0)
	{
		req = sim->scratch.items[0];
		pq_pop(&sim->scratch);
		c = &sim->coders[req.coder - 1];
		if (!try_serve(sim, c, now) && must_block(sim, &req, now))
		{
			sim->blocked[c->first] = 1;
			sim->blocked[c->second] = 1;
		}
	}
}

void	arbitrate(t_sim *sim)
{
	pthread_mutex_lock(&sim->arbiter);
	arbitrate_pass(sim);
	pthread_mutex_unlock(&sim->arbiter);
}
