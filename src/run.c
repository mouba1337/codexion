/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	spawn_coders(t_sim *sim, int *count)
{
	int	i;

	i = 0;
	while (i < sim->cfg.coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
			return (0);
		i++;
		*count = i;
	}
	return (1);
}

static void	join_coders(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

int	sim_run(t_sim *sim)
{
	int	count;
	int	ok;

	count = 0;
	ok = spawn_coders(sim, &count);
	if (ok)
		ok = (pthread_create(&sim->monitor, NULL, monitor_routine, sim) == 0);
	if (!ok)
	{
		sim_stop(sim);
		wake_all(sim);
		join_coders(sim, count);
		return (0);
	}
	pthread_join(sim->monitor, NULL);
	join_coders(sim, count);
	return (1);
}
