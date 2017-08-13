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
#include "target.h"

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

/**
 * Find the player's alignment
 */
int get_player_alignment(void)
{
	if (p_ptr->alignment == (-1 * PY_MAX_ALIGN))
	    return ALIGN_CHAOS_PURE;
    if (p_ptr->alignment < (-1 * PY_ALIGN_CHANGE))
        return ALIGN_CHAOS;
    if (p_ptr->alignment == PY_MAX_ALIGN)
        return ALIGN_HARMONY_PURE;
    if (p_ptr->alignment > PY_ALIGN_CHANGE)
        return ALIGN_HARMONY;
    
    /* Default */
    return ALIGN_NEUTRAL;
}

static int get_max_pets(void)
{
    int max_pet = adj_cha_max_pet[p_ptr->stat_cur[A_CHR]];
    
    /* When abilities increase your maximum number of pets, add code here */
    
    return max_pet;
}

/*
 * Add a new pet
 */
bool add_pet(monster_type *m_ptr)
{
	int i;
	int max_pet = get_max_pets();

	/* Determine if you are over your limit */
	if (p_ptr->curr_pets >= max_pet)
	{
		return FALSE;
	}
	if (p_ptr->curr_pets >= MAX_NUM_PETS)
	{
		return FALSE;
	}

	/* Add the pet to the list */
	for (i = 0; i < max_pet; i++)
	{
		if (p_ptr->pet_list[i] != 0)
			continue;
		p_ptr->pet_list[i] = m_ptr;
		m_ptr->pet_num = i;

		m_ptr->faction = F_PLAYER;
		m_ptr->hostile = 0;
		m_ptr->group = 0;
		m_ptr->group_leader = -1;
		m_ptr->y_terr = p_ptr->py;
		m_ptr->x_terr = p_ptr->px;
		target_get((s16b *) & m_ptr->ty, (s16b *) & m_ptr->tx);
		p_ptr->curr_pets++;
		return TRUE;
	}
	return FALSE;
}

/* Helper function for compact_pets to sort the list */
/*static void compact_pets_aux(monster_type* arr[], int left, int right)
{
	int i = left, j = right;
	monster_type *tmp_ptr;
	int pivot = left + right / 2;
*/
	/* Partition */
/*	while (i <= j)
	{
		while (arr[i] > 0)
			i++;
		while (arr[j] <= 0)
			j++;
		if ((i <= j))
		{
			tmp_ptr = arr[i];
			arr[i] = arr[j];
			arr[j] = tmp_ptr;
			i++;
			j--;
		}
	}

	if (left < j)
		compact_pets_aux(arr, left, j);
	if (i < right)
		compact_pets_aux(arr, i, right);
}*/

void compact_pets(void)
{
	int i, curr, last;
	monster_type* tmp_ptr;

	/* Compact the list by moving all non-blank entries to the left */
	/* compact_pets_aux(p_ptr->pet_list, 0, MAX_NUM_PETS); */
	curr = 0;
	last = MAX_NUM_PETS;
	while (curr < last)
	{
	    while (p_ptr->pet_list[last] == 0)
	        last--;
        while (p_ptr->pet_list[curr] != 0)
            curr++;
        if (curr < last)
        {
            tmp_ptr = p_ptr->pet_list[last];
            p_ptr->pet_list[last] = p_ptr->pet_list[curr];
            p_ptr->pet_list[curr] = tmp_ptr;
        }
    }

	/* Update the indexes of all pets */
	for (i = 0; i < MAX_NUM_PETS; i++)
	{
		p_ptr->pet_list[i]->pet_num = i;
	}
}

/* Helper function to determine which pet to release */
static int reduce_pets_helper(monster_type* pet)
{
    
    monster_race* pet_race = &r_info[pet->r_idx];
    int score = 0;
    
    /* Favor low level pets */
    score += 100 - pet_race->level;
    
    /* Favor injured pets */
    score += 100 - (100 * pet->hp / pet->maxhp);
    
    /* Favor distant pets */
    score += pet->cdis;
    
    /* Scared pets will abandon you */
    if (pet->monfear)
       score += 30;
    
    return score;
}


/* Free pets if over the pet limit */
bool reduce_pets(void)
{
    bool reduced = FALSE;
    int max_pet = get_max_pets();
    
    while (p_ptr->curr_pets > max_pet)
    {
        /* Determine the best pet to get rid of */
        int this_pet = 0;
        int this_score = reduce_pets_helper(p_ptr->pet_list[this_pet]);
        int compare_score = 0;
        
        for (int i = 1; i < MAX_NUM_PETS; i++)
        {
            /* Skip null pets */
            if (p_ptr->pet_list[i])
            {
                compare_score = reduce_pets_helper(p_ptr->pet_list[i]);
                if (compare_score > this_score)
                {
                    this_pet = i;
                    this_score = compare_score;
                }
            }
        }
        
        /* Alert the player */
        msg("%s is no longer under your command", r_info[p_ptr->pet_list[this_pet]->r_idx].name);
        
        /* We found our candidate - now release them */
        release_pet(p_ptr->pet_list[this_pet]);
        reduced = TRUE;
    }
    
    return reduced;
}

/* Release a listed pet from your control */
void release_pet(monster_type *m_ptr)
{
    /* Remove from the pet list */
    p_ptr->pet_list[m_ptr->pet_num] = 0;
    m_ptr->pet_num = -1;
    
    /* Set to non-hostile "released pet" faction */
    m_ptr->faction = F_RELEASED_PET;
    m_ptr->hostile = 0;
    
    /* Clear various data for the monster */
    m_ptr->group = 0;
	m_ptr->group_leader = 0;
	m_ptr->y_terr = m_ptr->fy;
	m_ptr->x_terr = m_ptr->fx;
	
	/* Reduce the pet count */
	p_ptr->curr_pets--;
}
