/*
 * File: ui-spell.c
 * Purpose: Spell UI handing
 *
 * Copyright (c) 2010 Andi Sidwell
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"
#include "cave.h"
#include "cmds.h"
#include "tvalsval.h"
#include "game-cmd.h"
#include "spells.h"
#include "ui.h"
#include "ui-menu.h"





/**
 * Warriors will eventually learn to pseudo-probe monsters.  If they use 
 * the browse command, give ability information. -LM-
 */
static void warrior_probe_desc(void)
{
	/* Save screen */
	screen_save();

	/* Erase the screen */
	Term_clear();

	/* Set the indent */
	text_out_indent = 5;

	/* Title in light blue. */
	text_out_to_screen(TERM_L_BLUE, "Warrior Pseudo-Probing Ability:");
	text_out_to_screen(TERM_WHITE, "\n\n");

	/* Print out information text. */
	text_out_to_screen(TERM_WHITE,
					   "Warriors learn to probe monsters at level 35.  This costs nothing except a full turn.  When you reach this level, type 'm', and target the monster you would like to learn more about.  This reveals the monster's race, approximate HPs, and basic resistances.  Be warned:  the information given is not always complete...");
	text_out_to_screen(TERM_WHITE, "\n\n\n");

	/* The "exit" sign. */
	text_out_to_screen(TERM_WHITE, "    (Press any key to continue.)\n");

	/* Wait for it. */
	(void) inkey_ex();

	/* Load screen */
	screen_load();
}

/**
 * Spell menu data struct
 */
struct spell_menu_data {
	int spells[PY_MAX_SPELLS];
	int n_spells;
	int book_sval;
	bool browse;
	 bool(*is_valid) (int spell);

	int selected_spell;
};

/**
 * Ability menu data struct
 */
struct ability_menu_data {
    int *abilities;
    int n_abilities;
    
    int selected_ability;
};

/**
 * Pet menu data struct
 */
struct pet_menu_data {
    int n_choices;
    
    int selected_option;
};


/**
 * Is item oid valid?
 */
static int spell_menu_valid(menu_type * m, int oid)
{
	struct spell_menu_data *d = menu_priv(m);
	int *spells = d->spells;

	return d->is_valid(spells[oid]);
}

/**
 * Check if an ability is valid. Hide it if it isn't
 */
static int ability_menu_valid(menu_type * m, int aid)
{
    int i;
    
    for (i = 0; i < z_info->ability_max; i++) {
        if(ability_info[i].aidx == aid) {
            if (player_contains(ability_info[i].pflags)) {
                return 1;
            } else {
                return 0;
            }
        }
    }
    return 0;
}

static int pet_menu_valid(menu_type * m, int aid)
{
    return TRUE;
}

/**
 * Display a row of the spell menu
 */
static void spell_menu_display(menu_type * m, int oid, bool cursor,
							   int row, int col, int wid)
{
	struct spell_menu_data *d = menu_priv(m);
	int spell = d->spells[oid];
	const magic_type *s_ptr = &mp_ptr->info[spell];

	char help[30];

	int attr_name, attr_extra, attr_book;
	int tval = mp_ptr->spell_book;
	const char *comment = NULL;

	/* Choose appropriate spellbook color. */
	if (tval == TV_MAGIC_BOOK) {
		if (d->book_sval < SV_BOOK_MIN_GOOD)
			attr_book = TERM_L_RED;
		else
			attr_book = TERM_RED;
	} else if (tval == TV_PRAYER_BOOK) {
		if (d->book_sval < SV_BOOK_MIN_GOOD)
			attr_book = TERM_L_BLUE;
		else
			attr_book = TERM_BLUE;
	} else if (tval == TV_DRUID_BOOK) {
		if (d->book_sval < SV_BOOK_MIN_GOOD)
			attr_book = TERM_L_GREEN;
		else
			attr_book = TERM_GREEN;
	} else if (tval == TV_NECRO_BOOK) {
		if (d->book_sval < SV_BOOK_MIN_GOOD)
			attr_book = TERM_L_PURPLE;
		else
			attr_book = TERM_PURPLE;
	} else
		attr_book = TERM_WHITE;

	if (p_ptr->spell_flags[spell] & PY_SPELL_FORGOTTEN) {
		comment = " forgotten";
		attr_name = TERM_L_WHITE;
		attr_extra = TERM_L_WHITE;
	} else if (p_ptr->spell_flags[spell] & PY_SPELL_LEARNED) {
		if (p_ptr->spell_flags[spell] & PY_SPELL_WORKED) {
			/* Get extra info */
			get_spell_info(mp_ptr->spell_book, s_ptr->index, help,
						   sizeof(help));
			comment = help;
			attr_name = attr_book;
			attr_extra = TERM_DEEP_L_BLUE;
		} else {
			comment = " untried";
			attr_name = TERM_WHITE;
			attr_extra = TERM_WHITE;
		}
	} else if (s_ptr->slevel <= p_ptr->lev) {
		comment = " unknown";
		attr_extra = TERM_L_WHITE;
		attr_name = TERM_WHITE;
	} else {
		comment = " difficult";
		attr_extra = TERM_MUD;
		attr_name = TERM_MUD;
	}

	/* Dump the spell --(-- */
	c_put_str(attr_name, format("%-30s", get_spell_name(s_ptr->index)),
			  row, col);
	put_str(format
			("%2d %4d %3d%%", s_ptr->slevel, spell_cost(spell),
			 spell_chance(spell)), row, col + 30);
	c_put_str(attr_extra, format("%s", comment), row, col + 42);
}

/**
 * Display a row of the ability menu
 */
static void ability_menu_display(menu_type * m, int oid, bool cursor,
                                 int row, int col, int wid)
{
    struct ability_menu_data *d = menu_priv(m);
    int ability = d->abilities[oid];
    const innate_ability *ia_ptr = &ability_info[ability];
    
    const char *comment = ia_ptr->mini;
    
        /* Dump the spell */
    c_put_str(TERM_WHITE, format("%-30s", ia_ptr->name), row, col);
    put_str(format("%2d %4d %3d%%", ia_ptr->level, ia_ptr->cost,
            ability_chance(ability)), row, col + 30);
    c_put_str(TERM_DEEP_L_BLUE, format("%s", comment), row, col + 42);
}

/**
 * Display a row of the pet command menu
 */
static void pet_menu_display(menu_type * m, int oid, bool cursor,
                             int row, int col, int wid)
{    
    char *comment;
    switch(oid)
    {
    case 0:
        comment = "Issue attack order";
        break;
    case 1:
        comment = "Change follow distance";
        break;
    case 2:
        comment = "Issue point order";
        break;
    case 3:
        comment = "Dismiss pet";
        break;
    case 4:
        comment = "Release pet";
        break;
    case 5:
    	comment = "Cancel orders";
    	break;
    default:
        comment = "Error - Missing option";
    }
    
    /* Dump the command */
    c_put_str(TERM_WHITE, format("%-30s", comment), row, col);
}
 
/**
 * Handle an event on a spell menu row.
 */
static bool spell_menu_handler(menu_type * m, const ui_event * e, int oid)
{
	struct spell_menu_data *d = menu_priv(m);

	if (e->type == EVT_SELECT) {
		d->selected_spell = d->spells[oid];
		return d->browse ? TRUE : FALSE;
	}

	return TRUE;
}

/**
 * Handle an event on an ability menu row.
 */
static bool ability_menu_handler(menu_type * m, const ui_event * e, int oid)
{
    struct ability_menu_data *d = menu_priv(m);
    
    if(e->type == EVT_SELECT) {
        d->selected_ability = d->abilities[oid];
        return FALSE;
    }
    
    return TRUE;
}


/**
 * Handle and event on a pet command row
 */
static bool pet_menu_handler(menu_type * m, const ui_event * e, int oid)
{
    struct pet_menu_data *d = menu_priv(m);
    
    if(e->type == EVT_SELECT) {
        d->selected_option = oid;
        return FALSE;
    }
    
    return TRUE;
}

/**
 * Show spell long description when browsing
 */
static void spell_menu_browser(int oid, void *data, const region * loc)
{
	struct spell_menu_data *d = data;
	int spell = d->spells[oid];
	const magic_type *s_ptr = &mp_ptr->info[spell];

	/* Redirect output to the screen */
	text_out_hook = text_out_to_screen;
	text_out_wrap = loc->col + 60;
	text_out_indent = loc->col - 1;
	text_out_pad = 1;

	Term_gotoxy(loc->col, loc->row + loc->page_rows);
	text_out_c(TERM_DEEP_L_BLUE,
			   format("\n%s\n", s_info[s_ptr->index].text));

	/* XXX */
	text_out_pad = 0;
	text_out_indent = 0;
	text_out_wrap = 0;
}

/**
 * Show ability long description when browsing
 */
static void ability_menu_browser(int oid, void *data, const region * loc)
{
    struct ability_menu_data *d = data;
    int ability = d->abilities[oid];
    const innate_ability *ia_ptr = &ability_info[ability];
    
    /* Redirect output to the screen */
	text_out_hook = text_out_to_screen;
	text_out_wrap = loc->col + 60;
	text_out_indent = loc->col - 1;
	text_out_pad = 1;

	Term_gotoxy(loc->col, loc->row + loc->page_rows);
	text_out_c(TERM_DEEP_L_BLUE,
	           format("\n%s\n", ability_info[ia_ptr->aidx].desc));
 
    /* XXX */
	text_out_pad = 0;
	text_out_indent = 0;
	text_out_wrap = 0;
}

static void pet_menu_browser(int oid, void *data, const region * loc)
{
    char desc[43];
    
    /* Set up ability description */
    switch(oid) {
    case 0:
        my_strcpy(desc, "Order pets to attack a single target", 37);
        break;
    case 1:
        my_strcpy(desc, "Change how closely pets should follow you", 43);
        break;
    case 2:
        my_strcpy(desc, "Order pets to a specific location", 34);
        break;
    case 3:
        my_strcpy(desc, "Kill one of your pets", 22);
        break;
    case 4:
        my_strcpy(desc, "Allow one of your pets to go free", 35);
        break;
    }
    
    /* Redirect output to the screen */
    text_out_hook = text_out_to_screen;
    text_out_wrap = loc->col + 60;
    text_out_indent = loc->col - 1;
    text_out_pad = 1;
    
    Term_gotoxy(loc->col, loc->row + loc->page_rows);
    text_out_c(TERM_DEEP_L_BLUE, format("\n%s\n", desc));
    
    /* XXX */
    text_out_pad = 0;
    text_out_indent = 0;
    text_out_wrap = 0;
}


static const menu_iter spell_menu_iter = {
	NULL,						/* get_tag = NULL, just use lowercase selections */
	spell_menu_valid,
	spell_menu_display,
	spell_menu_handler,
	NULL						/* no resize hook */
};

static const menu_iter ability_menu_iter = {
    NULL,
    ability_menu_valid,
    ability_menu_display,
    ability_menu_handler,
    NULL
};

static const menu_iter pet_menu_iter = {
    NULL,
    pet_menu_valid,
    pet_menu_display,
    pet_menu_handler,
    NULL
};

/**
 * Create and initialize the pet menu
 */
static menu_type *pet_menu_new(void)
{
    menu_type *m = menu_new(MN_SKIN_SCROLL, &pet_menu_iter);
    struct pet_menu_data *d = mem_alloc(sizeof *d);
    
    region loc = {-65, 1, 65, -99};
    
    d->selected_option = -1;
    
    d->n_choices = 6;
    
    menu_setpriv(m, d->n_choices, d);
    
    /* Set flags */
    m->header = "Option";
    m->flags = MN_CASELESS_TAGS;
    m->selections = lower_case;
    m->browse_hook = pet_menu_browser;
    
    /* Set size */
    loc.page_rows = 7;
    menu_layout(m, &loc);
    
    return m;
}

/** Clean up a pet menu instance */
static void pet_menu_destroy(menu_type *m)
{
    struct pet_menu_data *d = menu_priv(m);
    mem_free(d);
    mem_free(m);
}

/**
 * Run the pet menu to select a command 
 */
static int pet_menu_select(menu_type *m)
{
    struct pet_menu_data *d = menu_priv(m);
    
    screen_save();
    region_erase_bordered(&m->active);
    
    prt("Give which command?", 0, 0);
    
    menu_select(m, 0, TRUE);
    
    screen_load();
    
    return d->selected_option;
}

/**
 * Issue commands to pets
 */
void textui_pet(void)
{
    int selection = -1;
    menu_type *m;
    
    /* This list is full of predefined options, so build it manually */
    m = pet_menu_new();
    if (m) {
        selection = pet_menu_select(m);
        pet_menu_destroy(m);
    }
    
    if (selection >= 0) {
        cmd_insert(CMD_PET);
        cmd_set_arg_choice(cmd_get_top(), 0, selection);
    }
}

/** Create and initialise a spell menu, given an object and a validity hook */
static menu_type *spell_menu_new(const object_type * o_ptr,
								 bool(*is_valid) (int spell))
{
	menu_type *m = menu_new(MN_SKIN_SCROLL, &spell_menu_iter);
	struct spell_menu_data *d = mem_alloc(sizeof *d);

	region loc = { -65, 1, 65, -99 };

	/* collect spells from object */
	d->n_spells = spell_collect_from_book(o_ptr, d->spells);
	if (d->n_spells == 0
		|| !spell_okay_list(is_valid, d->spells, d->n_spells)) {
		mem_free(m);
		mem_free(d);
		return NULL;
	}

	/* copy across private data */
	d->is_valid = is_valid;
	d->selected_spell = -1;
	d->browse = FALSE;
	d->book_sval = o_ptr->sval;

	menu_setpriv(m, d->n_spells, d);

	/* set flags */
	m->header = "Name                             Lv Mana Fail Info";
	m->flags = MN_CASELESS_TAGS;
	m->selections = lower_case;
	m->browse_hook = spell_menu_browser;

	/* set size */
	loc.page_rows = d->n_spells + 1;
	menu_layout(m, &loc);

	return m;
}

/** Clean up a spell menu instance */
static void spell_menu_destroy(menu_type * m)
{
	struct spell_menu_data *d = menu_priv(m);
	mem_free(d);
	mem_free(m);
}

/**
 * Run the spell menu to select a spell.
 */
static int spell_menu_select(menu_type * m, const char *noun,
							 const char *verb)
{
	struct spell_menu_data *d = menu_priv(m);
	char buf[80];

	screen_save();
	region_erase_bordered(&m->active);

	/* Format, capitalise and display */
	strnfmt(buf, sizeof buf, "%s which %s? ", verb, noun);
	my_strcap(buf);
	prt(buf, 0, 0);

	menu_select(m, 0, TRUE);

	screen_load();

	return d->selected_spell;
}

/**
 * Run the spell menu, without selections.
 */
static void spell_menu_browse(menu_type * m, const char *noun)
{
	struct spell_menu_data *d = menu_priv(m);

	screen_save();

	region_erase_bordered(&m->active);
	prt(format("Browsing %ss.  Press Escape to exit.", noun), 0, 0);

	d->browse = TRUE;
	menu_select(m, 0, TRUE);

	screen_load();
}


/**
 * Interactively select a spell.
 *
 * Returns the spell selected, or -1.
 */
static int get_spell(const object_type * o_ptr, const char *verb,
					 bool(*spell_test) (int spell))
{
	menu_type *m;
	const char *noun;

	noun = magic_desc[mp_ptr->spell_realm][SPELL_NOUN];

	m = spell_menu_new(o_ptr, spell_test);
	if (m) {
		int spell = spell_menu_select(m, noun, verb);
		spell_menu_destroy(m);
		return spell;
	}

	return -1;
}

/** Create and initialiae a spell menu */
static menu_type *ability_menu_new(void)
{
    menu_type *m = menu_new(MN_SKIN_SCROLL, &ability_menu_iter);
    struct ability_menu_data *d = mem_alloc(sizeof *d);
        
    region loc = { -65, 1, 65, -99 };
    
    d->abilities = mem_alloc(sizeof(int) * z_info->ability_max);

    if(!player_ability_count()) {
        mem_free(m);
        mem_free(d);
        return NULL;
    }        
    
    /* Collect abilities */
    d->n_abilities = ability_collect(d->abilities, z_info->ability_max);
    if(!ability_okay_list(d->abilities, d->n_abilities)) {
        mem_free(m);
        mem_free(d);
        return NULL;
    }
    
    /* copy accross private data */
    d->selected_ability = -1;
    
    menu_setpriv(m, d->n_abilities, d);
    
    /* set flags */
    m->header = "Name                             Lv Cost Fail Info";
    m->flags = MN_CASELESS_TAGS;
    m->selections = lower_case;
    m->browse_hook = ability_menu_browser;
    
    /* set size */
    loc.page_rows = d->n_abilities + 1;
    menu_layout(m, &loc);
    
    return m;
}

/** Clean up an ability menu instance */
static void ability_menu_destroy(menu_type * m)
{
    struct ability_menu_data *d = menu_priv(m);
    mem_free(d);
    mem_free(m);
}

/**
 * Run the ability menu to select an ability
 */
static int ability_menu_select(menu_type * m)
{
    struct ability_menu_data *d = menu_priv(m);
    
    screen_save();
    region_erase_bordered(&m->active);
    
    prt("Use which ability?", 0, 0);
    
    menu_select(m, 0, TRUE);
    
    screen_load();
    
    return d->selected_ability;
}

/**
 * Interactively select an ability
 *
 * Returns the aidx of the selected ability or -1
 */
static int get_ability(void)
{
    menu_type *m;
    
    m = ability_menu_new();
    if (m) {
        int ability = ability_menu_select(m);
        ability_menu_destroy(m);
        return ability;
    }
    
    return -1;
}

/**
 * Browse a given book.
 */
void textui_book_browse(const object_type * o_ptr)
{
	menu_type *m;

	m = spell_menu_new(o_ptr, spell_okay_to_browse);
	if (m) {
		spell_menu_browse(m, magic_desc[mp_ptr->spell_realm][SPELL_NOUN]);
		spell_menu_destroy(m);
	} else {
		msg("You cannot browse that.");
	}
}

/**
 * Browse the given book.
 */
void textui_spell_browse(void)
{
	int item;

	char q[80];
	char s[80];

	if (mp_ptr->spell_realm == REALM_NONE) {
		if (player_has(PF_PROBE))
			warrior_probe_desc();
		else
			msg("You cannot read books!");
		return;
	}

	strnfmt(q, sizeof(q), "Browse which %s?",
			magic_desc[mp_ptr->spell_realm][BOOK_NOUN]);
	strnfmt(s, sizeof(s), " You have no %ss that you can read.",
			magic_desc[mp_ptr->spell_realm][BOOK_LACK]);

	item_tester_hook = obj_can_browse;
	if (!get_item(&item, q, s, CMD_BROWSE_SPELL,
				  (USE_INVEN | USE_FLOOR | IS_HARMLESS)))
		return;

	/* Track the object kind */
	track_object(item);
	handle_stuff(p_ptr);

	textui_book_browse(object_from_item_idx(item));
}

/**
 * Study a book to gain a new spell
 */
void textui_obj_study(void)
{
	int item;
	char q[80];
	char s[80];

	if (mp_ptr->spell_realm == REALM_NONE) {
		msg("You cannot read books!");
		return;
	}

	strnfmt(q, sizeof(q), "Study which %s?",
			magic_desc[mp_ptr->spell_realm][BOOK_NOUN]);
	strnfmt(s, sizeof(s), " You have no %ss that you can study.",
			magic_desc[mp_ptr->spell_realm][BOOK_LACK]);

	item_tester_hook = obj_can_study;
	if (!get_item(&item, q, s, CMD_STUDY_BOOK, (USE_INVEN | USE_FLOOR)))
		return;

	track_object(item);
	handle_stuff(p_ptr);

	if (mp_ptr->spell_book != TV_PRAYER_BOOK) {
		int spell = get_spell(object_from_item_idx(item),
							  "study", spell_okay_to_study);
		if (spell >= 0) {
			cmd_insert(CMD_STUDY_SPELL);
			cmd_set_arg_choice(cmd_get_top(), 0, spell);
		}
	} else {
		cmd_insert(CMD_STUDY_BOOK);
		cmd_set_arg_item(cmd_get_top(), 0, item);
	}
}

/**
 * Cast a spell from a book.
 */
void textui_obj_cast(void)
{
	int item;
	int spell;

	const char *verb = magic_desc[mp_ptr->spell_realm][SPELL_VERB];
	char q[80];
	char s[80];

	if (mp_ptr->spell_realm == REALM_NONE) {
		msg("You cannot read books!");
		return;
	}
	
	if((!rp_ptr->num_rings) && p_ptr->cumber_glove) {
	    msg("You cannot use arcane spells without a horn or fingers!");
	    return;
	}

	strnfmt(q, sizeof(q), "Use which %s?",
			magic_desc[mp_ptr->spell_realm][BOOK_NOUN]);
	strnfmt(s, sizeof(s), " You have no %ss that you can use.",
			magic_desc[mp_ptr->spell_realm][BOOK_LACK]);

	item_tester_hook = obj_can_cast_from;
	if (!get_item(&item, q, s, CMD_CAST, (USE_INVEN | USE_FLOOR)))
		return;

	/* Track the object kind */
	track_object(item);

	/* Ask for a spell */
	spell =
		get_spell(object_from_item_idx(item), verb, spell_okay_to_cast);
	if (spell >= 0) {
		cmd_insert(CMD_CAST);
		cmd_set_arg_choice(cmd_get_top(), 0, spell);
	}
}

/**
 * Use an innate ability
 */
void textui_ability(void)
{
    int ability, i;
    int num = 0;
    
    struct innate_ability *ability_list[z_info->ability_max];
    
    /* I need a list of usable abilities, without gaps... 
       I'll make my own ability list, with blackjack, and hookers! */
    for (i = 0; i < z_info->ability_max; i++)
        if (player_contains(ability_info[i].pflags))
            ability_list[num++] = &ability_info[i];
            
    if(p_ptr->timed[TMD_CONFUSED])
    {
        msg("You are too confused to use abilities right now.");
        return;
    }
    
    ability = get_ability();
    if (ability >= 0) {
        cmd_insert(CMD_ABILITY);
        cmd_set_arg_choice(cmd_get_top(), 0, ability);
    }
}
