/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wake.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Per-coder event channel: the arbiter runs in whichever thread triggered it,
** so it needs a way to hand its decision over to the waiting coder.
*/

void	wake_signal(t_coder *c)
{
	pthread_mutex_lock(&c->wake_lock);
	pthread_cond_signal(&c->wake_cond);
	pthread_mutex_unlock(&c->wake_lock);
}

void	wake_grant(t_coder *c)
{
	pthread_mutex_lock(&c->wake_lock);
	c->granted = 1;
	pthread_cond_signal(&c->wake_cond);
	pthread_mutex_unlock(&c->wake_lock);
}

/*
** Waits for the arbiter's decision. The stop flag is re-read inside the
** critical section, so a simulation end can never be missed here.
*/

int	wake_wait_grant(t_sim *sim, t_coder *c)
{
	int	ok;

	pthread_mutex_lock(&c->wake_lock);
	while (!c->granted && !sim_stopped(sim))
		pthread_cond_wait(&c->wake_cond, &c->wake_lock);
	ok = c->granted;
	c->granted = 0;
	pthread_mutex_unlock(&c->wake_lock);
	return (ok);
}
