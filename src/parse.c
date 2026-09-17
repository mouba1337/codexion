/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	str_to_long(char *s, long *out)
{
	long	value;
	int		i;

	value = 0;
	i = 0;
	if (s[i] == '+')
		i++;
	if (!s[i])
		return (0);
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		value = value * 10 + (s[i] - '0');
		if (value > 2147483647)
			return (0);
		i++;
	}
	*out = value;
	return (1);
}

static int	fill_values(t_config *cfg, char **argv)
{
	long	v[7];
	int		i;

	i = 0;
	while (i < 7)
	{
		if (!str_to_long(argv[i + 1], &v[i]))
			return (0);
		i++;
	}
	cfg->coders = (int)v[0];
	cfg->burnout = v[1];
	cfg->compile_t = v[2];
	cfg->debug_t = v[3];
	cfg->refactor_t = v[4];
	cfg->required = (int)v[5];
	cfg->cooldown = v[6];
	return (1);
}

static int	check_ranges(t_config *cfg)
{
	if (cfg->coders < 1 || cfg->coders > MAX_CODERS)
		return (0);
	if (cfg->burnout < 1)
		return (0);
	return (1);
}

static int	parse_policy(t_config *cfg, char *s)
{
	if (!strcmp(s, "fifo"))
		cfg->policy = FIFO;
	else if (!strcmp(s, "edf"))
		cfg->policy = EDF;
	else
		return (0);
	return (1);
}

int	parse_args(t_config *cfg, int argc, char **argv)
{
	if (argc != 9)
		return (0);
	if (!fill_values(cfg, argv))
		return (0);
	if (!check_ranges(cfg))
		return (0);
	if (!parse_policy(cfg, argv[8]))
		return (0);
	return (1);
}
