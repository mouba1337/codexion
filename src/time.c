/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000L + (long)tv.tv_usec / 1000L);
}

long	elapsed_ms(t_sim *sim)
{
	return (now_ms() - sim->start);
}

void	ms_to_timespec(long ms, struct timespec *ts)
{
	ts->tv_sec = (time_t)(ms / 1000L);
	ts->tv_nsec = (long)((ms % 1000L) * 1000000L);
}

void	sim_sleep(t_sim *sim, long ms)
{
	long	end;

	end = now_ms() + ms;
	while (now_ms() < end)
	{
		if (sim_stopped(sim))
			return ;
		usleep(200);
	}
}
