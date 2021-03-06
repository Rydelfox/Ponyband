/*
 * File: init.c
 * Purpose: Various game initialistion routines
 *
 * Copyright (c) 1997 Ben Harrison
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
#include "buildid.h"
#include "cave.h"
#include "cmds.h"
#include "game-cmd.h"
#include "game-event.h"
#include "generate.h"
#include "history.h"
#include "init.h"
#include "keymap.h"
#include "mapmode.h"
#include "monster.h"
#include "option.h"
#include "parser.h"
#include "prefs.h"
#include "quest.h"
#include "randname.h"
#include "squelch.h"
#include "store.h"
#include "types.h"
#include "tvalsval.h"

#ifdef DEBUF
#include <stdio>
#endif

/*
 * This file is used to initialize various variables and arrays for the
 * Angband game.  Note the use of "fd_read()" and "fd_write()" to bypass
 * the common limitation of "read()" and "write()" to only 32767 bytes
 * at a time.
 *
 * Several of the arrays for Angband are built from "template" files in
 * the "lib/edit" directory.
 *
 * Warning -- the "ascii" file parsers use a minor hack to collect the
 * name and text information in a single pass.  Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 */

static const char *object_flags[] = {
#define OF(a, b) #a,
#include "list-object-flags.h"
#undef OF
	NULL
};

static const char *curse_flags[] = {
#define CF(a, b) #a,
#include "list-curse-flags.h"
#undef CF
	NULL
};

static const char *kind_flags[] = {
#define KF(a, b) #a,
#include "list-kind-flags.h"
#undef KF
	NULL
};

static const char *id_flags[] = {
#define IF(a, b) #a,
#include "list-identify-flags.h"
#undef IF
	NULL
};

static const char *player_info_flags[] = {
#define PF(a, b, c) #a
#include "list-player-flags.h"
#undef PF
	NULL
};

static const char *effect_list[] = {
#define EFFECT(x, y, r, z, w, v)    #x,
#include "list-effects.h"
#undef EFFECT
};

/**
 * Percentage resists
 */
static const char *player_resist_values[] = {
	"NULL",
	"RES_ACID",
	"RES_ELEC",
	"RES_FIRE",
	"RES_COLD",
	"RES_POIS",
	"RES_LIGHT",
	"RES_DARK",
	"RES_CONFU",
	"RES_SOUND",
	"RES_SHARD",
	"RES_NEXUS",
	"RES_NETHR",
	"RES_CHAOS",
	"RES_DISEN"
};

/**
 * Stat bonuses
 */
static const char *bonus_stat_values[] = {
	"NULL",
	"STR",
	"INT",
	"WIS",
	"DEX",
	"CON",
	"CHR"
};

/**
 * Other bonuses
 */
static const char *bonus_other_values[] = {
	"NULL",
	"MAGIC_MASTERY",
	"STEALTH",
	"SEARCH",
	"INFRA",
	"TUNNEL",
	"SPEED",
	"SHOTS",
	"MIGHT"
};

/**
 * Slays
 */
static const char *slay_values[] = {
	"NULL",
	"SLAY_ANIMAL",
	"SLAY_EVIL",
	"SLAY_UNDEAD",
	"SLAY_DEMON",
	"SLAY_ORC",
	"SLAY_TROLL",
	"SLAY_GIANT",
	"SLAY_DRAGON"
};

/**
 * Brands
 */
static const char *brand_values[] = {
	"NULL",
	"BRAND_ACID",
	"BRAND_ELEC",
	"BRAND_FIRE",
	"BRAND_COLD",
	"BRAND_POIS"
};

static u32b grab_one_effect(const char *what)
{
	size_t i;

	/* Scan activations */
	for (i = 0; i < N_ELEMENTS(effect_list); i++) {
		if (streq(what, effect_list[i]))
			return i;
	}

	/* Oops */
	msg("Unknown effect '%s'.", what);

	/* Error */
	return 0;
}

u32b grab_value(const char *what, const char **value_type, int num,
				int *val)
{
	int i;
	char s[80];
	char *t;

	my_strcpy(s, what, strlen(what));

	/* Find the first bracket */
	for (t = s; *t && (*t != '['); ++t)	/* loop */
		;

	/* Get the value */
	if (1 != sscanf(t + 1, "%d", val))
		return (PARSE_ERROR_INVALID_VALUE);

	/* Terminate the string */
	*t = '\0';

	/* Check the possibilities */
	for (i = 0; i < num; i++) {
		if (streq(s, value_type[i]))
			return i;
	}

	/* Not found */
	return (0);
}

/*
 * Find the default paths to all of our important sub-directories.
 *
 * All of the sub-directories should, by default, be located inside
 * the main directory, whose location is very system dependant and is 
 * set by the ANGBAND_PATH environment variable, if it exists. (On multi-
 * user systems such as Linux this is not the default - see config.h for
 * more info.)
 *
 * This function takes a writable buffers, initially containing the
 * "path" to the "config", "lib" and "data" directories, for example, 
 * "/etc/angband/", "/usr/share/angband" and "/var/games/angband" -
 * or a system dependant string, for example, ":lib:".  The buffer
 * must be large enough to contain at least 32 more characters.
 *
 * Various command line options may allow some of the important
 * directories to be changed to user-specified directories, most
 * importantly, the "apex" and "user" and "save" directories,
 * but this is done after this function, see "main.c".
 *
 * In general, the initial path should end in the appropriate "PATH_SEP"
 * string.  All of the "sub-directory" paths (created below or supplied
 * by the user) will NOT end in the "PATH_SEP" string, see the special
 * "path_build()" function in "util.c" for more information.
 *
 * Hack -- first we free all the strings, since this is known
 * to succeed even if the strings have not been allocated yet,
 * as long as the variables start out as "NULL".  This allows
 * this function to be called multiple times, for example, to
 * try several base "path" values until a good one is found.
 */
void init_file_paths(const char *configpath, const char *libpath,
					 const char *datapath)
{
#ifdef PRIVATE_USER_PATH
	char buf[1024];
#endif							/* PRIVATE_USER_PATH */

	/*** Free everything ***/

	/* Free the sub-paths */
	string_free(ANGBAND_DIR_APEX);
	string_free(ANGBAND_DIR_BONE);
	string_free(ANGBAND_DIR_EDIT);
	string_free(ANGBAND_DIR_FILE);
	string_free(ANGBAND_DIR_HELP);
	string_free(ANGBAND_DIR_INFO);
	string_free(ANGBAND_DIR_SAVE);
	string_free(ANGBAND_DIR_PREF);
	string_free(ANGBAND_DIR_USER);
	string_free(ANGBAND_DIR_XTRA);

	string_free(ANGBAND_DIR_XTRA_FONT);
	string_free(ANGBAND_DIR_XTRA_GRAF);
	string_free(ANGBAND_DIR_XTRA_SOUND);
	string_free(ANGBAND_DIR_XTRA_HELP);
	string_free(ANGBAND_DIR_XTRA_ICON);

	/*** Prepare the paths ***/

	/* Build path names */
	ANGBAND_DIR_EDIT = string_make(format("%sedit", configpath));
	ANGBAND_DIR_FILE = string_make(format("%sfile", libpath));
	ANGBAND_DIR_HELP = string_make(format("%shelp", libpath));
	ANGBAND_DIR_INFO = string_make(format("%sinfo", libpath));
	ANGBAND_DIR_PREF = string_make(format("%spref", configpath));
	ANGBAND_DIR_XTRA = string_make(format("%sxtra", libpath));

	/* Build xtra/ paths */
	ANGBAND_DIR_XTRA_FONT =
		string_make(format("%s" PATH_SEP "font", ANGBAND_DIR_XTRA));
	ANGBAND_DIR_XTRA_GRAF =
		string_make(format("%s" PATH_SEP "graf", ANGBAND_DIR_XTRA));
	ANGBAND_DIR_XTRA_SOUND =
		string_make(format("%s" PATH_SEP "sound", ANGBAND_DIR_XTRA));
	ANGBAND_DIR_XTRA_HELP =
		string_make(format("%s" PATH_SEP "help", ANGBAND_DIR_XTRA));
	ANGBAND_DIR_XTRA_ICON =
		string_make(format("%s" PATH_SEP "icon", ANGBAND_DIR_XTRA));

#ifdef PRIVATE_USER_PATH

	/* Build the path to the user specific directory */
	if (strncmp(ANGBAND_SYS, "test", 4) == 0)
		path_build(buf, sizeof(buf), PRIVATE_USER_PATH, "Test");
	else
		path_build(buf, sizeof(buf), PRIVATE_USER_PATH, VERSION_NAME);
	ANGBAND_DIR_USER = string_make(buf);

	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "scores");
	ANGBAND_DIR_APEX = string_make(buf);

	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "bone");
	ANGBAND_DIR_BONE = string_make(buf);

	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "save");
	ANGBAND_DIR_SAVE = string_make(buf);

#else							/* !PRIVATE_USER_PATH */

	/* Build pathnames */
	ANGBAND_DIR_USER = string_make(format("%suser", datapath));
	ANGBAND_DIR_APEX = string_make(format("%sapex", datapath));
	ANGBAND_DIR_BONE = string_make(format("%sbone", datapath));
	ANGBAND_DIR_SAVE = string_make(format("%ssave", datapath));

#endif							/* PRIVATE_USER_PATH */
}


/*
 * Create any missing directories. We create only those dirs which may be
 * empty (user/, save/, apex/, info/, help/). The others are assumed 
 * to contain required files and therefore must exist at startup 
 * (edit/, pref/, file/, xtra/).
 *
 * ToDo: Only create the directories when actually writing files.
 */
void create_needed_dirs(void)
{
	char dirpath[512];

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_USER, "");
	if (!dir_create(dirpath))
		quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_SAVE, "");
	if (!dir_create(dirpath))
		quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_APEX, "");
	if (!dir_create(dirpath))
		quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_INFO, "");
	if (!dir_create(dirpath))
		quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_HELP, "");
	if (!dir_create(dirpath))
		quit_fmt("Cannot create '%s'", dirpath);
}

static enum parser_error parse_z(struct parser *p)
{
	maxima *z;
	const char *label;
	int value;

	z = parser_priv(p);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (value < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (streq(label, "F"))
		z->f_max = value;
	else if (streq(label, "T"))
		z->trap_max = value;
	else if (streq(label, "K"))
		z->k_max = value;
	else if (streq(label, "A"))
		z->a_max = value;
	else if (streq(label, "X"))
		z->set_max = value;
	else if (streq(label, "E"))
		z->e_max = value;
	else if (streq(label, "R"))
		z->r_max = value;
	else if (streq(label, "V"))
		z->v_max = value;
	else if (streq(label, "W"))
		z->t_max = value;
	else if (streq(label, "P"))
		z->p_max = value;
	else if (streq(label, "C"))
		z->c_max = value;
	else if (streq(label, "H"))
		z->h_max = value;
	else if (streq(label, "B"))
		z->b_max = value;
	else if (streq(label, "S"))
		z->s_max = value;
	else if (streq(label, "O"))
		z->o_max = value;
	else if (streq(label, "M"))
		z->m_max = value;
	else if (streq(label, "L"))
		z->flavor_max = value;
	else if (streq(label, "X"))
		z->set_max = value;
	else if (streq(label, "N"))
		z->l_max = value;
	else if (streq(label, "U"))
	    z->cm_max = value;
    else if (streq(label, "I"))
        z->ability_max = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return 0;
}

struct parser *init_parse_z(void)
{
	struct maxima *z = mem_zalloc(sizeof *z);
	struct parser *p = parser_new();

	parser_setpriv(p, z);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "M sym label int value", parse_z);
	return p;
}

static errr run_parse_z(struct parser *p)
{
	return parse_file(p, "limits");
}

static errr finish_parse_z(struct parser *p)
{
	z_info = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_z(void)
{
	mem_free(z_info);
}

static struct file_parser z_parser = {
	"limits",
	init_parse_z,
	run_parse_z,
	finish_parse_z,
	cleanup_z
};

static enum parser_error parse_k_n(struct parser *p)
{
	int idx = parser_getint(p, "index");
	int i;
	const char *name = parser_getstr(p, "name");
	struct object_kind *h = parser_priv(p);

	struct object_kind *k = mem_alloc(sizeof *k);
	memset(k, 0, sizeof(*k));
	k->next = h;
	parser_setpriv(p, k);
	k->kidx = idx;
	k->name = string_make(name);

	/* Initialise values */
	for (i = 0; i < MAX_P_RES; i++)
		k->percent_res[i] = RES_LEVEL_BASE;
	for (i = 0; i < A_MAX; i++)
		k->bonus_stat[i] = BONUS_BASE;
	for (i = 0; i < MAX_P_BONUS; i++)
		k->bonus_other[i] = BONUS_BASE;
	for (i = 0; i < MAX_P_SLAY; i++)
		k->multiple_slay[i] = MULTIPLE_BASE;
	for (i = 0; i < MAX_P_BRAND; i++)
		k->multiple_brand[i] = MULTIPLE_BASE;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_g(struct parser *p)
{
	const wchar_t glyph = parser_getchar(p, "glyph");
	const char *color = parser_getsym(p, "color");
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->d_char = glyph;
	if (strlen(color) > 1)
		k->d_attr = color_text_to_attr(color);
	else
		k->d_attr = color_char_to_attr(color[0]);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_i(struct parser *p)
{
	struct object_kind *k = parser_priv(p);
	int tval;

	assert(k);

	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	k->tval = tval;
	k->sval = parser_getint(p, "sval");
	k->pval = parser_getrand(p, "pval");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_w(struct parser *p)
{
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->level = parser_getint(p, "level");
	k->weight = parser_getint(p, "weight");
	k->cost = parser_getint(p, "cost");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_a(struct parser *p)
{
	struct object_kind *k = parser_priv(p);
	char *s = string_make(parser_getstr(p, "pairs"));
	char *t;
	int depth, rarity, i = 0;
	assert(k);

	t = strtok(s, ":");
	while (t) {
		if (sscanf(t, "%d/%d", &depth, &rarity) != 2)
			return PARSE_ERROR_GENERIC;
		k->chance[i] = rarity;
		k->locale[i++] = depth;
		t = strtok(NULL, ":");
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_k_p(struct parser *p)
{
	struct object_kind *k = parser_priv(p);
	struct random hd = parser_getrand(p, "hd");
	assert(k);

	k->ac = parser_getint(p, "ac");
	k->dd = hd.dice;
	k->ds = hd.sides;
	k->to_h = parser_getrand(p, "to-h");
	k->to_d = parser_getrand(p, "to-d");
	k->to_a = parser_getrand(p, "to-a");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_m(struct parser *p)
{
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->gen_mult_prob = parser_getint(p, "prob");
	k->stack_size = parser_getrand(p, "stack");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_f(struct parser *p)
{
	struct object_kind *k = parser_priv(p);
	char *s = string_make(parser_getstr(p, "flags"));
	char *t;
	assert(k);

	t = strtok(s, " |");
	while (t) {
		bool found = FALSE;
		if (!grab_flag(k->flags_obj, OF_SIZE, object_flags, t))
			found = TRUE;
		if (!grab_flag(k->flags_curse, CF_SIZE, curse_flags, t))
			found = TRUE;
		if (!grab_flag(k->flags_kind, KF_SIZE, kind_flags, t))
			found = TRUE;
		if (!found)
			break;
		t = strtok(NULL, " |");
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;

}

static enum parser_error parse_k_b(struct parser *p)
{
	struct object_kind *k = parser_priv(p);
	char *s = string_make(parser_getstr(p, "values"));
	char *t;
	int val, which = 0;
	assert(k);

	t = strtok(s, " |");
	while (t) {
		which = grab_value(t, player_resist_values,
						   N_ELEMENTS(player_resist_values), &val);
		if (which) {
			k->percent_res[which - 1] = RES_LEVEL_BASE - val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, bonus_stat_values,
						   N_ELEMENTS(bonus_stat_values), &val);
		if (which) {
			k->bonus_stat[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, bonus_other_values,
						   N_ELEMENTS(bonus_other_values), &val);
		if (which) {
			k->bonus_other[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, slay_values, N_ELEMENTS(slay_values), &val);
		if (which) {
			k->multiple_slay[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which =
			grab_value(t, brand_values, N_ELEMENTS(brand_values), &val);
		if (which) {
			k->multiple_brand[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		break;
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_k_e(struct parser *p)
{
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->effect = grab_one_effect(parser_getsym(p, "name"));
	if (parser_hasval(p, "time"))
		k->time = parser_getrand(p, "time");
	if (!k->effect)
		return PARSE_ERROR_GENERIC;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_d(struct parser *p)
{
	struct object_kind *k = parser_priv(p);
	assert(k);
	k->text = string_append(k->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_k(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N int index str name", parse_k_n);
	parser_reg(p, "G char glyph sym color", parse_k_g);
	parser_reg(p, "I sym tval int sval rand pval", parse_k_i);
	parser_reg(p, "W int level int extra int weight int cost", parse_k_w);
	parser_reg(p, "A str pairs", parse_k_a);
	parser_reg(p, "P int ac rand hd rand to-h rand to-d rand to-a",
			   parse_k_p);
	parser_reg(p, "M int prob rand stack", parse_k_m);
	parser_reg(p, "F str flags", parse_k_f);
	parser_reg(p, "B str values", parse_k_b);
	parser_reg(p, "E sym name ?rand time", parse_k_e);
	parser_reg(p, "D str text", parse_k_d);
	return p;
}

static errr run_parse_k(struct parser *p)
{
	return parse_file(p, "object");
}

static errr finish_parse_k(struct parser *p)
{
	struct object_kind *k, *n;

	k_info = mem_zalloc(z_info->k_max * sizeof(*k));
	for (k = parser_priv(p); k; k = k->next) {
		if (k->kidx >= z_info->k_max)
			continue;
		memcpy(&k_info[k->kidx], k, sizeof(*k));
	}

	k = parser_priv(p);
	while (k) {
		n = k->next;
		mem_free(k);
		k = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_k(void)
{
	int idx;
	for (idx = 0; idx < z_info->k_max; idx++) {
		string_free(k_info[idx].name);
		mem_free(k_info[idx].text);
	}
	mem_free(k_info);
}

struct file_parser k_parser = {
	"object",
	init_parse_k,
	run_parse_k,
	finish_parse_k,
	cleanup_k
};

static enum parser_error parse_a_n(struct parser *p)
{
	int i;
	int idx = parser_getint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct artifact *h = parser_priv(p);

	struct artifact *a = mem_zalloc(sizeof *a);
	a->next = h;
	parser_setpriv(p, a);
	a->aidx = idx;
	a->name = string_make(name);

	/* Ignore all elements */
	flags_set(a->flags_obj, OF_SIZE, OF_PROOF_MASK, FLAG_END);

	/* Initialise values */
	for (i = 0; i < MAX_P_RES; i++)
		a->percent_res[i] = RES_LEVEL_BASE;
	for (i = 0; i < A_MAX; i++)
		a->bonus_stat[i] = BONUS_BASE;
	for (i = 0; i < MAX_P_BONUS; i++)
		a->bonus_other[i] = BONUS_BASE;
	for (i = 0; i < MAX_P_SLAY; i++)
		a->multiple_slay[i] = MULTIPLE_BASE;
	for (i = 0; i < MAX_P_BRAND; i++)
		a->multiple_brand[i] = MULTIPLE_BASE;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_i(struct parser *p)
{
	struct artifact *a = parser_priv(p);
	int tval, sval;

	assert(a);

	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	a->tval = tval;

	sval = lookup_sval(a->tval, parser_getsym(p, "sval"));
	if (sval < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;
	a->sval = sval;

	a->pval = parser_getint(p, "pval");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_w(struct parser *p)
{
	struct artifact *a = parser_priv(p);
	assert(a);

	a->level = parser_getint(p, "level");
	a->rarity = parser_getint(p, "rarity");
	a->weight = parser_getint(p, "weight");
	a->cost = parser_getint(p, "cost");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_p(struct parser *p)
{
	struct artifact *a = parser_priv(p);
	struct random hd = parser_getrand(p, "hd");
	assert(a);

	a->ac = parser_getint(p, "ac");
	a->dd = hd.dice;
	a->ds = hd.sides;
	a->to_h = parser_getint(p, "to-h");
	a->to_d = parser_getint(p, "to-d");
	a->to_a = parser_getint(p, "to-a");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_f(struct parser *p)
{
	struct artifact *a = parser_priv(p);
	char *s;
	char *t;
	assert(a);

	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	s = string_make(parser_getstr(p, "flags"));

	t = strtok(s, " |");
	while (t) {
		bool found = FALSE;
		if (!grab_flag(a->flags_obj, OF_SIZE, object_flags, t))
			found = TRUE;
		if (!grab_flag(a->flags_curse, CF_SIZE, curse_flags, t))
			found = TRUE;
		if (!grab_flag(a->flags_kind, KF_SIZE, kind_flags, t))
			found = TRUE;
		if (!found)
			break;
		t = strtok(NULL, " |");
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_a_b(struct parser *p)
{
	struct artifact *a = parser_priv(p);
	char *s = string_make(parser_getstr(p, "values"));
	char *t;
	int val, which = 0;
	assert(a);

	t = strtok(s, " |");
	while (t) {
		which = grab_value(t, player_resist_values,
						   N_ELEMENTS(player_resist_values), &val);
		if (which) {
			a->percent_res[which - 1] = RES_LEVEL_BASE - val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, bonus_stat_values,
						   N_ELEMENTS(bonus_stat_values), &val);
		if (which) {
			a->bonus_stat[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, bonus_other_values,
						   N_ELEMENTS(bonus_other_values), &val);
		if (which) {
			a->bonus_other[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, slay_values, N_ELEMENTS(slay_values), &val);
		if (which) {
			a->multiple_slay[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, brand_values,
						   N_ELEMENTS(brand_values), &val);
		if (which) {
			a->multiple_brand[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		break;
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_a_e(struct parser *p)
{
	struct artifact *a = parser_priv(p);
	assert(a);

	a->effect = grab_one_effect(parser_getsym(p, "name"));
	a->time = parser_getrand(p, "time");
	if (!a->effect)
		return PARSE_ERROR_GENERIC;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_m(struct parser *p)
{
	struct artifact *a = parser_priv(p);
	assert(a);

	a->effect_msg = string_append(a->effect_msg, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_d(struct parser *p)
{
	struct artifact *a = parser_priv(p);
	assert(a);

	a->text = string_append(a->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_a(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N int index str name", parse_a_n);
	parser_reg(p, "I sym tval sym sval int pval", parse_a_i);
	parser_reg(p, "W int level int rarity int weight int cost", parse_a_w);
	parser_reg(p, "P int ac rand hd int to-h int to-d int to-a",
			   parse_a_p);
	parser_reg(p, "F ?str flags", parse_a_f);
	parser_reg(p, "B ?str values", parse_a_b);
	parser_reg(p, "E sym name rand time", parse_a_e);
	parser_reg(p, "M str text", parse_a_m);
	parser_reg(p, "D str text", parse_a_d);
	return p;
}

static errr run_parse_a(struct parser *p)
{
	return parse_file(p, "artifact");
}

static errr finish_parse_a(struct parser *p)
{
	struct artifact *a, *n;

	a_info = mem_zalloc(z_info->a_max * sizeof(*a));
	for (a = parser_priv(p); a; a = a->next) {
		if (a->aidx >= z_info->a_max)
			continue;
		memcpy(&a_info[a->aidx], a, sizeof(*a));
	}

	a = parser_priv(p);
	while (a) {
		n = a->next;
		mem_free(a);
		a = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_a(void)
{
	int idx;
	for (idx = 0; idx < z_info->a_max; idx++) {
		string_free(a_info[idx].name);
		mem_free(a_info[idx].effect_msg);
		mem_free(a_info[idx].text);
	}
	mem_free(a_info);
}

struct file_parser a_parser = {
	"artifact",
	init_parse_a,
	run_parse_a,
	finish_parse_a,
	cleanup_a
};

/* Current set item */
static int current_item = -1;

static enum parser_error parse_set_n(struct parser *p)
{
	int idx = parser_getint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct set_type *h = parser_priv(p);

	struct set_type *set = mem_zalloc(sizeof *set);
	set->next = h;
	parser_setpriv(p, set);
	set->setidx = idx;
	set->name = string_make(name);
	current_item = -1;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_set_c(struct parser *p)
{
	struct set_type *set = parser_priv(p);
	assert(set);

	set->no_of_items = parser_getint(p, "num");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_set_d(struct parser *p)
{
	struct set_type *set = parser_priv(p);
	assert(set);

	set->text = string_append(set->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_set_p(struct parser *p)
{
	int i;
	struct set_type *set = parser_priv(p);
	assert(set);

	set->setidx = parser_getint(p, "a_idx");
	current_item++;

	/* Initialise values */
	for (i = 0; i < MAX_P_RES; i++)
		set->set_items[current_item].percent_res[i] = RES_LEVEL_BASE;
	for (i = 0; i < A_MAX; i++)
		set->set_items[current_item].bonus_stat[i] = BONUS_BASE;
	for (i = 0; i < MAX_P_BONUS; i++)
		set->set_items[current_item].bonus_other[i] = BONUS_BASE;
	for (i = 0; i < MAX_P_SLAY; i++)
		set->set_items[current_item].multiple_slay[i] = MULTIPLE_BASE;
	for (i = 0; i < MAX_P_BRAND; i++)
		set->set_items[current_item].multiple_brand[i] = MULTIPLE_BASE;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_set_f(struct parser *p)
{
	struct set_type *set = parser_priv(p);
	char *s;
	char *t;
	assert(set);
	assert(current_item > -1);

	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	s = string_make(parser_getstr(p, "flags"));

	t = strtok(s, " |");
	while (t) {
		bool found = FALSE;
		if (!grab_flag
			(set->set_items[current_item].flags_obj, OF_SIZE, object_flags,
			 t))
			found = TRUE;
		if (!grab_flag
			(set->set_items[current_item].flags_curse, CF_SIZE,
			 curse_flags, t))
			found = TRUE;
		if (!found)
			break;
		t = strtok(NULL, " |");
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_set_b(struct parser *p)
{
	struct set_type *set = parser_priv(p);
	char *s = string_make(parser_getstr(p, "values"));
	char *t;
	int val, which = 0;
	assert(set);
	assert(current_item > -1);

	t = strtok(s, " |");
	while (t) {
		which = grab_value(t, player_resist_values,
						   N_ELEMENTS(player_resist_values), &val);
		if (which) {
			set->set_items[current_item].percent_res[which - 1] =
				RES_LEVEL_BASE - val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, bonus_stat_values,
						   N_ELEMENTS(bonus_stat_values), &val);
		if (which) {
			set->set_items[current_item].bonus_stat[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, bonus_other_values,
						   N_ELEMENTS(bonus_other_values), &val);
		if (which) {
			set->set_items[current_item].bonus_other[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, slay_values, N_ELEMENTS(slay_values), &val);
		if (which) {
			set->set_items[current_item].multiple_slay[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, brand_values,
						   N_ELEMENTS(brand_values), &val);
		if (which) {
			set->set_items[current_item].multiple_brand[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		break;
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

struct parser *init_parse_set(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N int index str name", parse_set_n);
	parser_reg(p, "C int num", parse_set_c);
	parser_reg(p, "D str text", parse_set_d);
	parser_reg(p, "P int a_idx", parse_set_p);
	parser_reg(p, "F ?str flags", parse_set_f);
	parser_reg(p, "B ?str values", parse_set_b);
	return p;
}

static errr run_parse_set(struct parser *p)
{
	return parse_file(p, "set_item");
}

static errr finish_parse_set(struct parser *p)
{
	struct set_type *set, *n;

	set_info = mem_zalloc(z_info->set_max * sizeof(*set));
	for (set = parser_priv(p); set; set = set->next) {
		if (set->setidx >= z_info->set_max)
			continue;
		memcpy(&set_info[set->setidx], set, sizeof(*set));
	}

	set = parser_priv(p);
	while (set) {
		n = set->next;
		mem_free(set);
		set = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_set(void)
{
	int idx;
	for (idx = 0; idx < z_info->set_max; idx++) {
		string_free(set_info[idx].name);
		mem_free(set_info[idx].text);
	}
	mem_free(set_info);
}

struct file_parser set_parser = {
	"set_item",
	init_parse_set,
	run_parse_set,
	finish_parse_set,
	cleanup_set
};

struct name {
	struct name *next;
	char *str;
};

struct names_parse {
	unsigned int section;
	unsigned int nnames[RANDNAME_NUM_TYPES];
	struct name *names[RANDNAME_NUM_TYPES];
};

static enum parser_error parse_names_n(struct parser *p)
{
	unsigned int section = parser_getint(p, "section");
	struct names_parse *s = parser_priv(p);
    if (s->section >= RANDNAME_NUM_TYPES)
		return PARSE_ERROR_GENERIC;
	s->section = section;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_names_d(struct parser *p)
{
	const char *name = parser_getstr(p, "name");
	struct names_parse *s = parser_priv(p);
	struct name *ns = mem_zalloc(sizeof *ns);

    s->nnames[s->section]++;
	ns->next = s->names[s->section];
	ns->str = string_make(name);
	s->names[s->section] = ns;
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_names(void)
{
	struct parser *p = parser_new();
	struct names_parse *n = mem_zalloc(sizeof *n);
    n->section = 0;
	parser_setpriv(p, n);
	parser_reg(p, "N int section", parse_names_n);
	parser_reg(p, "D str name", parse_names_d);
	return p;
}

static errr run_parse_names(struct parser *p)
{
    return parse_file(p, "names");
}

static errr finish_parse_names(struct parser *p)
{
	int i;
	unsigned int j;
	struct names_parse *n = parser_priv(p);
	struct name *nm;
	name_sections = mem_zalloc(sizeof(char **) * RANDNAME_NUM_TYPES);
	for (i = 0; i < RANDNAME_NUM_TYPES; i++) {
		name_sections[i] = mem_alloc(sizeof(char *) * (n->nnames[i] + 1));
		for (nm = n->names[i], j = 0; nm && j < n->nnames[i];
			 nm = nm->next, j++) {
			name_sections[i][j] = nm->str;
		}
		name_sections[i][n->nnames[i]] = NULL;
		while (n->names[i]) {
			nm = n->names[i]->next;
			mem_free(n->names[i]);
			n->names[i] = nm;
		}
	}
	mem_free(n);
	parser_destroy(p);
	return 0;
}

static void cleanup_names(void)
{
	int i, j;
	for (i = 0; i < RANDNAME_NUM_TYPES; i++) {
		for (j = 0; name_sections[i][j]; j++) {
			string_free((char *) name_sections[i][j]);
		}
		mem_free(name_sections[i]);
	}
	mem_free(name_sections);
}

struct file_parser names_parser = {
	"names",
	init_parse_names,
	run_parse_names,
	finish_parse_names,
	cleanup_names
};

static const char *trap_flags[] = {
#define TRF(a, b) #a,
#include "list-trap-flags.h"
#undef TRF
	NULL
};

static enum parser_error parse_trap_n(struct parser *p)
{
	int idx = parser_getuint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct trap *h = parser_priv(p);

	struct trap *t = mem_zalloc(sizeof *t);
	t->next = h;
	t->tidx = idx;
	t->name = string_make(name);
	parser_setpriv(p, t);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_g(struct parser *p)
{
	char glyph = parser_getchar(p, "glyph");
	const char *color = parser_getsym(p, "color");
	int attr = 0;
	struct trap *t = parser_priv(p);

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	t->d_char = glyph;
	if (strlen(color) > 1)
		attr = color_text_to_attr(color);
	else
		attr = color_char_to_attr(color[0]);
	if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
	t->d_attr = attr;
	t->x_char = t->d_char;
	t->x_attr = t->d_attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_m(struct parser *p)
{
	struct trap *t = parser_priv(p);

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	t->rarity = parser_getuint(p, "rarity");
	t->min_depth = parser_getuint(p, "mindepth");
	t->max_num = parser_getuint(p, "maxnum");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_f(struct parser *p)
{
	char *flags;
	struct trap *t = parser_priv(p);
	char *s;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));

	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(t->flags, TRF_SIZE, trap_flags, s)) {
			mem_free(s);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_d(struct parser *p)
{
	struct trap *t = parser_priv(p);
	assert(t);

	t->text = string_append(t->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_trap(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index str name", parse_trap_n);
	parser_reg(p, "G char glyph sym color", parse_trap_g);
	parser_reg(p, "M uint rarity uint mindepth uint maxnum", parse_trap_m);
	parser_reg(p, "F ?str flags", parse_trap_f);
	parser_reg(p, "D str text", parse_trap_d);
	return p;
}

static errr run_parse_trap(struct parser *p)
{
	return parse_file(p, "trap");
}

static errr finish_parse_trap(struct parser *p)
{
	struct trap *t, *n;

	trap_info = mem_zalloc(z_info->trap_max * sizeof(*t));
	for (t = parser_priv(p); t; t = t->next) {
		if (t->tidx >= z_info->trap_max)
			continue;
		memcpy(&trap_info[t->tidx], t, sizeof(*t));
	}

	t = parser_priv(p);
	while (t) {
		n = t->next;
		mem_free(t);
		t = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_trap(void)
{
	int i;
	for (i = 0; i < z_info->trap_max; i++) {
		string_free(trap_info[i].name);
		mem_free(trap_info[i].text);
	}
	mem_free(trap_info);
}

struct file_parser trap_parser = {
	"trap",
	init_parse_trap,
	run_parse_trap,
	finish_parse_trap,
	cleanup_trap
};

static const char *terrain_flags[] = {
#define TF(a, b) #a,
#include "list-terrain-flags.h"
#undef TF
	NULL
};

static enum parser_error parse_f_n(struct parser *p)
{
	int idx = parser_getuint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct feature *h = parser_priv(p);

	struct feature *f = mem_zalloc(sizeof *f);
	f->next = h;
	f->fidx = idx;
	f->mimic = idx;
	f->priority = 0;
	f->name = string_make(name);
	parser_setpriv(p, f);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_f_g(struct parser *p)
{
	wchar_t glyph = parser_getchar(p, "glyph");
	const char *color = parser_getsym(p, "color");
	int attr = 0;
	struct feature *f = parser_priv(p);

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f->d_char = glyph;
	if (strlen(color) > 1)
		attr = color_text_to_attr(color);
	else
		attr = color_char_to_attr(color[0]);
	if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
	f->d_attr = attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_f_m(struct parser *p)
{
	unsigned int idx = parser_getuint(p, "index");
	struct feature *f = parser_priv(p);

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f->mimic = idx;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_f_p(struct parser *p)
{
	unsigned int priority = parser_getuint(p, "priority");
	struct feature *f = parser_priv(p);

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f->priority = priority;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_f_x(struct parser *p)
{
	struct feature *f = parser_priv(p);

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f->locked = parser_getint(p, "locked");
	f->jammed = parser_getint(p, "jammed");
	f->shopnum = parser_getint(p, "shopnum");
	f->dig = parser_getrand(p, "dig");
	f->duration = parser_getint(p, "duration");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_f_f(struct parser *p)
{
	char *flags;
	struct feature *f = parser_priv(p);
	char *s;

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));

	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(f->flags, TF_SIZE, terrain_flags, s)) {
			mem_free(s);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_f_d(struct parser *p)
{
	struct feature *f = parser_priv(p);
	assert(f);

	f->text = string_append(f->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_f(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index str name", parse_f_n);
	parser_reg(p, "G char glyph sym color", parse_f_g);
	parser_reg(p, "M uint index", parse_f_m);
	parser_reg(p, "P uint priority", parse_f_p);
	parser_reg(p, "X int locked int jammed int shopnum rand dig int duration",
			   parse_f_x);
	parser_reg(p, "F ?str flags", parse_f_f);
	parser_reg(p, "D str text", parse_f_d);
	return p;
}

static errr run_parse_f(struct parser *p)
{
	return parse_file(p, "terrain");
}

static errr finish_parse_f(struct parser *p)
{
	struct feature *f, *n;

	f_info = mem_zalloc(z_info->f_max * sizeof(*f));
	for (f = parser_priv(p); f; f = f->next) {
		if (f->fidx >= z_info->f_max)
			continue;
		memcpy(&f_info[f->fidx], f, sizeof(*f));
	}

	f = parser_priv(p);
	while (f) {
		n = f->next;
		mem_free(f);
		f = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_f(void)
{
	int idx;
	for (idx = 0; idx < z_info->f_max; idx++) {
		string_free(f_info[idx].name);
		mem_free(f_info[idx].text);
	}
	mem_free(f_info);
}

struct file_parser f_parser = {
	"terrain",
	init_parse_f,
	run_parse_f,
	finish_parse_f,
	cleanup_f
};

static enum parser_error parse_e_n(struct parser *p)
{
	int i;
	int idx = parser_getint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct ego_item *h = parser_priv(p);

	struct ego_item *e = mem_alloc(sizeof *e);
	memset(e, 0, sizeof(*e));
	e->next = h;
	parser_setpriv(p, e);
	e->eidx = idx;
	e->name = string_make(name);
	/* Initialise values */
	for (i = 0; i < MAX_P_RES; i++)
		e->percent_res[i] = RES_LEVEL_BASE;
	for (i = 0; i < A_MAX; i++)
		e->bonus_stat[i] = BONUS_BASE;
	for (i = 0; i < MAX_P_BONUS; i++)
		e->bonus_other[i] = BONUS_BASE;
	for (i = 0; i < MAX_P_SLAY; i++)
		e->multiple_slay[i] = MULTIPLE_BASE;
	for (i = 0; i < MAX_P_BRAND; i++)
		e->multiple_brand[i] = MULTIPLE_BASE;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_w(struct parser *p)
{
	int level = parser_getint(p, "level");
	int rarity = parser_getint(p, "rarity");
	int cost = parser_getint(p, "cost");
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	e->level = level;
	e->rarity = rarity;
	e->cost = cost;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_x(struct parser *p)
{
	int rating = parser_getint(p, "rating");
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	e->rating = rating;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_t(struct parser *p)
{
	int i;
	int tval;
	int min_sval, max_sval;

	struct ego_item *e = parser_priv(p);
	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	min_sval = parser_getint(p, "min-sval");
	max_sval = parser_getint(p, "max-sval");

	for (i = 0; i < EGO_TVALS_MAX; i++) {
		if (!e->tval[i]) {
			e->tval[i] = tval;
			e->min_sval[i] = min_sval;
			e->max_sval[i] = max_sval;
			break;
		}
	}

	if (i == EGO_TVALS_MAX)
		return PARSE_ERROR_GENERIC;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_c(struct parser *p)
{
	int th = parser_getint(p, "th");
	int td = parser_getint(p, "td");
	int ta = parser_getint(p, "ta");
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	e->max_to_h = th;
	e->max_to_d = td;
	e->max_to_a = ta;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_f(struct parser *p)
{
	struct ego_item *e = parser_priv(p);
	char *s;
	char *t;

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	s = string_make(parser_getstr(p, "flags"));
	t = strtok(s, " |");
	while (t) {
		bool found = FALSE;
		if (!grab_flag(e->flags_obj, OF_SIZE, object_flags, t))
			found = TRUE;
		if (!grab_flag(e->flags_curse, CF_SIZE, curse_flags, t))
			found = TRUE;
		if (!grab_flag(e->flags_kind, KF_SIZE, kind_flags, t))
			found = TRUE;
		if (!found)
			break;
		t = strtok(NULL, " |");
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_e_i(struct parser *p)
{
	struct ego_item *e = parser_priv(p);
	char *s;
	char *t;

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	s = string_make(parser_getstr(p, "flags"));
	t = strtok(s, " |");
	while (t) {
		bool found = FALSE;
		if (!grab_flag(e->id_obj, OF_SIZE, object_flags, t))
			found = TRUE;
		if (!grab_flag(e->id_curse, CF_SIZE, curse_flags, t))
			found = TRUE;
		if (!grab_flag(e->id_other, IF_SIZE, id_flags, t))
			found = TRUE;
		if (!found)
			break;
		t = strtok(NULL, " |");
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_e_b(struct parser *p)
{
	struct ego_item *e = parser_priv(p);
	char *s = string_make(parser_getstr(p, "values"));
	char *t;
	int val, which = 0;
	assert(e);

	t = strtok(s, " |");
	while (t) {
		which = grab_value(t, player_resist_values,
						   N_ELEMENTS(player_resist_values), &val);
		if (which) {
			e->percent_res[which - 1] = RES_LEVEL_BASE - val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, bonus_stat_values,
						   N_ELEMENTS(bonus_stat_values), &val);
		if (which) {
			e->bonus_stat[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, bonus_other_values,
						   N_ELEMENTS(bonus_other_values), &val);
		if (which) {
			e->bonus_other[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, slay_values, N_ELEMENTS(slay_values), &val);
		if (which) {
			e->multiple_slay[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		which = grab_value(t, brand_values,
						   N_ELEMENTS(brand_values), &val);
		if (which) {
			e->multiple_brand[which - 1] = val;
			t = strtok(NULL, " |");
			continue;
		}
		break;
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_e_e(struct parser *p)
{
	struct ego_item *e = parser_priv(p);
	assert(e);

	e->effect = grab_one_effect(parser_getsym(p, "name"));
	if (parser_hasval(p, "time"))
		e->time = parser_getrand(p, "time");
	if (!e->effect)
		return PARSE_ERROR_GENERIC;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_d(struct parser *p)
{
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	e->text = string_append(e->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_e(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N int index str name", parse_e_n);
	parser_reg(p, "W int level int rarity int pad int cost", parse_e_w);
	parser_reg(p, "X int rating", parse_e_x);
	parser_reg(p, "T sym tval int min-sval int max-sval", parse_e_t);
	parser_reg(p, "C int th int td int ta", parse_e_c);
	parser_reg(p, "F ?str flags", parse_e_f);
	parser_reg(p, "I ?str flags", parse_e_i);
	parser_reg(p, "B ?str values", parse_e_b);
	parser_reg(p, "E sym name rand time", parse_e_e);
	parser_reg(p, "D str text", parse_e_d);
	return p;
}

static errr run_parse_e(struct parser *p)
{
	return parse_file(p, "ego_item");
}

static errr finish_parse_e(struct parser *p)
{
	struct ego_item *e, *n;

	e_info = mem_zalloc(z_info->e_max * sizeof(*e));
	for (e = parser_priv(p); e; e = e->next) {
		if (e->eidx >= z_info->e_max)
			continue;
		memcpy(&e_info[e->eidx], e, sizeof(*e));
	}

	e = parser_priv(p);
	while (e) {
		n = e->next;
		mem_free(e);
		e = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_e(void)
{
	int idx;
	for (idx = 0; idx < z_info->e_max; idx++) {
		string_free(e_info[idx].name);
		mem_free(e_info[idx].text);
	}
	mem_free(e_info);
}

struct file_parser e_parser = {
	"ego_item",
	init_parse_e,
	run_parse_e,
	finish_parse_e,
	cleanup_e
};

/* Parsing functions for monster_base.txt */

static enum parser_error parse_rb_n(struct parser *p)
{
	struct monster_base *h = parser_priv(p);
	struct monster_base *rb = mem_zalloc(sizeof *rb);
	rb->next = h;
	rb->name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, rb);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_rb_g(struct parser *p)
{
	struct monster_base *rb = parser_priv(p);

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	rb->d_char = parser_getchar(p, "glyph");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_rb_m(struct parser *p)
{
	struct monster_base *rb = parser_priv(p);
	int pain_idx;

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	pain_idx = parser_getuint(p, "pain");
	if (pain_idx >= z_info->mp_max)
		/* XXX need a real error code for this */
		return PARSE_ERROR_GENERIC;

	rb->pain = &pain_messages[pain_idx];

	return PARSE_ERROR_NONE;
}

const char *r_info_flags[] = {
#define RF(a, b) #a,
#include "list-mon-flags.h"
#undef RF
	NULL
};

static enum parser_error parse_rb_f(struct parser *p)
{
	struct monster_base *rb = parser_priv(p);
	char *flags;
	char *s;

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(rb->flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			quit_fmt("bad f-flag: %s", s);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}

const char *r_info_spell_flags[] = {
#define RSF(a, b, c, d, e, f, g, h, i, j, k) #a,
#include "list-mon-spells.h"
#undef RSF
	NULL
};

static enum parser_error parse_rb_s(struct parser *p)
{
	struct monster_base *rb = parser_priv(p);
	char *flags;
	char *s;

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "spells"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(rb->spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
			mem_free(flags);
			quit_fmt("bad s-flag: %s", s);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_rb_d(struct parser *p)
{
	struct monster_base *rb = parser_priv(p);

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	rb->text = string_append(rb->text, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}


struct parser *init_parse_rb(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N str name", parse_rb_n);
	parser_reg(p, "G char glyph", parse_rb_g);
	parser_reg(p, "M uint pain", parse_rb_m);
	parser_reg(p, "F ?str flags", parse_rb_f);
	parser_reg(p, "S ?str spells", parse_rb_s);
	parser_reg(p, "D str desc", parse_rb_d);
	return p;
}

static errr run_parse_rb(struct parser *p)
{
	return parse_file(p, "monster_base");
}

static errr finish_parse_rb(struct parser *p)
{
	rb_info = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_rb(void)
{
	struct monster_base *rb, *next;

	rb = rb_info;
	while (rb) {
		next = rb->next;
		string_free(rb->text);
		string_free(rb->name);
		mem_free(rb);
		rb = next;
	}
}

struct file_parser rb_parser = {
	"monster_base",
	init_parse_rb,
	run_parse_rb,
	finish_parse_rb,
	cleanup_rb
};


/* Parsing functions for monster.txt */
static enum parser_error parse_r_n(struct parser *p)
{
	struct monster_race *h = parser_priv(p);
	struct monster_race *r = mem_alloc(sizeof *r);
	memset(r, 0, sizeof(*r));
	r->next = h;
	r->ridx = parser_getuint(p, "index");
	r->name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, r);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_t(struct parser *p)
{
	struct monster_race *r = parser_priv(p);

	r->base = lookup_monster_base(parser_getsym(p, "base"));
	if (r->base == NULL)
		/* Todo: make new error for this */
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	/* The template sets the default display character */
	r->d_char = r->base->d_char;

	/* Give the monster its default flags */
	rf_union(r->flags, r->base->flags);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_g(struct parser *p)
{
	struct monster_race *r = parser_priv(p);

	/* If the display character is specified, it overrides any template */
	r->d_char = parser_getchar(p, "glyph");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_c(struct parser *p)
{
	struct monster_race *r = parser_priv(p);
	const char *color;
	int attr;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	color = parser_getsym(p, "color");
	if (strlen(color) > 1)
		attr = color_text_to_attr(color);
	else
		attr = color_char_to_attr(color[0]);
	if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
	r->d_attr = attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_i(struct parser *p)
{
	random_value hp;
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->speed = parser_getint(p, "speed");
	hp = parser_getrand(p, "hp");
	r->hdice = hp.dice;
	r->hside = hp.sides;
	r->aaf = parser_getint(p, "aaf");
	r->ac = parser_getint(p, "ac");
	r->sleep = parser_getint(p, "sleep");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_w(struct parser *p)
{
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->level = parser_getint(p, "level");
	r->rarity = parser_getint(p, "rarity");
	r->mana = parser_getint(p, "mana");
	r->mexp = parser_getint(p, "mexp");
	r->faction = parser_getint(p, "faction");
	return PARSE_ERROR_NONE;
}

static const char *r_info_blow_method[] = {
#define RBM(a, b) #a,
#include "list-blow-methods.h"
#undef RBM
	NULL
};

static int find_blow_method(const char *name)
{
	int i;
	for (i = 0; r_info_blow_method[i]; i++)
		if (streq(name, r_info_blow_method[i]))
			break;
	return i;
}

static const char *r_info_blow_effect[] = {
#define RBE(a, b) #a,
#include "list-blow-effects.h"
#undef RBE
	NULL
};

static int find_blow_effect(const char *name)
{
	int i;
	for (i = 0; r_info_blow_effect[i]; i++)
		if (streq(name, r_info_blow_effect[i]))
			break;
	return i;
}

static enum parser_error parse_r_b(struct parser *p)
{
	struct monster_race *r = parser_priv(p);
	int i;
	struct random dam;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	for (i = 0; i < MONSTER_BLOW_MAX; i++)
		if (!r->blow[i].method)
			break;
	if (i == MONSTER_BLOW_MAX)
		return PARSE_ERROR_TOO_MANY_ENTRIES;
	r->blow[i].method = find_blow_method(parser_getsym(p, "method"));
	if (!r_info_blow_method[r->blow[i].method])
		return PARSE_ERROR_UNRECOGNISED_BLOW;
	if (parser_hasval(p, "effect")) {
		r->blow[i].effect = find_blow_effect(parser_getsym(p, "effect"));
		if (!r_info_blow_effect[r->blow[i].effect])
			return PARSE_ERROR_INVALID_EFFECT;
	}
	if (parser_hasval(p, "damage")) {
		dam = parser_getrand(p, "damage");
		r->blow[i].d_dice = dam.dice;
		r->blow[i].d_side = dam.sides;
	}


	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_f(struct parser *p)
{
	struct monster_race *r = parser_priv(p);
	char *flags;
	char *s;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(r->flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_d(struct parser *p)
{
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->text = string_append(r->text, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_s(struct parser *p)
{
	struct monster_race *r = parser_priv(p);
	char *flags;
	char *s;
	int pct;
	int ret = PARSE_ERROR_NONE;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (1 == sscanf(s, "1_IN_%d", &pct)) {
			if (pct < 1 || pct > 100) {
				ret = PARSE_ERROR_INVALID_SPELL_FREQ;
				break;
			}
			r->freq_ranged = 100 / pct;
		} else if (1 == sscanf(s, "POW_%d", &pct)) {
			r->spell_power = pct;
		} else {
			if (grab_flag(r->spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
				ret = PARSE_ERROR_INVALID_FLAG;
				break;
			}
		}
		s = strtok(NULL, " |");
	}

	/* Add the "base monster" flags to the monster */
	if (r->base)
		rsf_union(r->spell_flags, r->base->spell_flags);

	mem_free(flags);
	return ret;
}

struct parser *init_parse_r(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index str name", parse_r_n);
	parser_reg(p, "T sym base", parse_r_t);
	parser_reg(p, "G char glyph", parse_r_g);
	parser_reg(p, "C sym color", parse_r_c);
	parser_reg(p, "I int speed rand hp int aaf int ac int sleep",
			   parse_r_i);
	parser_reg(p, "W int level int rarity int mana int mexp int faction", parse_r_w);
	parser_reg(p, "B sym method ?sym effect ?rand damage", parse_r_b);
	parser_reg(p, "F ?str flags", parse_r_f);
	parser_reg(p, "D str desc", parse_r_d);
	parser_reg(p, "S str spells", parse_r_s);
	return p;
}

static errr run_parse_r(struct parser *p)
{
	return parse_file(p, "monster");
}

static errr finish_parse_r(struct parser *p)
{
	struct monster_race *r, *n;

	r_info = mem_zalloc(sizeof(*r) * z_info->r_max);
	for (r = parser_priv(p); r; r = r->next) {
		if (r->ridx >= z_info->r_max)
			continue;
		memcpy(&r_info[r->ridx], r, sizeof(*r));
	}

	r = parser_priv(p);
	while (r) {
		n = r->next;
		mem_free(r);
		r = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_r(void)
{
	int ridx;

	for (ridx = 0; ridx < z_info->r_max - 1; ridx++) {
		struct monster_race *r = &r_info[ridx];

		string_free(r->text);
		string_free(r->name);
	}

	mem_free(r_info);
}

struct file_parser r_parser = {
	"monster",
	init_parse_r,
	run_parse_r,
	finish_parse_r,
	cleanup_r
};

static enum parser_error parse_b_n(struct parser *p)
{
	struct owner_type *h = parser_priv(p);
	struct owner_type *b = mem_alloc(sizeof *b);
	memset(b, 0, sizeof(*b));
	b->next = h;
	b->bidx = parser_getuint(p, "index");
	b->owner_name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, b);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_b_i(struct parser *p)
{
	struct owner_type *b = parser_priv(p);

	if (!b)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	b->owner_race = parser_getint(p, "race");
	b->max_cost = parser_getint(p, "gld");
	b->inflate = parser_getint(p, "inf");
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_b(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index str name", parse_b_n);
	parser_reg(p, "I int race int gld int inf", parse_b_i);
	return p;
}

static errr run_parse_b(struct parser *p)
{
	return parse_file(p, "shop_own");
}

static errr finish_parse_b(struct parser *p)
{
	struct owner_type *b, *n;
	int i = MAX_STORE_TYPES - 1;

	b_info = mem_zalloc(sizeof(*b) * MAX_STORE_TYPES * z_info->b_max);
	for (b = parser_priv(p); b; b = b->next) {
		if (b->bidx + i * z_info->b_max >= MAX_STORE_TYPES * z_info->b_max)
			continue;
		memcpy(&b_info[b->bidx + i * z_info->b_max], b, sizeof(*b));
		if (b->bidx == 0)
			i--;
	}

	b = parser_priv(p);
	while (b) {
		n = b->next;
		mem_free(b);
		b = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_b(void)
{
	int idx;
	for (idx = 0; idx < z_info->b_max * MAX_STORE_TYPES; idx++) {
		string_free(b_info[idx].owner_name);
	}
	mem_free(b_info);
}

struct file_parser b_parser = {
	"shop_own",
	init_parse_b,
	run_parse_b,
	finish_parse_b,
	cleanup_b
};

static enum parser_error parse_p_n(struct parser *p)
{
	struct player_race *h = parser_priv(p);
	struct player_race *r = mem_zalloc(sizeof *r);
	int i;

	/* Initialise values */
	for (i = 0; i < MAX_P_RES; i++)
		r->percent_res[i] = RES_LEVEL_BASE;

	r->next = h;
	r->ridx = parser_getuint(p, "index");
	r->name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, r);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_s(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_adj[A_STR] = parser_getint(p, "str");
	r->r_adj[A_DEX] = parser_getint(p, "dex");
	r->r_adj[A_CON] = parser_getint(p, "con");
	r->r_adj[A_INT] = parser_getint(p, "int");
	r->r_adj[A_WIS] = parser_getint(p, "wis");
	r->r_adj[A_CHR] = parser_getint(p, "chr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_r(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_DISARM] = parser_getint(p, "dis");
	r->r_skills[SKILL_DEVICE] = parser_getint(p, "dev");
	r->r_skills[SKILL_SAVE] = parser_getint(p, "sav");
	r->r_skills[SKILL_STEALTH] = parser_getint(p, "stl");
	r->r_skills[SKILL_SEARCH] = parser_getint(p, "srh");
	r->r_skills[SKILL_SEARCH_FREQUENCY] = parser_getint(p, "fos");
	r->r_skills[SKILL_TO_HIT_MELEE] = parser_getint(p, "thn");
	r->r_skills[SKILL_TO_HIT_BOW] = parser_getint(p, "thb");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_m(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->rx_skills[SKILL_DISARM] = parser_getint(p, "xdis");
	r->rx_skills[SKILL_DEVICE] = parser_getint(p, "xdev");
	r->rx_skills[SKILL_SAVE] = parser_getint(p, "xsav");
	r->rx_skills[SKILL_STEALTH] = parser_getint(p, "xstl");
	r->rx_skills[SKILL_SEARCH] = parser_getint(p, "xsrh");
	r->rx_skills[SKILL_SEARCH_FREQUENCY] = parser_getint(p, "xfos");
	r->rx_skills[SKILL_TO_HIT_MELEE] = parser_getint(p, "xthn");
	r->rx_skills[SKILL_TO_HIT_BOW] = parser_getint(p, "xthb");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_e(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->re_id = parser_getint(p, "id");
	r->re_mint = parser_getint(p, "mint");
	r->re_maxt = parser_getint(p, "maxt");
	r->re_skde = parser_getint(p, "skde");
	r->re_ac = parser_getint(p, "ac");
	r->re_bonus = parser_getint(p, "bonus");
	r->num_rings = parser_getint(p, "rings");
	/* Prevent out of bounds rings */
	if(r->num_rings > 2) r->num_rings = 2;
	r->clawed = (bool)parser_getint(p, "claws");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_x(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_mhp = parser_getint(p, "mhp");
	r->difficulty = parser_getint(p, "diff");
	r->infra = parser_getint(p, "infra");
	r->start_lev = parser_getint(p, "start_lev");
	r->hometown = parser_getint(p, "hometown");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_i(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->hist = parser_getint(p, "hist");
	r->b_age = parser_getint(p, "b-age");
	r->m_age = parser_getint(p, "m-age");
	r->name_m = parser_getint(p, "mname");
	r->name_f = parser_getint(p, "fname");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_h(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->m_b_ht = parser_getint(p, "mbht");
	r->m_m_ht = parser_getint(p, "mmht");
	r->f_b_ht = parser_getint(p, "fbht");
	r->f_m_ht = parser_getint(p, "fmht");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_w(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->m_b_wt = parser_getint(p, "mbwt");
	r->m_m_wt = parser_getint(p, "mmwt");
	r->f_b_wt = parser_getint(p, "fbwt");
	r->f_m_wt = parser_getint(p, "fmwt");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_f(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	char *flags;
	char *s;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(r->flags_obj, OF_SIZE, object_flags, s))
			break;
		s = strtok(NULL, " |");
	}
	mem_free(flags);
	return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_p_b(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	char *s = string_make(parser_getstr(p, "values"));
	char *t;
	int val, which = 0;
	assert(r);

	t = strtok(s, " |");
	while (t) {
		which = grab_value(t, player_resist_values,
						   N_ELEMENTS(player_resist_values), &val);
		if (which) {
			r->percent_res[which - 1] = RES_LEVEL_BASE - val;
			t = strtok(NULL, " |");
			continue;
		}
		break;
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_p_u(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	char *flags;
	char *s;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(r->pflags, PF_SIZE, player_info_flags, s))
			break;
		s = strtok(NULL, " |");
	}
	mem_free(flags);
	return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_p_c(struct parser *p)
{
	struct player_race *r = parser_priv(p);
	char *classes;
	char *s;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "classes"))
		return PARSE_ERROR_NONE;
	classes = string_make(parser_getstr(p, "classes"));
	s = strtok(classes, " |");
	while (s) {
		r->choice |= 1 << atoi(s);
		s = strtok(NULL, " |");
	}
	mem_free(classes);
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_p(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index str name", parse_p_n);
	parser_reg(p, "S int str int int int wis int dex int con int chr",
			   parse_p_s);
	parser_reg(p,
			   "R int dis int dev int sav int stl int srh int fos int thn int thb",
			   parse_p_r);
	parser_reg(p,
			   "M int xdis int xdev int xsav int xstl int xsrh int xfos int xthn int xthb",
			   parse_p_m);
	parser_reg(p, "E int id int mint int maxt int skde int ac int bonus int rings int claws",
			   parse_p_e);
	parser_reg(p,
			   "X int mhp int diff int infra int start_lev int hometown",
			   parse_p_x);
	parser_reg(p, "I int hist int b-age int m-age int mname int fname", parse_p_i);
	parser_reg(p, "H int mbht int mmht int fbht int fmht", parse_p_h);
	parser_reg(p, "W int mbwt int mmwt int fbwt int fmwt", parse_p_w);
	parser_reg(p, "F ?str flags", parse_p_f);
	parser_reg(p, "B ?str values", parse_p_b);
	parser_reg(p, "U ?str flags", parse_p_u);
	parser_reg(p, "C ?str classes", parse_p_c);
	return p;
}

static errr run_parse_p(struct parser *p)
{
	return parse_file(p, "p_race");
}

static errr finish_parse_p(struct parser *p)
{
	struct player_race *r, *n;

	p_info = mem_zalloc(sizeof(*r) * z_info->p_max);
	for (r = parser_priv(p); r; r = r->next) {
		if (r->ridx >= z_info->p_max)
			continue;
		memcpy(&p_info[r->ridx], r, sizeof(*r));
	}

	r = parser_priv(p);
	while (r) {
		n = r->next;
		mem_free(r);
		r = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_p(void)
{
	int idx;
	for (idx = 0; idx < z_info->p_max; idx++) {
		string_free((char *) p_info[idx].name);
	}
	mem_free(p_info);
}

struct file_parser p_parser = {
	"p_race",
	init_parse_p,
	run_parse_p,
	finish_parse_p,
	cleanup_p
};

static enum parser_error parse_ability_n(struct parser *p)
{
    struct innate_ability *h = parser_priv(p);
    struct innate_ability *a = mem_zalloc(sizeof *a);
    
    a->next = h;
    a->aidx = parser_getuint(p, "index");
    a->name = string_make(parser_getstr(p, "name"));
    parser_setpriv(p, a);
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_ability_d(struct parser *p)
{
    struct innate_ability *a = parser_priv(p);
    if(!a)
        return PARSE_ERROR_MISSING_RECORD_HEADER;
    a->desc = string_make(parser_getstr(p, "desc"));
    parser_setpriv(p, a);
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_ability_c(struct parser *p)
{
    struct innate_ability *a = parser_priv(p);
    if(!a)
        return PARSE_ERROR_MISSING_RECORD_HEADER;
    a->cast = string_make(parser_getstr(p, "cast"));
    parser_setpriv(p, a);
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_ability_s(struct parser *p)
{
    struct innate_ability *a = parser_priv(p);
    if(!a)
        return PARSE_ERROR_MISSING_RECORD_HEADER;
    a->level = parser_getint(p, "level");
    a->cost = parser_getint(p, "cost");
    a->stat = parser_getint(p, "stat");
    a->fail = parser_getint(p, "fail");
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_ability_u(struct parser *p)
{
    struct innate_ability *a = parser_priv(p);
    char *flag;
    char *s;
    
    if(!a)
        return PARSE_ERROR_MISSING_RECORD_HEADER;
    flag = string_make(parser_getstr(p, "flag"));
    s = strtok(flag, " |");
	while (s) {
	    if (grab_flag(a->pflags, PF_SIZE, player_info_flags, s))
	       break;
        s = strtok(NULL, " |");
    }
    mem_free(flag);
    return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_ability_m(struct parser *p)
{
    struct innate_ability *a = parser_priv(p);
    
    if(!a)
        return PARSE_ERROR_MISSING_RECORD_HEADER;
    if(!parser_hasval(p, "mini")) {
        a->mini = string_make("");
        return PARSE_ERROR_NONE;
    }
    
    a->mini = string_make(parser_getstr(p, "mini"));
    parser_setpriv(p, a);
    return PARSE_ERROR_NONE;
}
        

static struct parser *init_parse_ability(void)
{
    struct parser *p = parser_new();
    parser_setpriv(p, NULL);
    parser_reg(p, "V sym version", ignored);
    parser_reg(p, "N uint index str name", parse_ability_n);
    parser_reg(p, "D str desc", parse_ability_d);
    parser_reg(p, "C str cast", parse_ability_c);
    parser_reg(p, "S int level int cost int stat int fail", parse_ability_s);
    parser_reg(p, "U str flag", parse_ability_u);
    parser_reg(p, "M ?str mini", parse_ability_m);
    return p;
}

static errr run_parse_ability(struct parser *p)
{
    return parse_file(p, "ability");
}

static errr finish_parse_ability(struct parser *p)
{
    struct innate_ability *a, *n;
    
    ability_info = mem_zalloc(sizeof(*a) * z_info->ability_max);
    for (a = parser_priv(p); a; a = a->next) {
        if (a->aidx >= z_info->ability_max)
            continue;
        memcpy(&ability_info[a->aidx], a, sizeof(*a));
    }
    
    a = parser_priv(p);
    while(a) {
        n = a->next;
        mem_free(a);
        a = n;
    }
    
    parser_destroy(p);
    return 0;
}
static void cleanup_ability(void)
{
    int idx;
    for(idx = 0; idx < z_info->ability_max; idx++) {
        string_free((char*) ability_info[idx].name);
        string_free((char*) ability_info[idx].desc);
        string_free((char*) ability_info[idx].cast);
        string_free((char*) ability_info[idx].mini);
    }
    mem_free(ability_info);
}

struct file_parser ability_parser = {
    "ability",
    init_parse_ability,
    run_parse_ability,
    finish_parse_ability,
    cleanup_ability
};

static enum parser_error parse_c_n(struct parser *p)
{
	struct player_class *h = parser_priv(p);
	struct player_class *c = mem_zalloc(sizeof *c);
	int i;
	c->cidx = parser_getuint(p, "index");
	c->name = string_make(parser_getstr(p, "name"));
	c->next = h;
	parser_setpriv(p, c);

	/* Initialise spells */
	for (i = 0; i < PY_MAX_SPELLS; i++) {
		c->magic.info[i].index = 0;
		c->magic.info[i].slevel = 0;
		c->magic.info[i].smana = 0;
		c->magic.info[i].sfail = 0;
		c->magic.info[i].sexp = 0;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_c_s(struct parser *p)
{
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	c->c_adj[A_STR] = parser_getint(p, "str");
	c->c_adj[A_INT] = parser_getint(p, "int");
	c->c_adj[A_WIS] = parser_getint(p, "wis");
	c->c_adj[A_DEX] = parser_getint(p, "dex");
	c->c_adj[A_CON] = parser_getint(p, "con");
	c->c_adj[A_CHR] = parser_getint(p, "chr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_c_c(struct parser *p)
{
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_DISARM] = parser_getint(p, "dis");
	c->c_skills[SKILL_DEVICE] = parser_getint(p, "dev");
	c->c_skills[SKILL_SAVE] = parser_getint(p, "sav");
	c->c_skills[SKILL_STEALTH] = parser_getint(p, "stl");
	c->c_skills[SKILL_SEARCH] = parser_getint(p, "srh");
	c->c_skills[SKILL_SEARCH_FREQUENCY] = parser_getint(p, "fos");
	c->c_skills[SKILL_TO_HIT_MELEE] = parser_getint(p, "thm");
	c->c_skills[SKILL_TO_HIT_BOW] = parser_getint(p, "thb");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_c_x(struct parser *p)
{
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->cx_skills[SKILL_DISARM] = parser_getint(p, "dis");
	c->cx_skills[SKILL_DEVICE] = parser_getint(p, "dev");
	c->cx_skills[SKILL_SAVE] = parser_getint(p, "sav");
	c->cx_skills[SKILL_STEALTH] = parser_getint(p, "stl");
	c->cx_skills[SKILL_SEARCH] = parser_getint(p, "srh");
	c->cx_skills[SKILL_SEARCH_FREQUENCY] = parser_getint(p, "fos");
	c->cx_skills[SKILL_TO_HIT_MELEE] = parser_getint(p, "thm");
	c->cx_skills[SKILL_TO_HIT_BOW] = parser_getint(p, "thb");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_c_i(struct parser *p)
{
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_mhp = parser_getint(p, "mhp");
	c->sense_base = parser_getint(p, "sense-base");
	c->alignment = parser_getint(p, "alignment");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_c_a(struct parser *p)
{
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->max_1 = parser_getint(p, "max_1");
	c->max_50 = parser_getint(p, "max_50");
	c->penalty = parser_getint(p, "penalty");
	c->max_penalty = parser_getint(p, "max_penalty");
	c->bonus = parser_getint(p, "bonus");
	c->max_bonus = parser_getint(p, "max_bonus");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_c_m(struct parser *p)
{
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->magic.spell_book = parser_getuint(p, "book");
	c->magic.spell_stat = parser_getuint(p, "stat");
	c->magic.spell_realm = parser_getuint(p, "realm");
	c->magic.spell_first = parser_getuint(p, "first");
	c->magic.spell_weight1 = parser_getuint(p, "weight1");
	c->magic.spell_weight2 = parser_getuint(p, "weight2");
	c->magic.spell_number = parser_getuint(p, "number");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_c_f(struct parser *p)
{
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->magic.book_start_index[0] = parser_getuint(p, "first0");
	c->magic.book_start_index[1] = parser_getuint(p, "first1");
	c->magic.book_start_index[2] = parser_getuint(p, "first2");
	c->magic.book_start_index[3] = parser_getuint(p, "first3");
	c->magic.book_start_index[4] = parser_getuint(p, "first4");
	c->magic.book_start_index[5] = parser_getuint(p, "first5");
	c->magic.book_start_index[6] = parser_getuint(p, "first6");
	c->magic.book_start_index[7] = parser_getuint(p, "first7");
	c->magic.book_start_index[8] = parser_getuint(p, "first8");
	c->magic.book_start_index[9] = parser_getuint(p, "first9");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_c_b(struct parser *p)
{
	struct player_class *c = parser_priv(p);
	unsigned int spell;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	spell = parser_getuint(p, "spell");
	if (spell >= PY_MAX_SPELLS)
		return PARSE_ERROR_OUT_OF_BOUNDS;
	c->magic.info[spell].index = parser_getuint(p, "index");
	c->magic.info[spell].slevel = parser_getuint(p, "level");
	c->magic.info[spell].smana = parser_getint(p, "mana");
	c->magic.info[spell].sfail = parser_getint(p, "fail");
	c->magic.info[spell].sexp = parser_getint(p, "exp");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_c_e(struct parser *p)
{
	struct player_class *c = parser_priv(p);
	int i;
	int tval, sval;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	sval = lookup_sval(tval, parser_getsym(p, "sval"));
	if (sval < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	for (i = 0; i <= MAX_START_ITEMS; i++)
		if (!c->start_items[i].min)
			break;
	if (i > MAX_START_ITEMS)
		return PARSE_ERROR_TOO_MANY_ENTRIES;
	c->start_items[i].kind = objkind_get(tval, sval);
	c->start_items[i].min = parser_getuint(p, "min");
	c->start_items[i].max = parser_getuint(p, "max");
	/* XXX: MAX_ITEM_STACK? */
	if (c->start_items[i].min > 99 || c->start_items[i].max > 99)
		return PARSE_ERROR_INVALID_ITEM_NUMBER;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_c_u(struct parser *p)
{
	struct player_class *c = parser_priv(p);
	char *flags;
	char *s;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(c->pflags, PF_SIZE, player_info_flags, s))
			break;
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_c_l(struct parser *p)
{
	struct player_class *c = parser_priv(p);
	char *flags;
	char *s;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(c->specialties, PF_SIZE, player_info_flags, s))
			break;
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_c_t(struct parser *p)
{
	struct player_class *c = parser_priv(p);
	int i;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	for (i = 0; i < PY_MAX_LEVEL / 5; i++) {
		if (!c->title[i]) {
			c->title[i] = string_make(parser_getstr(p, "title"));
			break;
		}
	}

	if (i >= PY_MAX_LEVEL / 5)
		return PARSE_ERROR_TOO_MANY_ENTRIES;
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_c(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index str name", parse_c_n);
	parser_reg(p, "S int str int int int wis int dex int con int chr",
			   parse_c_s);
	parser_reg(p,
			   "C int dis int dev int sav int stl int srh int fos int thm int thb",
			   parse_c_c);
	parser_reg(p,
			   "X int dis int dev int sav int stl int srh int fos int thm int thb",
			   parse_c_x);
	parser_reg(p, "I int mhp int sense-base int alignment", parse_c_i);
	parser_reg(p,
			   "A int max_1 int max_50 int penalty int max_penalty int bonus int max_bonus",
			   parse_c_a);
	parser_reg(p,
			   "M uint book uint stat uint realm uint first uint weight1 uint weight2 uint number",
			   parse_c_m);
	parser_reg(p,
			   "F uint first0 uint first1 uint first2 uint first3 uint first4 uint first5 uint first6 uint first7 uint first8 uint first9",
			   parse_c_f);
	parser_reg(p,
			   "B uint spell uint index uint level int mana int fail int exp",
			   parse_c_b);
	parser_reg(p, "E sym tval sym sval uint min uint max", parse_c_e);
	parser_reg(p, "U ?str flags", parse_c_u);
	parser_reg(p, "L ?str flags", parse_c_l);
	parser_reg(p, "T str title", parse_c_t);
	return p;
}

static errr run_parse_c(struct parser *p)
{
	return parse_file(p, "p_class");
}

static errr finish_parse_c(struct parser *p)
{
	struct player_class *c, *n;

	c_info = mem_zalloc(sizeof(*c) * z_info->c_max);
	for (c = parser_priv(p); c; c = c->next) {
		if (c->cidx >= z_info->c_max)
			continue;
		memcpy(&c_info[c->cidx], c, sizeof(*c));
	}

	c = parser_priv(p);
	while (c) {
		n = c->next;
		mem_free(c);
		c = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_c(void)
{
	int idx, i;
	for (idx = 0; idx < z_info->c_max; idx++) {
		for (i = 0; i < PY_MAX_LEVEL / 5; i++) {
			string_free((char *) c_info[idx].title[i]);
		}
		string_free((char *) c_info[idx].name);
	}
	mem_free(c_info);
}

struct file_parser c_parser = {
	"p_class",
	init_parse_c,
	run_parse_c,
	finish_parse_c,
	cleanup_c
};

static enum parser_error parse_mark_n(struct parser *p)
{
    struct player_cutiemark *h = parser_priv(p);
    struct player_cutiemark *cm = mem_zalloc(sizeof *cm);
    int i;
    
    /* Initialise values */
    for (i = 0; i < MAX_P_RES; i++)
        cm->percent_res[i] = RES_LEVEL_BASE;
    
    cm->next = h;
    cm->cmidx = parser_getuint(p, "index");
    cm->name = string_make(parser_getstr(p, "name"));
    parser_setpriv(p, cm);
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_mark_s(struct parser *p)
{
    struct player_cutiemark *cm = parser_priv(p);

	if (!cm)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	cm->cm_adj[A_STR] = parser_getint(p, "str");
	cm->cm_adj[A_INT] = parser_getint(p, "int");
	cm->cm_adj[A_WIS] = parser_getint(p, "wis");
	cm->cm_adj[A_DEX] = parser_getint(p, "dex");
	cm->cm_adj[A_CON] = parser_getint(p, "con");
	cm->cm_adj[A_CHR] = parser_getint(p, "chr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mark_i(struct parser *p)
{
    struct player_cutiemark *cm = parser_priv(p);
    if(!cm)
        return PARSE_ERROR_MISSING_RECORD_HEADER;
        
    cm->hist = parser_getint(p, "hist");
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_mark_u(struct parser *p)
{
    struct player_cutiemark *cm = parser_priv(p);
    char *flags;
    char *s;
    
    if(!cm)
        return PARSE_ERROR_MISSING_RECORD_HEADER;
    if(!parser_hasval(p, "flags"))
        return PARSE_ERROR_NONE;
    flags = string_make(parser_getstr(p, "flags"));
    s = strtok(flags, " |");
    while (s) {
        if(grab_flag(cm->pflags, PF_SIZE, player_info_flags, s))
            break;
        s = strtok(NULL, " |");
    }
    mem_free(flags);
    return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_mark_b(struct parser *p)
{
    struct player_cutiemark *cm = parser_priv(p);
    char *s = string_make(parser_getstr(p, "values"));
    char *t;
    int val, which = 0;
    assert(cm);
    
    t = strtok(s, " |");
    while (t) {
        which = grab_value(t, player_resist_values,
                           N_ELEMENTS(player_resist_values), &val);
        if (which) {
            cm->percent_res[which-1] = RES_LEVEL_BASE - val;
            t = strtok(NULL, " |");
            continue;
        }
        break;
    }
    mem_free(s);
    return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_mark_d(struct parser *p)
{
    struct player_cutiemark *cm = parser_priv(p);
    
    assert(cm);
    
    cm->desc = string_make(parser_getstr(p, "desc"));
    
    return PARSE_ERROR_NONE;
}

struct parser *init_parse_mark(void)
{
    struct parser *p = parser_new();
    parser_setpriv(p, NULL);
    parser_reg(p, "V sym version", ignored);
    parser_reg(p, "N uint index str name", parse_mark_n);
    parser_reg(p, "S int str int int int wis int dex int con int chr",
               parse_mark_s);
    parser_reg(p, "I int hist", parse_mark_i);
    parser_reg(p, "D str desc", parse_mark_d);
    parser_reg(p, "U ?str flags", parse_mark_u);
    parser_reg(p, "B ?str values", parse_mark_b);
    return p;
}

static errr run_parse_mark(struct parser *p)
{
    return parse_file(p, "p_mark");
}

static errr finish_parse_mark(struct parser *p)
{
    struct player_cutiemark *cm, *n;
    
    cm_info = mem_zalloc(sizeof(*cm) * z_info->cm_max);
    for (cm = parser_priv(p); cm; cm = cm->next) {
        if (cm->cmidx >= z_info->cm_max)
            continue;
        memcpy(&cm_info[cm->cmidx], cm, sizeof(*cm));
    }
    
    cm = parser_priv(p);
    while (cm) {
        n = cm->next;
        mem_free(cm);
        cm = n;
    }
    
    parser_destroy(p);
    return 0;
}

static void cleanup_mark(void)
{
    int idx;

    for (idx = 0; idx < z_info->cm_max; idx++) {
    	if(cm_info[idx].name)
    	{
    		assert(cm_info[idx].name);
    		string_free((char *) cm_info[idx].name);
    	}
    	if(cm_info[idx].desc)
    	{
    		assert(cm_info[idx].desc);
    		string_free((char *) cm_info[idx].desc);
    	}
    }
    mem_free(cm_info);
}
    
struct file_parser mark_parser = {
    "p_mark",
    init_parse_mark,
    run_parse_mark,
    finish_parse_mark,
    cleanup_mark
};

static enum parser_error parse_v_n(struct parser *p)
{
	struct vault *h = parser_priv(p);
	struct vault *v = mem_zalloc(sizeof *v);

	v->vidx = parser_getuint(p, "index");
	v->name = string_make(parser_getstr(p, "name"));
	v->next = h;
	parser_setpriv(p, v);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_v_x(struct parser *p)
{
	struct vault *v = parser_priv(p);
	int max_lev;

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	v->typ = parser_getuint(p, "type");
	v->rat = parser_getint(p, "rating");
	v->hgt = parser_getuint(p, "height");
	v->hgt = 0;
	v->wid = parser_getuint(p, "width");
	v->wid = 0;
	v->min_lev = parser_getuint(p, "min_lev");
	max_lev = parser_getuint(p, "max_lev");
	v->max_lev = max_lev ? max_lev : MAX_DEPTH;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_v_d(struct parser *p)
{
	struct vault *v = parser_priv(p);

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	v->text = string_append(v->text, parser_getstr(p, "text"));
	if (v->hgt == 0)
		v->wid = strlen(v->text);
	if (strlen(v->text) % v->wid) 
		return PARSE_ERROR_FIELD_TOO_LONG;
	v->hgt++;
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_v(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index str name", parse_v_n);
	parser_reg(p,
			   "X uint type int rating uint height uint width uint min_lev uint max_lev",
			   parse_v_x);
	parser_reg(p, "D str text", parse_v_d);
	return p;
}

static errr run_parse_v(struct parser *p)
{
	return parse_file(p, "vault");
}

static errr finish_parse_v(struct parser *p)
{
	struct vault *v, *n;

	v_info = mem_zalloc(sizeof(*v) * z_info->v_max);
	for (v = parser_priv(p); v; v = v->next) {
		if (v->vidx >= z_info->v_max)
			continue;
;
		memcpy(&v_info[v->vidx], v, sizeof(*v));
	}

	v = parser_priv(p);
	while (v) {
		n = v->next;
		mem_free(v);
		v = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_v(void)
{
	int idx;
	for (idx = 0; idx < z_info->v_max; idx++) {
		mem_free(v_info[idx].name);
		mem_free(v_info[idx].text);
	}
	mem_free(v_info);
}

struct file_parser v_parser = {
	"vault",
	init_parse_v,
	run_parse_v,
	finish_parse_v,
	cleanup_v
};

static enum parser_error parse_t_n(struct parser *p)
{
	struct vault *h = parser_priv(p);
	struct vault *v = mem_zalloc(sizeof *v);

	v->vidx = parser_getuint(p, "index");
	v->name = string_make(parser_getstr(p, "name"));
	v->next = h;
	parser_setpriv(p, v);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_t_m(struct parser *p)
{
	struct vault *v = parser_priv(p);

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	v->message = string_make(parser_getstr(p, "feel"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_t_d(struct parser *p)
{
	struct vault *v = parser_priv(p);

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	v->text = string_append(v->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_t(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index str name", parse_t_n);
	parser_reg(p, "M str feel", parse_t_m);
	parser_reg(p, "D str text", parse_t_d);
	return p;
}

static errr run_parse_t(struct parser *p)
{
	return parse_file(p, "themed");
}

static errr finish_parse_t(struct parser *p)
{
	struct vault *v, *n;

	t_info = mem_zalloc(sizeof(*v) * z_info->t_max);
	for (v = parser_priv(p); v; v = v->next) {
		if (v->vidx >= z_info->t_max)
			continue;
		memcpy(&t_info[v->vidx], v, sizeof(*v));
	}

	v = parser_priv(p);
	while (v) {
		n = v->next;
		mem_free(v);
		v = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_t(void)
{
	int idx;
	for (idx = 0; idx < z_info->t_max; idx++) {
		mem_free(t_info[idx].name);
		mem_free(t_info[idx].text);
		mem_free(t_info[idx].message);
	}
	mem_free(t_info);
}

struct file_parser t_parser = {
	"themed",
	init_parse_t,
	run_parse_t,
	finish_parse_t,
	cleanup_t
};

static enum parser_error parse_h_n(struct parser *p)
{
	struct history *oh = parser_priv(p);
	struct history *h = mem_zalloc(sizeof *h);
	
	h->chart = parser_getint(p, "chart");
	h->next = parser_getint(p, "next");
	h->roll = parser_getint(p, "roll");
	h->bonus = parser_getint(p, "bonus");
	h->nextp = oh;
	h->hidx = oh ? oh->hidx + 1 : 0;
	parser_setpriv(p, h);
	
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_h_d(struct parser *p)
{
	struct history *h = parser_priv(p);

	if (!h)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	h->text = string_append(h->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_h(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N int chart int next int roll int bonus", parse_h_n);
	parser_reg(p, "D str text", parse_h_d);
	return p;
}

static errr run_parse_h(struct parser *p)
{
	return parse_file(p, "p_hist");
}

static errr finish_parse_h(struct parser *p)
{
	struct history *h, *n;

	h_info = mem_zalloc(sizeof(*h) * z_info->h_max);
	for (h = parser_priv(p); h; h = h->nextp) {
		if (h->hidx >= z_info->h_max) {
			printf("warning: skipping bad history %d\n", h->hidx);
			continue;
		}
		memcpy(&h_info[h->hidx], h, sizeof(*h));
	}

	h = parser_priv(p);
	while (h) {
		n = h->nextp;
		mem_free(h);
		h = n;
	}

	parser_destroy(p);
	return PARSE_ERROR_NONE;
}

static void cleanup_h(void)
{
	int idx;
	for (idx = 0; idx < z_info->h_max; idx++) {
		mem_free(h_info[idx].text);
	}
	mem_free(h_info);
}

struct file_parser h_parser = {
	"p_hist",
	init_parse_h,
	run_parse_h,
	finish_parse_h,
	cleanup_h
};

static enum parser_error parse_flavor_n(struct parser *p)
{
	struct flavor *h = parser_priv(p);
	struct flavor *f = mem_zalloc(sizeof *f);

	f->next = h;
	f->fidx = parser_getuint(p, "index");
	f->tval = tval_find_idx(parser_getsym(p, "tval"));
	/* assert(f->tval); */
	if (parser_hasval(p, "sval"))
		f->sval = lookup_sval(f->tval, parser_getsym(p, "sval"));
	else
		f->sval = SV_UNKNOWN;
	parser_setpriv(p, f);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_flavor_g(struct parser *p)
{
	struct flavor *f = parser_priv(p);
	int d_attr;
	const char *attr;

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	f->d_char = parser_getchar(p, "glyph");
	attr = parser_getsym(p, "attr");
	if (strlen(attr) == 1) {
		d_attr = color_char_to_attr(attr[0]);
	} else {
		d_attr = color_text_to_attr(attr);
	}
	if (d_attr < 0)
		return PARSE_ERROR_GENERIC;
	f->d_attr = d_attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_flavor_d(struct parser *p)
{
	struct flavor *f = parser_priv(p);

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f->text = string_append(f->text, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_flavor(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index sym tval ?sym sval", parse_flavor_n);
	parser_reg(p, "G char glyph sym attr", parse_flavor_g);
	parser_reg(p, "D str desc", parse_flavor_d);
	return p;
}

static errr run_parse_flavor(struct parser *p)
{
	return parse_file(p, "flavor");
}

static errr finish_parse_flavor(struct parser *p)
{
	struct flavor *f, *n;

	flavor_info = mem_zalloc(z_info->flavor_max * sizeof(*f));

	for (f = parser_priv(p); f; f = f->next) {
		if (f->fidx >= z_info->flavor_max)
			continue;
		memcpy(&flavor_info[f->fidx], f, sizeof(*f));
	}

	f = parser_priv(p);
	while (f) {
		n = f->next;
		mem_free(f);
		f = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_flavor(void)
{
	int idx;
	for (idx = 0; idx < z_info->flavor_max; idx++) {
		/* Hack - scrolls get randomly-generated names */
		if (flavor_info[idx].tval != TV_SCROLL)
			mem_free(flavor_info[idx].text);
	}
	mem_free(flavor_info);
}

struct file_parser flavor_parser = {
	"flavor",
	init_parse_flavor,
	run_parse_flavor,
	finish_parse_flavor,
	cleanup_flavor
};

static enum parser_error parse_s_n(struct parser *p)
{
	struct spell *s = mem_zalloc(sizeof *s);
	s->next = parser_priv(p);
	s->sidx = parser_getuint(p, "index");
	s->name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, s);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_s_d(struct parser *p)
{
	struct spell *s = parser_priv(p);

	if (!s)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	s->text = string_append(s->text, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_s(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index str name", parse_s_n);
	parser_reg(p, "D str desc", parse_s_d);
	return p;
}

static errr run_parse_s(struct parser *p)
{
	return parse_file(p, "spell");
}

static errr finish_parse_s(struct parser *p)
{
	struct spell *s, *n;

	s_info = mem_zalloc(z_info->s_max * sizeof(*s_info));
	for (s = parser_priv(p); s; s = s->next) {
		if (s->sidx >= z_info->s_max)
			continue;
		memcpy(&s_info[s->sidx], s, sizeof(*s));
	}

	s = parser_priv(p);
	while (s) {
		n = s->next;
		mem_free(s);
		s = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_s(void)
{
	int idx;
	for (idx = 0; idx < z_info->s_max; idx++) {
		string_free(s_info[idx].name);
		mem_free(s_info[idx].text);
	}
	mem_free(s_info);
}

static struct file_parser s_parser = {
	"spell",
	init_parse_s,
	run_parse_s,
	finish_parse_s,
	cleanup_s
};

/* Initialise hints */
static enum parser_error parse_hint(struct parser *p)
{
	struct hint *h = parser_priv(p);
	struct hint *new = mem_zalloc(sizeof *new);

	new->hint = string_make(parser_getstr(p, "text"));
	new->next = h;

	parser_setpriv(p, new);
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_hints(void)
{
	struct parser *p = parser_new();
	parser_reg(p, "H str text", parse_hint);
	return p;
}

static errr run_parse_hints(struct parser *p)
{
	return parse_file(p, "hints");
}

static errr finish_parse_hints(struct parser *p)
{
	hints = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_hints(void)
{
	struct hint *h, *next;

	h = hints;
	while (h) {
		next = h->next;
		string_free(h->hint);
		mem_free(h);
		h = next;
	}
}

static struct file_parser hints_parser = {
	"hints",
	init_parse_hints,
	run_parse_hints,
	finish_parse_hints,
	cleanup_hints
};


/* Initialise monster pain messages */
static enum parser_error parse_mp_n(struct parser *p)
{
	struct monster_pain *h = parser_priv(p);
	struct monster_pain *mp = mem_zalloc(sizeof *mp);
	mp->next = h;
	mp->pain_idx = parser_getuint(p, "index");
	parser_setpriv(p, mp);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mp_m(struct parser *p)
{
	struct monster_pain *mp = parser_priv(p);
	int i;

	if (!mp)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	for (i = 0; i < 7; i++)
		if (!mp->messages[i])
			break;
	if (i == 7)
		return PARSE_ERROR_TOO_MANY_ENTRIES;
	mp->messages[i] = string_make(parser_getstr(p, "message"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_mp(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "N uint index", parse_mp_n);
	parser_reg(p, "M str message", parse_mp_m);
	return p;
}

static errr run_parse_mp(struct parser *p)
{
	return parse_file(p, "pain");
}

static errr finish_parse_mp(struct parser *p)
{
	struct monster_pain *mp, *n;

	/* scan the list for the max id */
	z_info->mp_max = 0;
	mp = parser_priv(p);
	while (mp) {
		if (mp->pain_idx > z_info->mp_max)
			z_info->mp_max = mp->pain_idx;
		mp = mp->next;
	}

	/* allocate the direct access list and copy the data to it */
	pain_messages = mem_zalloc((z_info->mp_max + 1) * sizeof(*mp));
	for (mp = parser_priv(p); mp; mp = n) {
		memcpy(&pain_messages[mp->pain_idx], mp, sizeof(*mp));
		n = mp->next;
		if (n)
			pain_messages[mp->pain_idx].next = &pain_messages[n->pain_idx];
		else
			pain_messages[mp->pain_idx].next = NULL;
		mem_free(mp);
	}
	z_info->mp_max += 1;

	parser_destroy(p);
	return 0;
}

static void cleanup_mp(void)
{
	int idx, i;
	for (idx = 0; idx < z_info->mp_max; idx++) {
		for (i = 0; i < 7; i++) {
			string_free((char *) pain_messages[idx].messages[i]);
		}
	}
	mem_free(pain_messages);
}

struct file_parser mp_parser = {
	"pain messages",
	init_parse_mp,
	run_parse_mp,
	finish_parse_mp,
	cleanup_mp
};


/**
 * Hack -- Objects sold in the stores -- by tval/sval pair.
 * Note code for initializing the stores, below.
 */
static byte store_table[MAX_STORE_TYPES][STORE_CHOICES][2] = {
	{
	 /* General Store. */

	 {TV_FOOD, SV_FOOD_RATION},
	 {TV_FOOD, SV_FOOD_RATION},
	 {TV_FOOD, SV_FOOD_RATION},
	 {TV_FOOD, SV_FOOD_RATION},
	 {TV_FOOD, SV_FOOD_BISCUIT},
	 {TV_FOOD, SV_FOOD_BISCUIT},
	 {TV_FOOD, SV_FOOD_JERKY},
	 {TV_FOOD, SV_FOOD_JERKY},

	 {TV_FOOD, SV_FOOD_PINT_OF_WINE},
	 {TV_FOOD, SV_FOOD_PINT_OF_ALE},
	 {TV_LIGHT, SV_LIGHT_TORCH},
	 {TV_LIGHT, SV_LIGHT_TORCH},
	 {TV_LIGHT, SV_LIGHT_TORCH},
	 {TV_LIGHT, SV_LIGHT_TORCH},
	 {TV_LIGHT, SV_LIGHT_LANTERN},
	 {TV_LIGHT, SV_LIGHT_LANTERN},

	 {TV_FLASK, 0},
	 {TV_FLASK, 0},
	 {TV_FLASK, 0},
	 {TV_FLASK, 0},
	 {TV_FLASK, 0},
	 {TV_FLASK, 0},
	 {TV_SPIKE, 0},
	 {TV_SPIKE, 0},

	 {TV_SHOT, SV_AMMO_NORMAL},
	 {TV_ARROW, SV_AMMO_NORMAL},
	 {TV_BOLT, SV_AMMO_NORMAL},
	 {TV_DIGGING, SV_SHOVEL},
	 {TV_DIGGING, SV_PICK},
	 {TV_CLOAK, SV_CLOAK},
	 {TV_CLOAK, SV_CLOAK},
	 {TV_CLOAK, SV_CLOAK}
	 },

	{
	 /* Armoury */

	 {TV_BOOTS, SV_PAIR_OF_SOFT_LEATHER_BOOTS},
	 {TV_BOOTS, SV_PAIR_OF_SOFT_LEATHER_BOOTS},
	 {TV_BOOTS, SV_PAIR_OF_HARD_LEATHER_BOOTS},
	 {TV_BOOTS, SV_PAIR_OF_HARD_LEATHER_BOOTS},
	 {TV_HELM, SV_HARD_LEATHER_CAP},
	 {TV_HELM, SV_HARD_LEATHER_CAP},
	 {TV_HELM, SV_METAL_CAP},
	 {TV_HELM, SV_IRON_HELM},

	 {TV_SOFT_ARMOR, SV_ROBE},
	 {TV_SOFT_ARMOR, SV_ROBE},
	 {TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR},
	 {TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR},
	 {TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR},
	 {TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR},
	 {TV_SOFT_ARMOR, SV_HARD_STUDDED_LEATHER},
	 {TV_SOFT_ARMOR, SV_HARD_STUDDED_LEATHER},

	 {TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL},
	 {TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL},
	 {TV_HARD_ARMOR, SV_METAL_SCALE_MAIL},
	 {TV_HARD_ARMOR, SV_CHAIN_MAIL},
	 {TV_HARD_ARMOR, SV_DOUBLE_CHAIN_MAIL},
	 {TV_HARD_ARMOR, SV_AUGMENTED_CHAIN_MAIL},
	 {TV_HARD_ARMOR, SV_BAR_CHAIN_MAIL},
	 {TV_HARD_ARMOR, SV_DOUBLE_CHAIN_MAIL},

	 {TV_HARD_ARMOR, SV_METAL_BRIGANDINE_ARMOUR},
	 {TV_GLOVES, SV_SET_OF_LEATHER_GLOVES},
	 {TV_GLOVES, SV_SET_OF_LEATHER_GLOVES},
	 {TV_GLOVES, SV_SET_OF_MAIL_GAUNTLETS},
	 {TV_SHIELD, SV_WICKER_SHIELD},
	 {TV_SHIELD, SV_SMALL_LEATHER_SHIELD},
	 {TV_SHIELD, SV_LARGE_LEATHER_SHIELD},
	 {TV_SHIELD, SV_SMALL_METAL_SHIELD}
	 },

	{
	 /* Weaponsmith */

	 {TV_SWORD, SV_DAGGER},
	 {TV_SWORD, SV_DAGGER},
	 {TV_SWORD, SV_MAIN_GAUCHE},
	 {TV_SWORD, SV_RAPIER},
	 {TV_SWORD, SV_SMALL_SWORD},
	 {TV_SWORD, SV_SHORT_SWORD},
	 {TV_SWORD, SV_SABRE},
	 {TV_SWORD, SV_CUTLASS},

	 {TV_SWORD, SV_BROAD_SWORD},
	 {TV_SWORD, SV_LONG_SWORD},
	 {TV_SWORD, SV_SCIMITAR},
	 {TV_SWORD, SV_KATANA},
	 {TV_SWORD, SV_BASTARD_SWORD},
	 {TV_SWORD, SV_TWO_HANDED_SWORD},
	 {TV_POLEARM, SV_SPEAR},
	 {TV_POLEARM, SV_TRIDENT},

	 {TV_POLEARM, SV_PIKE},
	 {TV_POLEARM, SV_BEAKED_AXE},
	 {TV_POLEARM, SV_BROAD_AXE},
	 {TV_POLEARM, SV_DART},
	 {TV_POLEARM, SV_BATTLE_AXE},
	 {TV_BOW, SV_SLING},
	 {TV_BOW, SV_SLING},
	 {TV_BOW, SV_SHORT_BOW},

	 {TV_BOW, SV_LONG_BOW},
	 {TV_BOW, SV_LIGHT_XBOW},
	 {TV_SHOT, SV_AMMO_NORMAL},
	 {TV_SHOT, SV_AMMO_NORMAL},
	 {TV_ARROW, SV_AMMO_NORMAL},
	 {TV_ARROW, SV_AMMO_NORMAL},
	 {TV_BOLT, SV_AMMO_NORMAL},
	 {TV_BOLT, SV_AMMO_NORMAL},
	 },

	{
	 /* Temple. */

	 {TV_HAFTED, SV_WHIP},
	 {TV_HAFTED, SV_QUARTERSTAFF},
	 {TV_HAFTED, SV_MACE},
	 {TV_HAFTED, SV_MACE},
	 {TV_SCROLL, SV_SCROLL_PROTECTION_FROM_EVIL},
	 {TV_HAFTED, SV_WAR_HAMMER},
	 {TV_HAFTED, SV_WAR_HAMMER},
	 {TV_HAFTED, SV_MORNING_STAR},

	 {TV_HAFTED, SV_FLAIL},
	 {TV_HAFTED, SV_FLAIL},
	 {TV_HAFTED, SV_LEAD_FILLED_MACE},
	 {TV_SCROLL, SV_SCROLL_REMOVE_CURSE},
	 {TV_SCROLL, SV_SCROLL_BLESSING},
	 {TV_SCROLL, SV_SCROLL_HOLY_CHANT},
	 {TV_POTION, SV_POTION_BOLDNESS},
	 {TV_POTION, SV_POTION_HEROISM},

	 {TV_POTION, SV_POTION_CURE_LIGHT},
	 {TV_SCROLL, SV_SCROLL_RECHARGING},
	 {TV_POTION, SV_POTION_RESIST_ACID_ELEC},
	 {TV_POTION, SV_POTION_CURE_CRITICAL},
	 {TV_POTION, SV_POTION_CURE_POISON},
	 {TV_POTION, SV_POTION_SLOW_POISON},
	 {TV_POTION, SV_POTION_RESIST_HEAT_COLD},
	 {TV_POTION, SV_POTION_RESTORE_EXP},

	 {TV_POTION, SV_POTION_CURE_LIGHT},
	 {TV_POTION, SV_POTION_CURE_SERIOUS},
	 {TV_POTION, SV_POTION_CURE_SERIOUS},
	 {TV_POTION, SV_POTION_CURE_CRITICAL},
	 {TV_POTION, SV_POTION_CURE_CRITICAL},
	 {TV_POTION, SV_POTION_RESTORE_EXP},
	 {TV_POTION, SV_POTION_RESTORE_EXP},
	 {TV_POTION, SV_POTION_RESTORE_EXP}
	 },

	{
	 /* Alchemy shop.  All the general-purpose scrolls and potions. */

	 {TV_SCROLL, SV_SCROLL_WORD_OF_RECALL},
	 {TV_SCROLL, SV_SCROLL_WORD_OF_RECALL},
	 {TV_SCROLL, SV_SCROLL_WORD_OF_RECALL},
	 {TV_SCROLL, SV_SCROLL_WORD_OF_RECALL},
	 {TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_HIT},
	 {TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_DAM},
	 {TV_SCROLL, SV_SCROLL_ENCHANT_ARMOR},
	 {TV_SCROLL, SV_SCROLL_TELEPORT},

	 {TV_SCROLL, SV_SCROLL_TELEPORT_LEVEL},
	 {TV_SCROLL, SV_SCROLL_IDENTIFY},
	 {TV_SCROLL, SV_SCROLL_IDENTIFY},
	 {TV_SCROLL, SV_SCROLL_LIGHT},
	 {TV_SCROLL, SV_SCROLL_PHASE_DOOR},
	 {TV_SCROLL, SV_SCROLL_PHASE_DOOR},
	 {TV_SCROLL, SV_SCROLL_PHASE_DOOR},
	 {TV_SCROLL, SV_SCROLL_MONSTER_CONFUSION},

	 {TV_SCROLL, SV_SCROLL_MAPPING},
	 {TV_SCROLL, SV_SCROLL_WORD_OF_RECALL},
	 {TV_SCROLL, SV_SCROLL_WORD_OF_RECALL},
	 {TV_SCROLL, SV_SCROLL_DETECT_TRAP},
	 {TV_SCROLL, SV_SCROLL_DETECT_DOOR},
	 {TV_SCROLL, SV_SCROLL_DETECT_INVIS},
	 {TV_SCROLL, SV_SCROLL_RECHARGING},
	 {TV_SCROLL, SV_SCROLL_SATISFY_HUNGER},

	 {TV_SCROLL, SV_SCROLL_DISPEL_UNDEAD},
	 {TV_POTION, SV_POTION_HEROISM},
	 {TV_POTION, SV_POTION_RES_STR},
	 {TV_POTION, SV_POTION_RES_INT},
	 {TV_POTION, SV_POTION_RES_WIS},
	 {TV_POTION, SV_POTION_RES_DEX},
	 {TV_POTION, SV_POTION_RES_CON},
	 {TV_POTION, SV_POTION_RES_CHR}
	 },

	{
	 /* Magic-User store. */

	 {TV_WAND, SV_WAND_STONE_TO_MUD},
	 {TV_WAND, SV_WAND_LIGHT},
	 {TV_WAND, SV_WAND_DISARMING},
	 {TV_STAFF, SV_STAFF_DETECT_TRAP},
	 {TV_STAFF, SV_STAFF_DETECT_TRAP},
	 {TV_STAFF, SV_STAFF_DETECT_DOOR},
	 {TV_WAND, SV_WAND_SLOW_MONSTER},
	 {TV_WAND, SV_WAND_CONFUSE_MONSTER},

	 {TV_WAND, SV_WAND_SLEEP_MONSTER},
	 {TV_WAND, SV_WAND_MAGIC_MISSILE},
	 {TV_WAND, SV_WAND_STINKING_CLOUD},
	 {TV_RING, SV_RING_PEWTER},
	 {TV_AMULET, SV_AMULET_MALACHITE},
	 {TV_WAND, SV_WAND_MAGIC_MISSILE},
	 {TV_STAFF, SV_STAFF_DETECT_TRAP},
	 {TV_STAFF, SV_STAFF_DETECT_DOOR},

	 {TV_STAFF, SV_STAFF_DETECT_GOLD},
	 {TV_STAFF, SV_STAFF_DETECT_ITEM},
	 {TV_STAFF, SV_STAFF_DETECT_INVIS},
	 {TV_STAFF, SV_STAFF_DETECT_EVIL},
	 {TV_STAFF, SV_STAFF_TELEPORTATION},
	 {TV_STAFF, SV_STAFF_TELEPORTATION},
	 {TV_STAFF, SV_STAFF_IDENTIFY},
	 {TV_STAFF, SV_STAFF_IDENTIFY},

	 {TV_WAND, SV_WAND_DOOR_DEST},
	 {TV_WAND, SV_WAND_STONE_TO_MUD},
	 {TV_WAND, SV_WAND_STINKING_CLOUD},
	 {TV_WAND, SV_WAND_POLYMORPH},
	 {TV_STAFF, SV_STAFF_LIGHT},
	 {TV_STAFF, SV_STAFF_MAPPING},
	 {TV_ROD, SV_ROD_DETECT_TRAP},
	 {TV_ROD, SV_ROD_DETECT_DOOR}
	 },

	{
	 /* Black Market (unused) */
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0}
	 },

	{
	 /* Home (unused) */
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0}
	 },

	{
	 /* Bookseller. */

	 {TV_MAGIC_BOOK, 0},
	 {TV_MAGIC_BOOK, 0},
	 {TV_MAGIC_BOOK, 0},
	 {TV_MAGIC_BOOK, 1},
	 {TV_MAGIC_BOOK, 1},
	 {TV_MAGIC_BOOK, 2},
	 {TV_MAGIC_BOOK, 2},
	 {TV_MAGIC_BOOK, 3},

	 {TV_PRAYER_BOOK, 0},
	 {TV_PRAYER_BOOK, 0},
	 {TV_PRAYER_BOOK, 0},
	 {TV_PRAYER_BOOK, 1},
	 {TV_PRAYER_BOOK, 1},
	 {TV_PRAYER_BOOK, 2},
	 {TV_PRAYER_BOOK, 2},
	 {TV_PRAYER_BOOK, 3},

	 {TV_DRUID_BOOK, 0},
	 {TV_DRUID_BOOK, 0},
	 {TV_DRUID_BOOK, 0},
	 {TV_DRUID_BOOK, 1},
	 {TV_DRUID_BOOK, 1},
	 {TV_DRUID_BOOK, 2},
	 {TV_DRUID_BOOK, 2},
	 {TV_DRUID_BOOK, 3},

	 {TV_NECRO_BOOK, 0},
	 {TV_NECRO_BOOK, 0},
	 {TV_NECRO_BOOK, 0},
	 {TV_NECRO_BOOK, 1},
	 {TV_NECRO_BOOK, 1},
	 {TV_NECRO_BOOK, 2},
	 {TV_NECRO_BOOK, 2},
	 {TV_NECRO_BOOK, 3}
	 },

	{
	 /* Travelling Merchant. */

	 {TV_FOOD, SV_FOOD_RATION},
	 {TV_FOOD, SV_FOOD_RATION},
	 {TV_FOOD, SV_FOOD_BISCUIT},
	 {TV_FOOD, SV_FOOD_JERKY},
	 {TV_FOOD, SV_FOOD_PINT_OF_WINE},
	 {TV_FOOD, SV_FOOD_PINT_OF_ALE},
	 {TV_LIGHT, SV_LIGHT_TORCH},
	 {TV_LIGHT, SV_LIGHT_LANTERN},

	 {TV_FLASK, 0},
	 {TV_FLASK, 0},
	 {TV_SHOT, SV_AMMO_NORMAL},
	 {TV_ARROW, SV_AMMO_NORMAL},
	 {TV_BOLT, SV_AMMO_NORMAL},
	 {TV_DIGGING, SV_SHOVEL},
	 {TV_DIGGING, SV_PICK},
	 {TV_BOW, SV_SLING},

	 {TV_CLOAK, SV_CLOAK},
	 {TV_CLOAK, SV_CLOAK},
	 {TV_SOFT_ARMOR, SV_ROBE},
	 {TV_GLOVES, SV_SET_OF_LEATHER_GLOVES},
	 {TV_BOOTS, SV_PAIR_OF_SOFT_LEATHER_BOOTS},
	 {TV_HELM, SV_HARD_LEATHER_CAP},
	 {TV_SHIELD, SV_WICKER_SHIELD},
	 {TV_SWORD, SV_SHORT_SWORD},

	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 {0, 0},
	 }

};

/*** Initialize others ***/

static void autoinscribe_init(void)
{
	if (inscriptions)
		FREE(inscriptions);

	inscriptions = 0;
	inscriptions_count = 0;

	inscriptions = C_ZNEW(AUTOINSCRIPTIONS_MAX, autoinscription);
}

static void autoinscribe_free(void)
{
	FREE(inscriptions);
}


/*
 * Initialize some other arrays
 */
static errr init_other(void)
{
	int i, k, n;


	/*** Prepare the various "bizarre" arrays ***/

	/* Initialize the "quark" package */
	(void) quarks_init();

	/* Initialize squelch things */
	autoinscribe_init();
	textui_knowledge_init();

	/* Initialize the "message" package */
	(void) messages_init();

	/*** Prepare grid arrays ***/

	/* Array of grids */
	temp_g = C_ZNEW(TEMP_MAX, u16b);

	/* Hack -- use some memory twice */
	temp_y = ((byte *) (temp_g)) + 0;
	temp_x = ((byte *) (temp_g)) + TEMP_MAX;


	/*** Prepare dungeon arrays ***/

	/* Padded into array */
	cave_info = C_ZNEW(DUNGEON_HGT, grid_256);

	/* Feature array */
	cave_feat = C_ZNEW(DUNGEON_HGT, byte_wid);

	/* Entity arrays */
	cave_o_idx = C_ZNEW(DUNGEON_HGT, s16b_wid);
	cave_m_idx = C_ZNEW(DUNGEON_HGT, s16b_wid);

	/* Flow arrays */
	cave_cost = C_ZNEW(DUNGEON_HGT, byte_wid);
	cave_when = C_ZNEW(DUNGEON_HGT, byte_wid);


	/*** Prepare entity arrays ***/

	/* Objects */
	o_list = C_ZNEW(z_info->o_max, object_type);

	/* Monsters */
	m_list = C_ZNEW(z_info->m_max, monster_type);

	/* Traps */
	trap_list = C_ZNEW(z_info->l_max, trap_type);


	/*** Prepare lore array ***/

	/* Lore */
	l_list = C_ZNEW(z_info->r_max, monster_lore);


	/*** Prepare character display arrays ***/

	/* Lines of character screen/dump */
	dumpline = C_ZNEW(DUMP_MAX_LINES, char_attr_line);

	/* Lines of character subwindows */
	pline0 = C_ZNEW(30, char_attr_line);
	pline1 = C_ZNEW(30, char_attr_line);

	/*** Prepare quest array ***/

	quest_init();


	/*** Prepare the inventory ***/

	/* Allocate it */
	p_ptr->inventory = C_ZNEW(ALL_INVEN_TOTAL, object_type);


  /*** Prepare the stores ***/

	/* Allocate the stores */
	store = C_ZNEW(MAX_STORES, struct store_type);

	for (n = 0; n < MAX_STORES; n++) {
		/* Access the store */
		struct store_type *st_ptr = &store[n];

		/* Set the type */
		st_ptr->type = type_of_store[n];

		/* Assume full stock */
		st_ptr->stock_size = STORE_INVEN_MAX;

		/* Allocate the stock */
		st_ptr->stock = C_ZNEW(st_ptr->stock_size, object_type);

		/* No table for the black market or home */
		if ((st_ptr->type == STORE_BLACKM) || (st_ptr->type == STORE_HOME))
			continue;

		/* Assume full table */
		st_ptr->table_size = STORE_CHOICES;

		/* Nothing there yet */
		st_ptr->table_num = 0;

		/* Allocate the stock */
		st_ptr->table = C_ZNEW(st_ptr->table_size, s16b);

		/* Scan the choices */
		for (k = 0; k < STORE_CHOICES; k++) {
			int k_idx;
			int tv, sv;

			/* Cut short for the merchant */
			if ((st_ptr->type == STORE_MERCH) && (k >= STORE_CHOICES - 8)) {
				st_ptr->table[st_ptr->table_num++] = 0;
				continue;
			}

			/* Extract the tval/sval codes */
			tv = store_table[st_ptr->type][k][0];
			sv = store_table[st_ptr->type][k][1];

			/* Look for it */
			for (k_idx = 1; k_idx < z_info->k_max; k_idx++) {
				object_kind *k_ptr = &k_info[k_idx];

				/* Found a match */
				if ((k_ptr->tval == tv) && (k_ptr->sval == sv))
					break;
			}

			/* Catch errors */
			if (k_idx == z_info->k_max)
				continue;

			/* Add that item index to the table */
			st_ptr->table[st_ptr->table_num++] = k_idx;
		}
	}


	/*** Prepare the options ***/
	init_options();

	/* Initialize the window flags */
	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		/* Assume no flags */
		op_ptr->window_flag[i] = 0L;
	}


	/*** Pre-allocate space for the "format()" buffer ***/

	/* Hack -- Just call the "format()" function */
	(void) format("I used to wonder what friendship could be, until you shared its magic with me.");

	/* Success */
	return (0);
}



/*
 * Initialize some other arrays
 */
static errr init_alloc(void)
{
	int i, j;

	object_kind *k_ptr;

	monster_race *r_ptr;

	ego_item_type *e_ptr;

	alloc_entry *table;

	s16b num[MAX_DEPTH];

	s16b aux[MAX_DEPTH];


  /*** Analyze object allocation info ***/

	/* Clear the "aux" array */
	(void) C_WIPE(aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	(void) C_WIPE(num, MAX_DEPTH, s16b);

	/* Size of "alloc_kind_table" */
	alloc_kind_size = 0;

	/* Scan the objects */
	for (i = 1; i < z_info->k_max; i++) {
		k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (j = 0; j < 4; j++) {
			/* Count the "legal" entries */
			if (k_ptr->chance[j]) {
				/* Count the entries */
				alloc_kind_size++;

				/* Group by level */
				num[k_ptr->locale[j]]++;
			}
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH; i++) {
		/* Group by level */
		num[i] += num[i - 1];
	}

	/* Paranoia */
	if (!num[0])
		quit("No town objects!");


	/*** Initialize object allocation info ***/

	/* Allocate the alloc_kind_table */
	alloc_kind_table = C_ZNEW(alloc_kind_size, alloc_entry);

	/* Access the table entry */
	table = alloc_kind_table;

	/* Scan the objects */
	for (i = 1; i < z_info->k_max; i++) {
		object_kind *k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (j = 0; j < 4; j++) {
			/* Count the "legal" entries */
			if (k_ptr->chance[j]) {
				int p, x, y, z;

				/* Extract the base level */
				x = k_ptr->locale[j];

				/* Extract the base probability */
				p = (100 / k_ptr->chance[j]);

				/* Skip entries preceding our locale */
				y = (x > 0) ? num[x - 1] : 0;

				/* Skip previous entries at this locale */
				z = y + aux[x];

				/* Load the entry */
				table[z].index = i;
				table[z].level = x;
				table[z].prob1 = p;
				table[z].prob2 = p;
				table[z].prob3 = p;

				/* Another entry complete for this locale */
				aux[x]++;
			}
		}
	}

	/*** Analyze monster allocation info ***/

	/* Clear the "aux" array */
	(void) C_WIPE(aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	(void) C_WIPE(num, MAX_DEPTH, s16b);

	/* Size of "alloc_race_table" */
	alloc_race_size = 0;

	/* Scan the monsters */
	for (i = 1; i < z_info->r_max; i++) {
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Legal monsters */
		if (r_ptr->rarity) {
			/* Count the entries */
			alloc_race_size++;

			/* Group by level */
			num[r_ptr->level]++;
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH; i++) {
		/* Group by level */
		num[i] += num[i - 1];
	}

	/* Paranoia */
	if (!num[0])
		quit("No town monsters!");


	/*** Initialize monster allocation info ***/

	/* Allocate the alloc_race_table */
	alloc_race_table = C_ZNEW(alloc_race_size, alloc_entry);

	/* Get the table entry */
	table = alloc_race_table;

	/* Scan the monsters (not the ghost) */
	for (i = 1; i < z_info->r_max - 1; i++) {
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Count valid pairs */
		if (r_ptr->rarity) {
			int p, x, y, z;

			/* Extract the base level */
			x = r_ptr->level;

			/* Extract the base probability */
			p = (100 / r_ptr->rarity);

			/* Skip entries preceding our locale */
			y = (x > 0) ? num[x - 1] : 0;

			/* Skip previous entries at this locale */
			z = y + aux[x];

			/* Load the entry */
			table[z].index = i;
			table[z].level = x;
			table[z].prob1 = p;
			table[z].prob2 = p;
			table[z].prob3 = p;

			/* Another entry complete for this locale */
			aux[x]++;
		}
	}

	/*** Analyze ego_item allocation info ***/

	/* Clear the "aux" array */
	(void) C_WIPE(aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	(void) C_WIPE(num, MAX_DEPTH, s16b);

	/* Size of "alloc_ego_table" */
	alloc_ego_size = 0;

	/* Scan the ego items */
	for (i = 1; i < z_info->e_max; i++) {
		/* Get the i'th ego item */
		e_ptr = &e_info[i];

		/* Legal items */
		if (e_ptr->rarity) {
			/* Count the entries */
			alloc_ego_size++;

			/* Group by level */
			num[e_ptr->level]++;
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH; i++) {
		/* Group by level */
		num[i] += num[i - 1];
	}

	/*** Initialize ego-item allocation info ***/

	/* Allocate the alloc_ego_table */
	alloc_ego_table = C_ZNEW(alloc_ego_size, alloc_entry);

	/* Get the table entry */
	table = alloc_ego_table;

	/* Scan the ego-items */
	for (i = 1; i < z_info->e_max; i++) {
		/* Get the i'th ego item */
		e_ptr = &e_info[i];

		/* Count valid pairs */
		if (e_ptr->rarity) {
			int p, x, y, z;

			/* Extract the base level */
			x = e_ptr->level;

			/* Extract the base probability */
			p = (100 / e_ptr->rarity);

			/* Skip entries preceding our locale */
			y = (x > 0) ? num[x - 1] : 0;

			/* Skip previous entries at this locale */
			z = y + aux[x];

			/* Load the entry */
			table[z].index = i;
			table[z].level = x;
			table[z].prob1 = p;
			table[z].prob2 = p;
			table[z].prob3 = p;

			/* Another entry complete for this locale */
			aux[x]++;
		}
	}


	/* Success */
	return (0);
}



/**
 * Initialize the racial probability array
 */
extern errr init_race_probs(void)
{
	int i, j, k, n;

	/* Make the array */
	race_prob = C_ZNEW(32, u16b_stage);

	/*** Prepare temporary adjacency arrays ***/
	adjacency = C_ZNEW(NUM_STAGES, u16b_stage);
	stage_path = C_ZNEW(NUM_STAGES, u16b_stage);
	temp_path = C_ZNEW(NUM_STAGES, u16b_stage);

	/* Make the adjacency matrix */
	for (i = 0; i < NUM_STAGES; i++) {
		/* Initialise this row */
		for (k = 0; k < NUM_STAGES; k++) {
			adjacency[i][k] = 0;
			stage_path[i][k] = 0;
			temp_path[i][k] = 0;
		}

		/* Add 1s where there's an adjacent stage (not up or down) */
		for (k = 2; k < 6; k++)
			if (stage_map[i][k] != 0) {
				adjacency[i][stage_map[i][k]] = 1;
				temp_path[i][stage_map[i][k]] = 1;
			}
	}

	/* Power it up (squaring 3 times gives eighth power) */
	for (n = 0; n < 3; n++) {
		/* Square */
		for (i = 0; i < NUM_STAGES; i++)
			for (j = 0; j < NUM_STAGES; j++) {
				stage_path[i][j] = 0;
				for (k = 0; k < NUM_STAGES; k++)
					stage_path[i][j] += temp_path[i][k] * temp_path[k][j];
			}

		/* Copy it over for the next squaring or final multiply */
		for (i = 0; i < NUM_STAGES; i++)
			for (j = 0; j < NUM_STAGES; j++)
				temp_path[i][j] = stage_path[i][j];

	}

	/* Get the max of length 8 and length 9 paths */
	for (i = 0; i < NUM_STAGES; i++)
		for (j = 0; j < NUM_STAGES; j++) {
			/* Multiply to get the length 9s */
			stage_path[i][j] = 0;
			for (k = 0; k < NUM_STAGES; k++)
				stage_path[i][j] += temp_path[i][k] * adjacency[k][j];

			/* Now replace by the length 8s if it's larger */
			if (stage_path[i][j] < temp_path[i][j])
				stage_path[i][j] = temp_path[i][j];

		}

	/* We now have the maximum of the number of paths of length 8 and the 
	 * number of paths of length 9 (we need to try odd and even length paths,
	 * as using just one leads to anomalies) from any stage to any other,
	 * which we will use as a basis for the racial probability table for 
	 * racially based monsters in any given stage.  For a stage, we give 
	 * every race a 1, then add the number of paths of length 8 from their 
	 * hometown to that stage.  We then turn every row entry into the 
	 * cumulative total of the row to that point.  Whenever a racially based 
	 * monster is called for, we will take a random integer less than the 
	 * last entry of the row for that stage, and proceed along the row, 
	 * allocating the race corresponding to the position where we first 
	 *exceed that integer.
	 */

	for (i = 0; i < NUM_STAGES; i++) {
		int prob = 0;

		/* No more than 32 races */
		for (j = 0; j < 32; j++) {
			/* Nobody lives nowhere */
			if (stage_map[i][LOCALITY] == NOWHERE) {
				race_prob[j][i] = 0;
				continue;
			}

			/* Invalid race */
			if (j >= z_info->p_max) {
				race_prob[j][i] = 0;
				continue;
			}

			/* Enter the cumulative probability */
			prob += 1 + stage_path[towns[p_info[j].hometown]][i];
			race_prob[j][i] = prob;
		}
	}

	/* Free the temporary arrays */
	FREE(temp_path);
	FREE(adjacency);
	FREE(stage_path);

	return 0;
}


/**
 * Hack - identify set item artifacts.
 *
 * Go through the list of Set Items and identify all artifacts in each set
 * as belonging to that set. By GS
 */
void update_artifact_sets()
{
	byte i;
	byte j;
	set_type *set_ptr;
	set_element *set_el_ptr;
	artifact_type *a_ptr;

	for (i = 0; i < z_info->set_max; i++) {

		set_ptr = &set_info[i];
		for (j = 0; j < set_ptr->no_of_items; j++) {
			set_el_ptr = &set_ptr->set_items[j];
			a_ptr = &a_info[set_el_ptr->a_idx];
			a_ptr->set_no = i;
		}
	}
}

/*
 * Hack -- main Angband initialization entry point
 *
 * Verify some files, display the "news.txt" file, create
 * the high score file, initialize all internal arrays, and
 * load the basic "user pref files".
 *
 * Be very careful to keep track of the order in which things
 * are initialized, in particular, the only thing *known* to
 * be available when this function is called is the "z-term.c"
 * package, and that may not be fully initialized until the
 * end of this function, when the default "user pref files"
 * are loaded and "Term_xtra(TERM_XTRA_REACT,0)" is called.
 *
 * Note that this function attempts to verify the "news" file,
 * and the game aborts (cleanly) on failure, since without the
 * "news" file, it is likely that the "lib" folder has not been
 * correctly located.  Otherwise, the news file is displayed for
 * the user.
 *
 * Note that this function attempts to verify (or create) the
 * "high score" file, and the game aborts (cleanly) on failure,
 * since one of the most common "extraction" failures involves
 * failing to extract all sub-directories (even empty ones), such
 * as by failing to use the "-d" option of "pkunzip", or failing
 * to use the "save empty directories" option with "Compact Pro".
 * This error will often be caught by the "high score" creation
 * code below, since the "lib/apex" directory, being empty in the
 * standard distributions, is most likely to be "lost", making it
 * impossible to create the high score file.
 *
 * Note that various things are initialized by this function,
 * including everything that was once done by "init_some_arrays".
 *
 * This initialization involves the parsing of special files
 * in the "lib/data" and sometimes the "lib/edit" directories.
 *
 * Note that the "template" files are initialized first, since they
 * often contain errors.  This means that macros and message recall
 * and things like that are not available until after they are done.
 *
 * We load the default "user pref files" here in case any "color"
 * changes are needed before character creation.
 *
 * Note that the "graf-xxx.prf" file must be loaded separately,
 * if needed, in the first (?) pass through "TERM_XTRA_REACT".
 */
bool init_angband(void)
{
	event_signal(EVENT_ENTER_INIT);
	Term_xtra(TERM_XTRA_DELAY, 2000);


	/*** Initialize some arrays ***/

	/* Initialize size info */
	event_signal_string(EVENT_INITSTATUS, "Initializing array sizes...");
	if (run_parser(&z_parser))
		quit("Cannot initialize sizes");

	/* Initialize trap info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (traps)");
	if (run_parser(&trap_parser))
		quit("Cannot initialize traps");

	/* Initialize feature info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (features)");
	if (run_parser(&f_parser))
		quit("Cannot initialize features");

	/* Initialize object info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (objects)");
	if (run_parser(&k_parser))
		quit("Cannot initialize objects");

	/* Initialize ego-item info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (ego-items)");
	if (run_parser(&e_parser))
		quit("Cannot initialize ego-items");

	/* Initialize monster pain messages */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (pain messages)");
	if (run_parser(&mp_parser))
		quit("Cannot initialize monster pain messages");

	/* Initialize monster-base info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (monster bases)");
	if (run_parser(&rb_parser))
		quit("Cannot initialize monster bases");

	/* Initialize monster info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (monsters)");
	if (run_parser(&r_parser))
		quit("Cannot initialize monsters");

	/* Initialize artifact info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (artifacts)");
	if (run_parser(&a_parser))
		quit("Cannot initialize artifacts");

	/* Initialize set item info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (set items)");
	if (run_parser(&set_parser))
		quit("Cannot initialize set items");
	update_artifact_sets();

	/* Initialize vault info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (vaults)");
	if (run_parser(&v_parser))
		quit("Cannot initialize vaults");

	/* Initialize themed level info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (themed)");
	if (run_parser(&t_parser))
		quit("Cannot initialize themed levels");

	/* Initialize history info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (histories)");
	if (run_parser(&h_parser))
		quit("Cannot initialize histories");

	/* Initialize store info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (stores)");
	if (run_parser(&b_parser))
		quit("Cannot initialize stores");

	/* Initialize race info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (races)");
	if (run_parser(&p_parser))
		quit("Cannot initialize races");

	/* Initialize class info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (classes)");
	if (run_parser(&c_parser))
		quit("Cannot initialize classes");
		
	/* Initialize cutie mark info */
	event_signal_string(EVENT_INITSTATUS,
                        "Initializing arrays... (cutie marks)");
    if (run_parser(&mark_parser))
        quit("Cannot initialize cutie marks");
        
    /* Initialize innate abilities */
    event_signal_string(EVENT_INITSTATUS,
                        "Initializing arrays... (innate abilities)");
    if(run_parser(&ability_parser))
        quit("Cannot initialize innate abilities");

	/* Initialize flavor info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (flavors)");
	if (run_parser(&flavor_parser))
		quit("Cannot initialize flavors");

	/* Initialize spell info */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (spells)");
	if (run_parser(&s_parser))
		quit("Cannot initialize spells");

	/* Initialize hint text */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (hints)");
	if (run_parser(&hints_parser))
		quit("Cannot initialize hints");

	/* Initialise random name data */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (random names)");
	if (run_parser(&names_parser))
		quit("Can't parse names");

	/* Initialize some other arrays */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (other)");
	if (init_other())
		quit("Cannot initialize other stuff");

#if 0
	/* Initialise store stocking data */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (store stocks)");
	store_init();
#endif
	/* Initialize some other arrays */
	event_signal_string(EVENT_INITSTATUS,
						"Initializing arrays... (alloc)");
	if (init_alloc())
		quit("Cannot initialize alloc stuff");

	/*** Load default user pref files ***/

	/* Initialize feature info */
	event_signal_string(EVENT_INITSTATUS,
						"Loading basic user pref file...");

	/* Process that file */
	(void) process_pref_file("pref.prf", FALSE, FALSE);

	/* Done */
	event_signal_string(EVENT_INITSTATUS, "Initialization complete");
	Term_xtra(TERM_XTRA_DELAY, 1000);
	show_news();

	/* Sneakily init command list */
	cmd_init();

	/* Ask for a "command" until we get one we like. */
	while (1) {
		game_command *command_req;
		int failed = cmd_get(CMD_INIT, &command_req, TRUE);

		if (failed)
			continue;
		else if (command_req->command == CMD_QUIT)
			quit(NULL);
		else if (command_req->command == CMD_NEWGAME) {
			event_signal(EVENT_LEAVE_INIT);
			return TRUE;
		} else if (command_req->command == CMD_LOADFILE) {
			event_signal(EVENT_LEAVE_INIT);
			return FALSE;
		}
	}
}


void cleanup_angband(void)
{
	int i;

	/* Free the macros */
	keymap_free();

	/* Free racial probability arrays */
	FREE(race_prob);
	FREE(dummy);

	/* Free the artifact lists */
	FREE(artifact_normal);
	FREE(artifact_special);

	/* Free the allocation tables */
	FREE(alloc_kind_table);
	FREE(alloc_ego_table);
	FREE(alloc_race_table);

	event_remove_all_handlers();

	if (store) {
		/* Free the store inventories */
		for (i = 0; i < MAX_STORES; i++) {
			/* Get the store */
			struct store_type *st_ptr = &store[i];

			/* Free the store inventory */
			FREE(st_ptr->stock);
			FREE(st_ptr->table);
		}
	}


	/* Free the stores */
	FREE(store);

	/* Free the quests */
	quest_free();

	FREE(p_ptr->inventory);

	/* Free the character screen arrays */
	FREE(dumpline);
	FREE(pline0);
	FREE(pline1);

	/* Free the lore, trap, monster, and object lists */
	FREE(l_list);
	FREE(trap_list);
	FREE(m_list);
	FREE(o_list);

	/* Flow arrays */
	FREE(cave_when);
	FREE(cave_cost);

	/* Free the cave */
	FREE(cave_o_idx);
	FREE(cave_m_idx);
	FREE(cave_feat);
	FREE(cave_info);

	/* Free the temp array */
	FREE(temp_g);

	/* Free the messages */
	messages_free();

	/* Free the history */
	history_clear();

	/* Free the autoinscriptions */
	autoinscribe_free();

	/* Free the "quarks" */
	quarks_free();

	cleanup_parser(&k_parser);
	cleanup_parser(&a_parser);
	cleanup_parser(&set_parser);
	cleanup_parser(&names_parser);
	cleanup_parser(&trap_parser);
	cleanup_parser(&r_parser);
	cleanup_parser(&rb_parser);
	cleanup_parser(&f_parser);
	cleanup_parser(&e_parser);
	cleanup_parser(&b_parser);
	cleanup_parser(&p_parser);
	cleanup_parser(&c_parser);
	cleanup_parser(&v_parser);
	cleanup_parser(&h_parser);
	cleanup_parser(&t_parser);
	cleanup_parser(&flavor_parser);
	cleanup_parser(&s_parser);
	cleanup_parser(&hints_parser);
	cleanup_parser(&mp_parser);
	cleanup_parser(&mark_parser);
	cleanup_parser(&ability_parser);
	cleanup_parser(&z_parser);

	/* Free the format() buffer */
	vformat_kill();

	/* Free the directories */
	string_free(ANGBAND_DIR_APEX);
	string_free(ANGBAND_DIR_BONE);
	string_free(ANGBAND_DIR_EDIT);
	string_free(ANGBAND_DIR_FILE);
	string_free(ANGBAND_DIR_HELP);
	string_free(ANGBAND_DIR_INFO);
	string_free(ANGBAND_DIR_SAVE);
	string_free(ANGBAND_DIR_PREF);
	string_free(ANGBAND_DIR_USER);
	string_free(ANGBAND_DIR_XTRA);

	string_free(ANGBAND_DIR_XTRA_FONT);
	string_free(ANGBAND_DIR_XTRA_GRAF);
	string_free(ANGBAND_DIR_XTRA_HELP);
	string_free(ANGBAND_DIR_XTRA_SOUND);
	string_free(ANGBAND_DIR_XTRA_ICON);
}
