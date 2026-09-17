/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	do_compile(t_sim *sim, t_coder *c)
{
	if (!request_dongles(sim, c))
		return (0);
	pthread_mutex_lock(&sim->state_lock);
	c->last_start = now_ms();
	c->compiles++;
	pthread_mutex_unlock(&sim->state_lock);
	log_state(sim, c->id, MSG_COMPILE);
	sim_sleep(sim, sim->cfg.compile_t);
	release_dongles(sim, c);
	return (1);
}

static int	do_phase(t_sim *sim, t_coder *c, char *msg, long ms)
{
	if (sim_stopped(sim))
		return (0);
	log_state(sim, c->id, msg);
	sim_sleep(sim, ms);
	return (!sim_stopped(sim));
}

static void	*lonely_coder(t_sim *sim, t_coder *c)
{
	if (!request_dongles(sim, c))
		return (NULL);
	while (!sim_stopped(sim))
		usleep(200);
	release_dongles(sim, c);
	return (NULL);
}

void	*coder_routine(void *arg)
{
	t_coder	*c;
	t_sim	*sim;

	c = (t_coder *)arg;
	sim = c->sim;
	if (sim->cfg.coders == 1)
		return (lonely_coder(sim, c));
	while (!sim_stopped(sim))
	{
		if (!do_compile(sim, c))
			break ;
		if (!do_phase(sim, c, MSG_DEBUG, sim->cfg.debug_t))
			break ;
		if (!do_phase(sim, c, MSG_REFACTOR, sim->cfg.refactor_t))
			break ;
	}
	return (NULL);
}
