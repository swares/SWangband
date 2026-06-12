/**
 * \file threat.h
 * \brief Danger telemetry — turn-by-turn threat assessment model.
 *
 * The data structures (threat_tier, threat_foe, threat_summary, TF_* flags)
 * are defined in player.h to avoid circular includes.  This header only
 * declares the public functions.
 */

#ifndef INCLUDED_THREAT_H
#define INCLUDED_THREAT_H

#include "player.h"

struct chunk;  /* forward decl */

/**
 * Recompute the threat summary from scratch.
 * Pure function of (player, cave): no UI calls, no state changes.
 */
void threat_assess(struct player *p, struct chunk *c,
                   struct threat_summary *out);

/** Classify a single foe record into a threat tier. */
enum threat_tier threat_tier_of(const struct threat_foe *f);

/** Hit points lost per player-turn from current poison + bleeding. */
int player_dot_per_turn(const struct player *p);

/** Does the player carry a fast escape item? */
bool player_has_escape(struct player *p, const char **name_out);

#endif /* INCLUDED_THREAT_H */
