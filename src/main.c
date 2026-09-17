/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by mhend             #+#    #+#             */
/*   Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	usage(int argc)
{
	if (argc != 9)
		fprintf(stderr, "Error: expected 8 arguments, got %d\n", argc - 1);
	else
		fprintf(stderr, "Error: invalid argument value\n");
	fprintf(stderr, "Usage: ./codexion number_of_coders time_to_burnout ");
	fprintf(stderr, "time_to_compile time_to_debug time_to_refactor ");
	fprintf(stderr, "number_of_compiles_required dongle_cooldown scheduler\n");
	fprintf(stderr, "  scheduler must be exactly \"fifo\" or \"edf\"\n");
	fprintf(stderr, "  number_of_coders and time_to_burnout must be >= 1\n");
	fprintf(stderr, "  every other numeric argument must be >= 0\n");
}

int	main(int argc, char **argv)
{
	t_config	cfg;
	t_sim		sim;

	memset(&cfg, 0, sizeof(t_config));
	if (!parse_args(&cfg, argc, argv))
	{
		usage(argc);
		return (1);
	}
	if (!sim_init(&sim, &cfg))
	{
		fprintf(stderr, "Error: initialisation failed\n");
		sim_destroy(&sim);
		return (1);
	}
	if (!sim_run(&sim))
		fprintf(stderr, "Error: could not start the threads\n");
	sim_destroy(&sim);
	return (0);
}
