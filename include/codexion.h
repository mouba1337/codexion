/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <time.h>
# include <unistd.h>

# define FIFO 0
# define EDF 1

# define MSG_TAKEN "has taken a dongle"
# define MSG_COMPILE "is compiling"
# define MSG_DEBUG "is debugging"
# define MSG_REFACTOR "is refactoring"
# define MSG_BURNED "burned out"

# define MAX_CODERS 500
# define BYPASS_LIMIT 1

typedef struct s_config
{
	int		coders;
	long	burnout;
	long	compile_t;
	long	debug_t;
	long	refactor_t;
	int		required;
	long	cooldown;
	int		policy;
}	t_config;

typedef struct s_req
{
	int		coder;
	int		bypass;
	long	seq;
	long	deadline;
}	t_req;

typedef struct s_pq
{
	t_req	*items;
	int		size;
	int		cap;
	int		policy;
}	t_pq;

typedef struct s_dongle
{
	pthread_mutex_t	lock;
	int				holder;
	long			free_at;
	t_pq			queue;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	int				first;
	int				second;
	int				compiles;
	int				granted;
	long			last_start;
	pthread_t		thread;
	pthread_mutex_t	wake_lock;
	pthread_cond_t	wake_cond;
	struct s_sim	*sim;
}	t_coder;

typedef struct s_sim
{
	t_config		cfg;
	long			start;
	int				stopped;
	long			seq;
	t_dongle		*dongles;
	t_coder			*coders;
	int				*blocked;
	t_pq			scratch;
	pthread_t		monitor;
	pthread_mutex_t	arbiter;
	pthread_mutex_t	log_lock;
	pthread_mutex_t	state_lock;
}	t_sim;

int		parse_args(t_config *cfg, int argc, char **argv);

long	now_ms(void);
long	elapsed_ms(t_sim *sim);
void	ms_to_timespec(long ms, struct timespec *ts);
void	sim_sleep(t_sim *sim, long ms);

int		sim_stopped(t_sim *sim);
void	sim_stop(t_sim *sim);
void	log_state(t_sim *sim, int id, char *msg);
void	log_burnout(t_sim *sim, int id, long stamp);

int		pq_init(t_pq *q, int cap, int policy);
void	pq_destroy(t_pq *q);
void	pq_swap(t_pq *q, int i, int j);
int		pq_less(t_pq *q, int i, int j);
void	pq_sift_up(t_pq *q, int i);
void	pq_sift_down(t_pq *q, int i);
void	pq_push(t_pq *q, t_req req);
void	pq_pop(t_pq *q);
void	pq_remove(t_pq *q, int coder);
int		pq_index(t_pq *q, int coder);
void	pq_mark_bypassed(t_pq *q, int coder);

void	wake_signal(t_coder *c);
void	wake_grant(t_coder *c);
int		wake_wait_grant(t_sim *sim, t_coder *c);

void	dongle_request(t_sim *sim, t_coder *c, t_req *req);
void	dongle_enqueue(t_sim *sim, t_req *req, int index);
void	dongle_dequeue(t_sim *sim, int id, int index);
int		dongle_is_free(t_sim *sim, int index, long now);
void	dongle_hold(t_sim *sim, int index, int id);
void	dongle_release(t_sim *sim, int index);

void	arbitrate_pass(t_sim *sim);
void	arbitrate(t_sim *sim);

void	request_enqueue(t_sim *sim, t_coder *c, t_req *req);
void	request_cancel(t_sim *sim, t_coder *c);
int		request_dongles(t_sim *sim, t_coder *c);
void	release_dongles(t_sim *sim, t_coder *c);

void	*coder_routine(void *arg);

void	wake_all(t_sim *sim);
void	*monitor_routine(void *arg);

int		sim_init(t_sim *sim, t_config *cfg);
void	sim_destroy(t_sim *sim);
int		sim_run(t_sim *sim);

#endif
