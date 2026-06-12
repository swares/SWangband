/**
 * \file threat.c
 * \brief Danger telemetry — turn-by-turn threat assessment.
 *
 * Pure read-only module: reads monster lore, player state, and timed effects
 * but never modifies game state or calls any UI functions.
 */

#include "angband.h"
#include "cave.h"
#include "game-world.h"
#include "mon-lore.h"
#include "mon-predicate.h"
#include "mon-spell.h"
#include "monster.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player.h"
#include "player-timed.h"
#include "init.h"
#include "player-util.h"
#include "threat.h"

/* -----------------------------------------------------------------------
 * Internal helpers
 * ----------------------------------------------------------------------- */

/**
 * Average melee damage of one full round by this monster.
 * Sets *certain = false if any blow has never been seen.
 */
static int expected_melee(const struct monster *mon, bool *certain)
{
	const struct monster_race *r = mon->race;
	const struct monster_lore *l = get_lore(r);
	int total = 0;
	int i;

	for (i = 0; i < z_info->mon_blows_max && r->blow[i].method; i++) {
		if (!l->blows[i].times_seen)
			*certain = false;
		total += randcalc(r->blow[i].dice, r->level, AVERAGE);
	}
	return total;
}

/**
 * How many of the monster's turns land per one of the player's turns.
 * Returned as a 10x fixed-point integer (10 = same speed).
 */
static int speed_ratio_x10(const struct monster *mon, const struct player *p)
{
	int mine = turn_energy(p->state.speed);
	int its  = turn_energy(mon->mspeed);
	return its * 10 / MAX(1, mine);
}

/**
 * Behaviour flags for this monster, read from race/lore definitions.
 */
static uint16_t threat_flags(const struct monster *mon, const struct player *p)
{
	const struct monster_race *r = mon->race;
	uint16_t f = 0;
	bitflag mask[RSF_SIZE];
	int i;

	if (r->level - p->depth >= 5)  f |= TF_OOD;
	if (mon->mspeed > p->state.speed) f |= TF_FAST;
	if (rf_has(r->flags, RF_MULTIPLY)) f |= TF_BREEDS;
	if (rf_has(r->flags, RF_UNIQUE))   f |= TF_UNIQUE;

	/* Summon spells */
	create_mon_spell_mask(mask, RST_SUMMON, RST_NONE);
	if (rsf_is_inter(r->spell_flags, mask)) f |= TF_SUMMONS;

	/* Breath attacks */
	create_mon_spell_mask(mask, RST_BREATH, RST_NONE);
	if (rsf_is_inter(r->spell_flags, mask)) f |= TF_BREATH;

	/* Damaging bolt / ball spells */
	create_mon_spell_mask(mask, RST_BOLT, RST_BALL, RST_NONE);
	if (rsf_is_inter(r->spell_flags, mask)) f |= TF_RANGED;

	/* Blow effects that drain XP or stats */
	for (i = 0; i < z_info->mon_blows_max && r->blow[i].method; i++) {
		const char *e = r->blow[i].effect ? r->blow[i].effect->name : "";
		if (prefix(e, "EXP_"))  f |= TF_DRAIN_XP;
		if (prefix(e, "LOSE_") || streq(e, "SHATTER")) f |= TF_DRAIN_STAT;
	}

	return f;
}

/* -----------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------- */

enum threat_tier threat_tier_of(const struct threat_foe *f)
{
	if (f->ttk <= 1)
		return THREAT_LETHAL;
	if (f->ttk <= 5 || (f->flags & TF_OOD))
		return THREAT_DANGER;
	if (f->dpt > 0)
		return THREAT_CAUTION;
	return THREAT_CLEAR;
}

int player_dot_per_turn(const struct player *p)
{
	int dot = 0;

	/* Poison: always 1 hp/turn (after any reduction) — we estimate raw 1 */
	if (p->timed[TMD_POISONED])
		dot += 1;

	/* Cuts: severity determines rate */
	if (p->timed[TMD_CUT]) {
		if (player_timed_grade_eq((struct player *)p, TMD_CUT, "Mortal Wound") ||
		    player_timed_grade_eq((struct player *)p, TMD_CUT, "Deep Gash"))
			dot += 3;
		else if (player_timed_grade_eq((struct player *)p, TMD_CUT, "Severe Cut"))
			dot += 2;
		else
			dot += 1;
	}

	return dot;
}

bool player_has_escape(struct player *p, const char **name_out)
{
	static const char *order[] = {
		"Phase Door",
		"Teleportation",
		"Word of Recall"
	};
	size_t k;

	for (k = 0; k < N_ELEMENTS(order); k++) {
		struct object *obj;
		int sval = lookup_sval(TV_SCROLL, order[k]);
		if (sval < 0) continue;
		for (obj = p->gear; obj; obj = obj->next) {
			if (tval_is_scroll(obj) && obj->sval == sval) {
				*name_out = order[k];
				return true;
			}
		}
	}
	return false;
}

void threat_assess(struct player *p, struct chunk *c,
                   struct threat_summary *out)
{
	int i;
	int dot;

	memset(out, 0, sizeof(*out));
	out->tier    = THREAT_CLEAR;
	out->dot_ttk = -1;

	/* Hallucination: suppress reliable data */
	if (p->timed[TMD_IMAGE]) {
		out->tier = THREAT_CAUTION; /* "something is out there" */
		return;
	}

	/* Score each visible, awake monster */
	for (i = 1; i < cave_monster_max(c) && out->n < THREAT_MAX_FOES; i++) {
		struct monster *mon = cave_monster(c, i);
		struct threat_foe *f;

		if (!mon->race) continue;               /* dead slot */
		if (!monster_is_visible(mon)) continue; /* not seen */
		if (mon->m_timed[MON_TMD_SLEEP]) continue; /* asleep */

		f = &out->foe[out->n++];
		f->mon     = mon;
		f->certain = true;
		f->ood     = mon->race->level - p->depth;
		f->flags   = threat_flags(mon, p);

		{
			int melee = expected_melee(mon, &f->certain);
			f->dpt = melee * speed_ratio_x10(mon, p) / 10;
		}
		f->ttk = (f->dpt > 0)
		         ? (p->chp + f->dpt - 1) / f->dpt
		         : INT_MAX;

		if (threat_tier_of(f) > out->tier)
			out->tier = threat_tier_of(f);
	}

	/* Damage-over-time can kill with nothing adjacent */
	dot = player_dot_per_turn(p);
	if (dot > 0) {
		out->dot_ttk = (p->chp + dot - 1) / dot;
		if (out->dot_ttk <= 1 && THREAT_LETHAL > out->tier)
			out->tier = THREAT_LETHAL;
		else if (out->dot_ttk <= 5 && THREAT_DANGER > out->tier)
			out->tier = THREAT_DANGER;
	}

	out->escape_ready = player_has_escape(p, &out->escape_name);
}
