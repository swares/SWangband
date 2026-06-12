/**
 * \file game-difficulty.c
 * \brief Difficulty preset system
 *
 * Three named presets that bundle Second Wind, Danger Telemetry, and birth
 * options into one legible choice at the start of character creation.
 *
 * Presets only *preset* the toggles — the full options screen is untouched,
 * so the player can still customise everything after choosing.
 */

#include "angband.h"
#include "game-difficulty.h"
#include "option.h"
#include "player.h"
#include "player-history.h"

int difficulty_preset = DIFFICULTY_ADVENTURER;
int preset_second_wind_charges = 1;

/* --------------------------------------------------------------------------
 * Preset metadata
 * -------------------------------------------------------------------------- */

typedef struct {
	const char *name;
	const char *desc;
	int  second_wind_charges;  /* charges granted at birth */

	/* Option overrides (all booleans).  Only options that differ from the
	 * game default need to be listed — everything else is left alone. */
	bool birth_second_wind;
	bool show_threat_meter;
	bool birth_connect_stairs;
	bool birth_start_kit;
	bool birth_no_selling;
	bool birth_ai_learn;
} preset_def;

static const preset_def PRESETS[DIFFICULTY_COUNT] = {
	/* Wanderer — first-timer safety net */
	[DIFFICULTY_WANDERER] = {
		.name               = "Wanderer",
		.desc               = "Learn the dungeon without losing the run to one mistake. "
		                      "3 second chances, threat meter, generous gold.",
		.second_wind_charges = 3,
		.birth_second_wind   = true,
		.show_threat_meter   = true,
		.birth_connect_stairs = true,
		.birth_start_kit     = true,
		.birth_no_selling    = true,
		.birth_ai_learn      = false,
	},
	/* Adventurer — recommended default */
	[DIFFICULTY_ADVENTURER] = {
		.name               = "Adventurer",
		.desc               = "The intended challenge — softened by a single second chance "
		                      "and a danger readout.",
		.second_wind_charges = 1,
		.birth_second_wind   = true,
		.show_threat_meter   = true,
		.birth_connect_stairs = true,
		.birth_start_kit     = true,
		.birth_no_selling    = false,  /* standard gold */
		.birth_ai_learn      = false,
	},
	/* Legend — old-school permadeath */
	[DIFFICULTY_LEGEND] = {
		.name               = "Legend",
		.desc               = "Vanilla permadeath. No safety net, no warnings. One life.",
		.second_wind_charges = 0,
		.birth_second_wind   = false,
		.show_threat_meter   = false,
		.birth_connect_stairs = true,  /* leave at default */
		.birth_start_kit     = true,   /* leave at default */
		.birth_no_selling    = false,
		.birth_ai_learn      = false,
	},
};

const char *difficulty_preset_name(int id)
{
	if (id < 0 || id >= DIFFICULTY_COUNT) return "Custom";
	return PRESETS[id].name;
}

const char *difficulty_preset_desc(int id)
{
	if (id < 0 || id >= DIFFICULTY_COUNT) return "";
	return PRESETS[id].desc;
}

/**
 * Apply a difficulty preset to the player's option block.
 *
 * Called at birth before the options screen so the player can still tweak
 * individual settings afterwards.  Also records the choice in history.
 */
void apply_difficulty_preset(struct player *p, int id)
{
	const preset_def *pr;
	char hist_buf[80];

	if (id < 0 || id >= DIFFICULTY_COUNT) {
		difficulty_preset = DIFFICULTY_CUSTOM;
		return;
	}

	difficulty_preset = id;
	pr = &PRESETS[id];

	/* Apply option overrides directly to the player's option array */
	p->opts.opt[OPT_birth_second_wind]    = pr->birth_second_wind;
	p->opts.opt[OPT_show_threat_meter]    = pr->show_threat_meter;
	p->opts.opt[OPT_birth_connect_stairs] = pr->birth_connect_stairs;
	p->opts.opt[OPT_birth_start_kit]      = pr->birth_start_kit;
	p->opts.opt[OPT_birth_no_selling]     = pr->birth_no_selling;
	p->opts.opt[OPT_birth_ai_learn]       = pr->birth_ai_learn;

	/* Stash charge count for player-birth.c to pick up */
	preset_second_wind_charges = pr->second_wind_charges;

	/* Record in character history for score-legal transparency */
	strnfmt(hist_buf, sizeof(hist_buf),
		"Started as a %s", pr->name);
	history_add(p, hist_buf, HIST_GENERIC);
}
