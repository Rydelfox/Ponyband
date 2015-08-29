#include "angband.h"
#include "birth.h"				/* find_roman_suffix_start */
#include "tvalsval.h"

/**
 * Modify a stat value by a "modifier", return new value
 *
 * Stats go up: 3,4,...,17,18,18/10,18/20,...,18/220
 * Or even: 18/13, 18/23, 18/33, ..., 18/220
 *
 * Stats go down: 18/220, 18/210,..., 18/10, 18, 17, ..., 3
 * Or even: 18/13, 18/03, 18, 17, ..., 3
 */

#include "cave.h"

s16b modify_stat_value(int value, int amount)
{
	int i;

	/* Reward */
	if (amount > 0) {
		/* Apply each point */
		for (i = 0; i < amount; i++) {
			/* One point at a time */
			if (value < 18)
				value++;

			/* Ten "points" at a time */
			else
				value += 10;
		}
	}

	/* Penalty */
	else if (amount < 0) {
		/* Apply each point */
		for (i = 0; i < (0 - amount); i++) {
			/* Ten points at a time */
			if (value >= 18 + 10)
				value -= 10;

			/* Hack -- prevent weirdness */
			else if (value > 18)
				value = 18;

			/* One point at a time */
			else if (value > 3)
				value--;
		}
	}

	/* Return new value */
	return (value);
}

/* Is the player capable of casting a spell? */
bool player_can_cast(void)
{
	/* Races without Horns or Hands cannot cast arcane spells */
	if((!rp_ptr->num_rings) && p_ptr->cumber_glove) {
	    msg("You need a horn, hands, or claws to cast arcane spells.");
	    return FALSE;
	}
	
    if (player_has(PF_PROBE)) {
		if (p_ptr->lev < 35) {
			msg("You do not know how to probe monsters yet.");
			return FALSE;
		} else if ((p_ptr->timed[TMD_CONFUSED])
				   || (p_ptr->timed[TMD_IMAGE])) {
			msg("You feel awfully confused.");
			return FALSE;
		}
	}

	if (p_ptr->timed[TMD_BLIND] || no_light()) {
		msg("You cannot see!");
		return FALSE;
	}

	if (p_ptr->timed[TMD_CONFUSED]) {
		msg("You are too confused!");
		return FALSE;
	}

	return TRUE;
}

/* Is the player capable of studying? */
bool player_can_study(void)
{
	if (!player_can_cast())
		return FALSE;

	if (!p_ptr->new_spells) {
		const char *p = magic_desc[mp_ptr->spell_realm][SPELL_NOUN];
		msg("You cannot learn any new %ss!", p);
		return FALSE;
	}

	return TRUE;
}

/* Determine if the player can read scrolls. */
bool player_can_read(void)
{
	if (p_ptr->timed[TMD_BLIND]) {
		msg("You can't see anything.");
		return FALSE;
	}

	if (no_light()) {
		msg("You have no light to read by.");
		return FALSE;
	}

	if (p_ptr->timed[TMD_CONFUSED]) {
		msg("You are too confused to read!");
		return FALSE;
	}

	return TRUE;
}

/* Determine if the player can fire with the bow */
bool player_can_fire(void)
{
	object_type *o_ptr = &p_ptr->inventory[INVEN_BOW];

	/* Require a usable launcher */
	if (!o_ptr->tval || !p_ptr->state.ammo_tval) {
		msg("You have nothing to fire with.");
		return FALSE;
	}

	return TRUE;
}

/**
 * Return a version of the player's name safe for use in filesystems.
 */
const char *player_safe_name(struct player *p, bool strip_suffix)
{
	static char buf[40];
	int i;
	int limit = 0;

	if (op_ptr->full_name[0]) {
		char *suffix = find_roman_suffix_start(op_ptr->full_name);
		if (suffix)
			limit = suffix - op_ptr->full_name - 1;	/* -1 for preceding space */
		else
			limit = strlen(op_ptr->full_name);
	}

	for (i = 0; i < limit; i++) {
		char c = op_ptr->full_name[i];

		/* Convert all non-alphanumeric symbols */
		if (!isalpha((unsigned char) c) && !isdigit((unsigned char) c))
			c = '_';

		/* Build "base_name" */
		buf[i] = c;
	}

	/* Terminate */
	buf[i] = '\0';

	/* Require a "base" name */
	if (!buf[0])
		my_strcpy(buf, "PLAYER", sizeof buf);

	return buf;
}

/* Can the player use innate abilities? */
bool player_can_ability(void)
{
    if(player_ability_count() < 1) {
        msg("You do not have any innate abilities");
        return FALSE;
    }
    
    if (p_ptr->timed[TMD_CONFUSED]) {
		msg("You are too confused!");
		return FALSE;
	}
	
	return TRUE;
}

/* Alter alignment when hitting an enemy */
void hit_alignment(u16b faction, int depth, bool evil, bool unique, s16b sleep, bool killed)
{
    int i;
    
    if(faction == F_GOOD) p_ptr->alignment--;
    if((faction == F_GOOD) && killed) p_ptr->alignment-=2;
    if(evil && killed) p_ptr->alignment++;
    if((faction == F_RELEASED_PET) && killed) p_ptr->alignment-=4;
    if(faction == F_PLAYER) p_ptr->alignment-=4;
    if(sleep) {
        if(evil) {
            if(!randint0(4)) {
                p_ptr->alignment--;
            }
        }else {
            p_ptr->alignment--;
        }
    }
    if((faction == F_GOOD) && killed && unique) p_ptr->alignment-=50;
    if(evil && killed && unique) p_ptr->alignment+=50;
    if(killed && !depth) p_ptr->alignment--;
    
    if(p_ptr->alignment > PY_MAX_ALIGN)
        p_ptr->alignment = PY_MAX_ALIGN;
    else if (p_ptr->alignment < (-1 * PY_MAX_ALIGN))
        p_ptr->alignment = (-1 * PY_MAX_ALIGN);
}

