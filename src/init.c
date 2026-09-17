/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->cfg.coders);
	if (!sim->dongles)
		return (0);
	memset(sim->dongles, 0, sizeof(t_dongle) * sim->cfg.coders);
	i = 0;
	while (i < sim->cfg.coders)
	{
		if (!pq_init(&sim->dongles[i].queue, sim->cfg.coders + 1,
				sim->cfg.policy))
			return (0);
		pthread_mutex_init(&sim->dongles[i].lock, NULL);
		sim->dongles[i].holder = 0;
		sim->dongles[i].free_at = sim->start;
		i++;
	}
	return (1);
}

/*
** Coder i sits between dongle i (left) and dongle (i + 1) % n (right); first
** and second are the same pair sorted by index, used as the lock order.
*/

static void	init_one_coder(t_sim *sim, int i)
{
	t_coder	*c;

	c = &sim->coders[i];
	c->id = i + 1;
	c->first = i;
	c->second = (i + 1) % sim->cfg.coders;
	if (c->second < c->first)
	{
		c->first = c->second;
		c->second = i;
	}
	c->compiles = 0;
	c->granted = 0;
	c->last_start = sim->start;
	c->sim = sim;
	pthread_mutex_init(&c->wake_lock, NULL);
	pthread_cond_init(&c->wake_cond, NULL);
}

int	sim_init(t_sim *sim, t_config *cfg)
{
	int	i;

	memset(sim, 0, sizeof(t_sim));
	sim->cfg = *cfg;
	sim->start = now_ms();
	pthread_mutex_init(&sim->arbiter, NULL);
	pthread_mutex_init(&sim->log_lock, NULL);
	pthread_mutex_init(&sim->state_lock, NULL);
	sim->coders = malloc(sizeof(t_coder) * cfg->coders);
	if (!sim->coders)
		return (0);
	memset(sim->coders, 0, sizeof(t_coder) * cfg->coders);
	i = 0;
	while (i < cfg->coders)
	{
		init_one_coder(sim, i);
		i++;
	}
	if (cfg->required == 0)
		sim->stopped = 1;
	sim->blocked = malloc(sizeof(int) * cfg->coders);
	if (!sim->blocked || !pq_init(&sim->scratch, cfg->coders + 1, cfg->policy))
		return (0);
	return (init_dongles(sim));
}

static void	destroy_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (sim->dongles && i < sim->cfg.coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].lock);
		pq_destroy(&sim->dongles[i].queue);
		i++;
	}
	i = 0;
	while (sim->coders && i < sim->cfg.coders)
	{
		pthread_mutex_destroy(&sim->coders[i].wake_lock);
		pthread_cond_destroy(&sim->coders[i].wake_cond);
		i++;
	}
}

void	sim_destroy(t_sim *sim)
{
	destroy_dongles(sim);
	pq_destroy(&sim->scratch);
	free(sim->blocked);
	free(sim->dongles);
	free(sim->coders);
	sim->blocked = NULL;
	sim->dongles = NULL;
	sim->coders = NULL;
	pthread_mutex_destroy(&sim->arbiter);
	pthread_mutex_destroy(&sim->log_lock);
	pthread_mutex_destroy(&sim->state_lock);
}
