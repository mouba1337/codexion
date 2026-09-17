/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	sim_stopped(t_sim *sim)
{
	int	stopped;

	pthread_mutex_lock(&sim->state_lock);
	stopped = sim->stopped;
	pthread_mutex_unlock(&sim->state_lock);
	return (stopped);
}

void	sim_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->state_lock);
	sim->stopped = 1;
	pthread_mutex_unlock(&sim->state_lock);
}

void	log_state(t_sim *sim, int id, char *msg)
{
	long	stamp;

	pthread_mutex_lock(&sim->log_lock);
	pthread_mutex_lock(&sim->state_lock);
	stamp = elapsed_ms(sim);
	if (!sim->stopped)
		printf("%ld %d %s\n", stamp, id, msg);
	pthread_mutex_unlock(&sim->state_lock);
	pthread_mutex_unlock(&sim->log_lock);
}

void	log_burnout(t_sim *sim, int id, long stamp)
{
	pthread_mutex_lock(&sim->log_lock);
	printf("%ld %d %s\n", stamp, id, MSG_BURNED);
	pthread_mutex_unlock(&sim->log_lock);
}
