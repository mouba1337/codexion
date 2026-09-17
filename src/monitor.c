/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_burnout(t_sim *sim)
{
	int		i;
	int		victim;
	long	now;

	victim = 0;
	pthread_mutex_lock(&sim->state_lock);
	now = now_ms();
	i = 0;
	while (i < sim->cfg.coders && !sim->stopped)
	{
		if (now - sim->coders[i].last_start > sim->cfg.burnout)
		{
			sim->stopped = 1;
			victim = sim->coders[i].id;
		}
		i++;
	}
	pthread_mutex_unlock(&sim->state_lock);
	if (victim != 0)
		log_burnout(sim, victim, now - sim->start);
	return (victim != 0);
}

static int	check_done(t_sim *sim)
{
	int	i;
	int	done;

	done = 1;
	i = 0;
	pthread_mutex_lock(&sim->state_lock);
	while (i < sim->cfg.coders)
	{
		if (sim->coders[i].compiles < sim->cfg.required)
			done = 0;
		i++;
	}
	if (done)
		sim->stopped = 1;
	pthread_mutex_unlock(&sim->state_lock);
	return (done);
}

void	wake_all(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.coders)
	{
		wake_signal(&sim->coders[i]);
		i++;
	}
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (1)
	{
		if (check_burnout(sim))
			break ;
		if (check_done(sim))
			break ;
		arbitrate(sim);
		usleep(300);
	}
	wake_all(sim);
	return (NULL);
}
