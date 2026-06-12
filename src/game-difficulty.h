/**
 * \file game-difficulty.h
 * \brief Difficulty preset system
 *
 * Three named presets (Wanderer / Adventurer / Legend) that bundle the
 * Second Wind, Danger Telemetry, and birth options into one legible choice
 * at character creation.
 *
 * Copyright (c) 2024 Angband developers
 * Licensed under the Angband Licence (see COPYING.txt)
 */

#ifndef INCLUDED_GAME_DIFFICULTY_H
#define INCLUDED_GAME_DIFFICULTY_H

#define DIFFICULTY_WANDERER    0  /* Soft mode — 3× second wind, threat meter, generous gold */
#define DIFFICULTY_ADVENTURER  1  /* Recommended — 1× second wind, threat meter, standard rules */
#define DIFFICULTY_LEGEND      2  /* Vanilla permadeath — no net, no telemetry */
#define DIFFICULTY_CUSTOM     -1  /* Player manually adjusted options; no preset active */

#define DIFFICULTY_COUNT       3

/**
 * Currently active difficulty preset id (set at birth, persisted to savefile
 * via character history).  DIFFICULTY_CUSTOM means no preset is active.
 */
extern int difficulty_preset;

/**
 * Number of Second Wind charges granted to a new character by the active
 * preset.  Set by apply_difficulty_preset(); read by player-birth.c.
 */
extern int preset_second_wind_charges;

const char *difficulty_preset_name(int preset_id);
const char *difficulty_preset_desc(int preset_id);
void apply_difficulty_preset(struct player *p, int preset_id);

#endif /* INCLUDED_GAME_DIFFICULTY_H */
