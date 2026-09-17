/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Both queue insertions and the arbitration that follows them happen in the
** same arbiter section, so no pass can ever see a half registered request.
*/

void	request_enqueue(t_sim *sim, t_coder *c, t_req *req)
{
	pthread_mutex_lock(&sim->arbiter);
	dongle_enqueue(sim, req, c->first);
	if (c->second != c->first)
		dongle_enqueue(sim, req, c->second);
	arbitrate_pass(sim);
	pthread_mutex_unlock(&sim->arbiter);
}

void	request_cancel(t_sim *sim, t_coder *c)
{
	pthread_mutex_lock(&sim->arbiter);
	dongle_dequeue(sim, c->id, c->first);
	if (c->second != c->first)
		dongle_dequeue(sim, c->id, c->second);
	pthread_mutex_unlock(&sim->arbiter);
}

/*
** The coder never holds a dongle while waiting for another one: the arbiter
** hands over the whole pair or nothing at all.
*/

int	request_dongles(t_sim *sim, t_coder *c)
{
	t_req	req;

	dongle_request(sim, c, &req);
	request_enqueue(sim, c, &req);
	if (wake_wait_grant(sim, c))
	{
		log_state(sim, c->id, MSG_TAKEN);
		if (c->second != c->first)
			log_state(sim, c->id, MSG_TAKEN);
		return (1);
	}
	request_cancel(sim, c);
	return (0);
}

/*
** Releasing and re-arbitrating in one section hands the dongles over to the
** next coder without any gap.
*/

void	release_dongles(t_sim *sim, t_coder *c)
{
	pthread_mutex_lock(&sim->arbiter);
	dongle_release(sim, c->first);
	if (c->second != c->first)
		dongle_release(sim, c->second);
	arbitrate_pass(sim);
	pthread_mutex_unlock(&sim->arbiter);
}
