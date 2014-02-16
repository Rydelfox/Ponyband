#ifndef INCLUDED_DEFINES_H
#define INCLUDED_DEFINES_H

/** \file defines.h 
    \brief Constant, flag and macro definitions 

 * Version #, grid, etc. size, MAX_* figures, limits, constants, critical
 * values, etc. for every characteristic of Angband.  Indexes, text locat-
 * ions, list of summonable monsters. Feature, artifact and ego-item codes.
 * Object tval (kind) and sval (specific type) with sval limitations.
 * Monster blow constants, function, player, object and monster bit flags
 * (translation from code to flag).  Definitions of options and object in-
 * scriptions.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *


 *
 * Do not edit this file unless you know *exactly* what you are doing.
 * 
 * Some of the values in this file were chosen to preserve game balance,
 * while others are hard-coded based on the format of old save-files, the
 * definition of arrays in various places, mathematical properties, fast
 * computation, storage limits, or the format of external text files.
 *
 * Changing some of these values will induce crashes or memory errors or
 * savefile mis-reads.  Most of the comments in this file are meant as
 * reminders, not complete descriptions, and even a complete knowledge
 * of the source may not be sufficient to fully understand the effects
 * of changing certain definitions.
 *
 * Lastly, note that the code does not always use the symbolic constants
 * below, and sometimes uses various hard-coded values that may not even
 * be defined in this file, but which may be related to definitions here.
 * This is of course bad programming practice, but nobody is perfect...
 *
 * For example, there are MANY things that depend on the screen being
 * 80x24, with the top line used for messages, the bottom line being
 * used for status, and exactly 22 lines used to show the dungeon.
 * Just because your screen can hold 46 lines does not mean that the
 * game will work if you try to use 44 lines to show the dungeon.
 *
 * You have been warned.
 */

/* Hack */
#include "tvalsval.h"

/**
 * Name of the version/variant
 */
#define SAVEFILE_NAME  "FAAN"


/**
 * Current version string - according to FAangband reckoning.
 */
/*
#ifdef BUILD_ID
#define VERSION_STRING	"1.4.4 (" BUILD_ID ")"
#endif
*/

/*
 * Current FAangband version numbers.
 */
#define VERSION_MAJOR	0
#define VERSION_MINOR	1
#define VERSION_PATCH	0
#define VERSION_EXTRA	0

/**
 * Number of grids in each block (vertically)
 * Probably hard-coded to 11, see "generate.c"
 */
#define BLOCK_HGT	11

/**
 * Number of grids in each block (horizontally)
 * Probably hard-coded to 11, see "generate.c"
 */
#define BLOCK_WID	11


/**
 * Number of grids in each panel (vertically)
 */
#define PANEL_HGT	((int)(BLOCK_HGT / tile_height))

/**
 * Number of grids in each panel (horizontally)
 */
#define PANEL_WID       ((int)(BLOCK_WID / tile_width))


/**
 * Number of text rows in each map screen, regardless of tile size
 */
#define SCREEN_ROWS	(Term->hgt - ROW_MAP  - 1) 

/**
 * Number of grids in each screen (vertically)
 */
#define SCREEN_HGT    ((int) (SCREEN_ROWS / tile_height))

/**
 * Number of grids in each screen (horizontally)
 */
#define SCREEN_WID	((int)((Term->wid - COL_MAP - 1) / tile_width))

#define ROW_MAP			1
#define COL_MAP			13

/**
 * Number of grids in each dungeon (from top to bottom)
 * Must be a multiple of SCREEN_HGT
 * Must be less or equal to 256
 */
#define DUNGEON_HGT		66

/**
 * Number of grids in each dungeon (from left to right)
 * Must be a multiple of SCREEN_WID
 * Must be less or equal to 256
 */
#define DUNGEON_WID		198

/*
 * Radii for various detection spells. -BR-
 */
#define DETECT_RAD_DEFAULT      30
#define DETECT_RAD_MAP          255

/**
 * Maximum amount of Angband windows.
 */
#define ANGBAND_TERM_MAX 8


/**
 * Maximum number of player "sex" types (see "table.c", etc)
 */
#define MAX_SEXES            2

/*
 * Hack -- first normal and random artifact in the artifact list.  
 * All of the artifacts with indexes from 1 to 22 are special (lights, 
 * rings, amulets), the ones from 23 to 209 are normal, and the ones from 
 * 210 to 249 are random.
 */
#define ART_MIN_NORMAL		23
#define ART_MIN_RANDOM		210

/*
 * Number of tval/min-sval/max-sval slots per ego_item
 */
#define EGO_TVALS_MAX 3

/**
 * Maximum number of high scores in the high score file
 */
#define MAX_HISCORES	100

/**
 * OPTION: Maximum number of autoinscriptions(see "object1.c")
 */
#define AUTOINSCRIPTIONS_MAX 216

/**
 * Defines for graphics mode
 */

#define GRAPHICS_NONE           0


/* player_type.noscore flags */
#define NOSCORE_WIZARD		0x0002
#define NOSCORE_DEBUG		0x0008
#define NOSCORE_BORG		0x0010



/*** FAangband Themed Levels ***/

/**
 * No current theme (player is on a normal level)
 */
#define THEME_NONE		0

/*
 * Themed level indices.  Used to activate any theme-specific code. 
 * See generate.c for the table of themed level information.
 */
#define THEME_ELEMENTAL  1
#define THEME_DRAGON     2
#define THEME_WILDERNESS 3
#define THEME_DEMON      4
#define THEME_MINE       5
#define THEME_WARLORDS   6
#define THEME_AELUIN     7
#define THEME_ESTOLAD    8
#define THEME_SLAIN      9

/**
 * Current number of defined themes.  The maximum theoretical number is 32.
 */
#define THEME_MAX		9


/*
 * Store constants
 */
#define STORE_INVEN_MAX	24	/* Max number of discrete objs in inven */
#define STORE_CHOICES	32	/* Number of items to choose stock from */
#define STORE_OBJ_LEVEL	5	/* Magic Level for normal stores */
#define STORE_TURNOVER	9	/* Normal shop turnover, per day */
#define STORE_MIN_KEEP	6	/* Min slots to "always" keep full */
#define STORE_MAX_KEEP	18	/* Max slots to "always" keep full */
#define STORE_SHUFFLE	20	/* 1/Chance (per day) of an owner changing */
#define STORE_TURNS	1000	/* Number of turns between turnovers */


/**
 * Maximum dungeon level.  The player can never reach this level
 * in the dungeon, and this value is used for various calculations
 * involving object and monster creation.  It must be at least 100.
 * Setting it below 128 may prevent the creation of some objects.
 */
#define MAX_DEPTH	128


/*
 * Misc constants
 */
#define TOWN_DAWN	2000	/* Number of turns from dawn to dawn XXX */
#define BREAK_GLYPH	300	/* Rune of protection resistance */
#define BTH_PLUS_ADJ    1       /* Adjust BTH per plus-to-hit */
#define MON_MULT_ADJ	8	/* High value slows multiplication */
#define MON_SUMMON_ADJ	2	/* Adjust level of summoned creatures */
#define MON_DRAIN_LIFE	2	/* Percent of player exp drained per hit */
#define USE_DEVICE      3	/* x> Harder devices x< Easier devices     */

/**
 * Fraction of turns in which the extend magic special ability causes timers to
 * not decrement.
 */
#define EXTEND_MAGIC_FRACTION  3 /* Skip 3rd turn -> 50% longer durations */

/**
 * Amount of mana removed by Mana Burn specialty.
 */
#define BASE_MANA_BURN          20

/*
 * Refueling constants
 */
#define FUEL_TORCH	5000	/* Maximum amount of fuel in a torch */
#define FUEL_LAMP	15000   /* Maximum amount of fuel in a lantern */


/*
 * More maximum values
 */
#define MAX_SIGHT_LGE   20      /* Maximum view distance */
#define MAX_RANGE_LGE   20      /* Maximum projection range */
#define MAX_SIGHT_SML   10      /* Maximum view distance (small devices) */
#define MAX_RANGE_SML   10      /* Maximum projection range (small devices) */
#define MAX_SIGHT (MODE(SMALL_DEVICE) ? MAX_SIGHT_SML : MAX_SIGHT_LGE)  
#define MAX_RANGE (MODE(SMALL_DEVICE) ? MAX_RANGE_SML : MAX_RANGE_LGE)


/*** General index values ***/


/*
 * Indexes of the various "stats" (hard-coded by savefiles, etc).
 */
enum
{
	A_STR = 0,
	A_INT,
	A_WIS,
	A_DEX,
	A_CON,
	A_CHR,

	A_MAX
};



/*
 * Indexes for array of player incremental resists.
 */
#define P_RES_ACID		0
#define P_RES_ELEC		1
#define P_RES_FIRE		2
#define P_RES_COLD		3
#define P_RES_POIS		4
#define P_RES_LIGHT		5
#define P_RES_DARK		6
#define P_RES_CONFU		7
#define P_RES_SOUND		8
#define P_RES_SHARD		9
#define P_RES_NEXUS		10
#define P_RES_NETHR		11
#define P_RES_CHAOS		12
#define P_RES_DISEN		13
#define MAX_P_RES		14

/*
 * Indexes for array of player non-stat bonuses.
 */
#define P_BONUS_M_MASTERY       0
#define P_BONUS_STEALTH         1
#define P_BONUS_SEARCH          2
#define P_BONUS_INFRA           3
#define P_BONUS_TUNNEL          4
#define P_BONUS_SPEED           5
#define P_BONUS_SHOTS           6
#define P_BONUS_MIGHT           7
#define MAX_P_BONUS             8

/*
 * Indexes for array of player slay multiples.
 */
#define P_SLAY_ANIMAL           0
#define P_SLAY_EVIL             1
#define P_SLAY_UNDEAD           2
#define P_SLAY_DEMON            3
#define P_SLAY_ORC              4
#define P_SLAY_TROLL            5
#define P_SLAY_GIANT            6
#define P_SLAY_DRAGON           7
#define MAX_P_SLAY              8

/*
 * Indexes for array of player brand multiples.
 */
#define P_BRAND_ACID            0
#define P_BRAND_ELEC            1
#define P_BRAND_FIRE            2
#define P_BRAND_COLD            3
#define P_BRAND_POIS            4
#define MAX_P_BRAND             5

/*
 * Resistance limits - the number really means percentage damage taken -NRM-.
 */
#define RES_LEVEL_BASE		100
#define RES_LEVEL_MAX		200
#define RES_LEVEL_MIN		0

/*
 * Incremental resistance modifiers and caps.
 */
#define RES_BOOST_NORMAL	60
#define RES_BOOST_GREAT		45
#define RES_BOOST_IMMUNE	0
#define RES_BOOST_MINOR		75
#define RES_CUT_MINOR           130
#define RES_CUT_NORMAL          150
#define RES_CUT_GREAT           180
#define RES_CAP_EXTREME		75
#define RES_CAP_MODERATE	40
#define RES_CAP_ITEM            20

/* Defaults and modifiers for bonuses and multiples */
#define BONUS_BASE              0
#define MULTIPLE_BASE           10
#define SLAY_BOOST_SMALL        15  /* Slay Evil */
#define SLAY_BOOST_MINOR        17  /* Slay Animal */
#define SLAY_BOOST_NORMAL       20  /* Regular Slay */
#define SLAY_BOOST_KILL         25  /* Slay Kill */
#define BRAND_BOOST_NORMAL      17  /* Regular Brand */

/*
 * Some qualitative checks.
 */
#define p_resist_pos(X) \
   (p_ptr->state.res_list[X] < 100)
#define p_resist_good(X) \
   (p_ptr->state.res_list[X] <= 80)
#define p_resist_strong(X) \
   (p_ptr->state.res_list[X] <= 20)
#define p_immune(X) \
   (p_ptr->state.res_list[X] == 0)
#define p_vulnerable(X) \
   (p_ptr->state.res_list[X] > 100)

#define k_resist_pos(X) \
   (p_ptr->state.dis_res_list[X] < 100)
#define k_resist_good(X) \
   (p_ptr->state.dis_res_list[X] <= 80)
#define k_resist_strong(X) \
   (p_ptr->state.dis_res_list[X] <= 20)
#define k_immune(X) \
   (p_ptr->state.dis_res_list[X] == 0)
#define k_vulnerable(X) \
   (p_ptr->state.dis_res_list[X] > 100)

#define o_slay_weak(X, Y) \
   (X->multiple_slay[Y] < 10)
#define o_slay(X, Y) \
   (X->multiple_slay[Y] > 10)
#define o_kill(X, Y) \
   (X->multiple_slay[Y] > 20)

#define o_brand(X, Y) \
   (X->multiple_brand[Y] > 10)

/*
 * Some constants for the "learn" code.  These generalized from the
 * old DRS constants.
 */
#define LRN_FREE_SAVE	14
#define LRN_MANA	15
#define LRN_ACID	16
#define LRN_ELEC	17
#define LRN_FIRE	18
#define LRN_COLD	19
#define LRN_POIS	20
#define LRN_FEAR_SAVE	21
#define LRN_LIGHT	22
#define LRN_DARK	23
#define LRN_BLIND	24
#define LRN_CONFU	25
#define LRN_SOUND	26
#define LRN_SHARD	27
#define LRN_NEXUS	28
#define LRN_NETHR	29
#define LRN_CHAOS	30
#define LRN_DISEN	31
/* new in Oangband 0.5 and beyond */
#define LRN_DFIRE	38
#define LRN_SAVE        39
#define LRN_ARCH        40
#define LRN_PARCH       41
#define LRN_ICE         42
#define LRN_PLAS        43
#define LRN_SOUND2      44 /* attacks which aren't resisted, 
			    * but res sound prevent stun */
#define LRN_STORM       45
#define LRN_WATER       46
#define LRN_NEXUS_SAVE  47 /* Both resist Nexus and Saves apply */
#define LRN_BLIND_SAVE  48 /* Both resist Blind and Saves apply */
#define LRN_CONFU_SAVE  49 /* Both resist Confusion and Saves apply */
#define LRN_ALL         50 /* Recurses to all the resistables */

/*** Function flags ***/


#define PROJECT_NO          0
#define PROJECT_NOT_CLEAR   1
#define PROJECT_CLEAR       2

/*
 * Bit flags for the "monster_desc" function
 */
#define MDESC_OBJE	0x01	/* Objective (or Reflexive) */
#define MDESC_POSS	0x02	/* Possessive (or Reflexive) */
#define MDESC_IND1	0x04	/* Indefinites for hidden monsters */
#define MDESC_IND2	0x08	/* Indefinites for visible monsters */
#define MDESC_PRO1	0x10	/* Pronominalize hidden monsters */
#define MDESC_PRO2	0x20	/* Pronominalize visible monsters */
#define MDESC_HIDE	0x40	/* Assume the monster is hidden */
#define MDESC_SHOW	0x80	/* Assume the monster is visible */


/*
 * Bit flags for the "get_item" function
 */
#define USE_EQUIP	0x01	/* Allow equip items */
#define USE_INVEN	0x02	/* Allow inven items */
#define USE_FLOOR	0x04	/* Allow floor items */
#define USE_TARGET	0x08	/* Allow targeted floor items */
#define CAN_SQUELCH	0x10	/* Allow selection of all squelched items */
#define IS_HARMLESS     0x20	/* Ignore generic warning inscriptions */
#define SHOW_PRICES     0x40	/* Show item prices in item lists */
#define SHOW_FAIL       0x80 	/* Show device failure in item lists */
#define QUIVER_TAGS     0x100  /* 0-9 are quiver slots when selecting */

/*** Player flags ***/


/*
 * Bit flags for the "p_ptr->notice" variable
 */
#define PN_COMBINE	0x00000001L	/* Combine the pack */
#define PN_REORDER	0x00000002L	/* Reorder the pack */
#define PN_AUTOINSCRIBE	0x00000004L	/* Autoinscribe items */
#define PN_PICKUP       0x00000008L	/* Pick stuff up */
#define PN_SQUELCH      0x00000010L	/* Squelch stuff */
#define PN_SORT_QUIVER  0x00000020L     /* Sort the quiver */
/* xxx (many) */


/*
 * Bit flags for the "p_ptr->update" variable
 */
#define PU_BONUS	0x00000001L	/* Calculate bonuses */
#define PU_TORCH	0x00000002L	/* Calculate torch radius */
/* xxx (many) */
#define PU_HP		0x00000010L	/* Calculate chp and mhp */
#define PU_MANA		0x00000020L	/* Calculate csp and msp */
#define PU_SPELLS	0x00000040L	/* Calculate spells */
#define PU_SPECIALTY	0x00000080L	/* Calculate specialties */
/* xxx (many) */
#define PU_FORGET_VIEW	0x00010000L	/* Forget field of view */
#define PU_UPDATE_VIEW	0x00020000L	/* Update field of view */
/* xxx (many) */
#define PU_MONSTERS	0x10000000L	/* Update monsters */
#define PU_DISTANCE	0x20000000L	/* Update distances */
/* xxx */
#define PU_PANEL	0x80000000L	/* Update panel */


/*
 * Bit flags for the "p_ptr->redraw" variable
 */
#define PR_MISC		0x00000001L	/* Display Race/Class */
#define PR_TITLE	0x00000002L	/* Display Title */
#define PR_LEV		0x00000004L	/* Display Level */
#define PR_EXP		0x00000008L	/* Display Experience */
#define PR_STATS	0x00000010L	/* Display Stats */
#define PR_ARMOR	0x00000020L	/* Display Armor */
#define PR_HP		0x00000040L	/* Display Hitpoints */
#define PR_MANA		0x00000080L	/* Display Mana */
#define PR_GOLD		0x00000100L	/* Display Gold */
#define PR_DEPTH	0x00000200L	/* Display Depth */
#define PR_SHAPE	0x00000400L	/* Display Shape.  -LM- */
#define PR_HEALTH	0x00000800L	/* Display Health Bar */
#define PR_INVEN	0x00001000L /* Display inven/equip */
#define PR_EQUIP	0x00002000L /* Display equip/inven */
#define PR_MESSAGE	0x00004000L /* Display messages */
#define PR_MONSTER	0x00008000L /* Display monster recall */
#define PR_OBJECT	0x00010000L /* Display object recall */
#define PR_MONLIST	0x00020000L /* Display monster list */

#define PR_ITEMLIST     0x00080000L /* Display item list */
#define PR_STATE	0x00100000L	/* Display Extra (State) */
#define PR_SPEED	0x00200000L	/* Display Extra (Speed) */
#define PR_STUDY	0x00400000L	/* Display Extra (Study) */
#define PR_DTRAP        0x00800000L     /* Display Extra (DTrap) */

#define PR_MON_MANA	0x04000000L	/* Display Mana Bar */
#define PR_MAP		0x08000000L	/* Display Map */
#define PR_WIPE         0x10000000L     /* Hack -- Total Redraw */

#define PR_STATUS	0x80000000L /* Display extra status messages */

/* Display Basic Info */
#define PR_BASIC \
	(PR_MISC | PR_TITLE | PR_STATS | PR_LEV |\
	 PR_EXP | PR_GOLD | PR_ARMOR | PR_HP |\
	 PR_MANA | PR_DEPTH | PR_HEALTH | PR_SPEED)

/* Display Extra Info */
#define PR_EXTRA \
	(PR_STATUS | PR_STATE | PR_STUDY)

/*
 * Bit flags for the "p_ptr->window" variable (etc)
 */
#define PW_INVEN	0x00000001L	/* Display inven/equip */
#define PW_EQUIP	0x00000002L	/* Display equip/inven */
#define PW_PLAYER_0	0x00000004L	/* Display player (basic) */
#define PW_PLAYER_1	0x00000008L	/* Display player (extra) */
#define PW_PLAYER_2     0x00000010L     /* Display player (compact) */
#define PW_MAP          0x00000020L     /* Display dungeon map */
#define PW_MESSAGE	0x00000040L	/* Display messages */
#define PW_OVERHEAD	0x00000080L	/* Display overhead view */
#define PW_MONSTER	0x00000100L	/* Display monster recall */
#define PW_OBJECT	0x00000200L	/* Display object recall */
#define PW_DUNGEON      0x00000400L     /* Display dungeon view */
#define PW_SNAPSHOT	0x00000800L	/* Display snap-shot */
#define PW_MONLIST      0x00001000L     /* Display monster list */
#define PW_ITEMLIST     0x00002000L     /* Display item list */
#define PW_BORG_1	0x00004000L	/* Display borg messages */
#define PW_BORG_2	0x00008000L	/* Display borg status */

#define PW_MAX_FLAGS		16

/*
 * Bit flags for the "p_ptr->special_attack" variable. -LM-
 *
 * Note:  The elemental and poison attacks should be managed using the 
 * function "set_ele_attack", in spell2.c.  This provides for timeouts and
 * prevents the player from getting more than one at a time.
 */
#define ATTACK_NORMAL           0x00000000
#define ATTACK_CONFUSE		0x00000001
#define ATTACK_BLKBRTH		0x00000002
#define ATTACK_FLEE		0x00000004
#define ATTACK_SUPERSHOT	0x00000008
#define ATTACK_ACID		0x00000010
#define ATTACK_ELEC		0x00000020
#define ATTACK_FIRE		0x00000040
#define ATTACK_COLD		0x00000080
#define ATTACK_POIS		0x00000100
#define ATTACK_HOLY		0x00000200

#define ATTACK_DRUID_CONFU	0x00010000

/* Special attack states that may be displayed on screen */
#define ATTACK_NOTICE		0x000003ff

/*
 * Values for shapechanges.  From Sangband.
 * As of FAangband 1.0.0 shapes must be consecutive
 */
#define SHAPE_NORMAL   0			/* Unaltered form. */
#define SHAPE_MOUSE    1
#define SHAPE_FERRET   2
#define SHAPE_HOUND    3
#define SHAPE_GAZELLE  4
#define SHAPE_LION     5
#define SHAPE_ENT      6
#define SHAPE_BAT      7
#define SHAPE_WEREWOLF 8
#define SHAPE_VAMPIRE  9
#define SHAPE_WYRM    10
#define SHAPE_BEAR    11
#define MAX_SHAPE     11

#define SCHANGE \
    (p_ptr->schange > 0 && p_ptr->schange < 12)



/*** Object flags ***/


/*
 * Chest trap flags (see "tables.c")
 */
#define CHEST_LOSE_STR		0x0001
#define CHEST_LOSE_CON		0x0002
#define CHEST_POISON		0x0004
#define CHEST_PARALYZE		0x0008
#define CHEST_EXPLODE		0x0010
#define CHEST_SUMMON		0x0020
#define CHEST_SCATTER		0x0040
#define CHEST_E_SUMMON		0x0080
#define CHEST_BIRD_STORM	0x0100
#define CHEST_H_SUMMON		0x0200
#define CHEST_RUNES_OF_EVIL	0x0400




/*
 * Game-generated feelings.  Used for inscriptions.
 */
#define FEEL_NONE              0
#define FEEL_DUBIOUS_STRONG    1
#define FEEL_PERILOUS          2
#define FEEL_DUBIOUS_WEAK      3
#define FEEL_AVERAGE           4
#define FEEL_GOOD_STRONG       5
#define FEEL_EXCELLENT         6
#define FEEL_GOOD_WEAK         7
#define FEEL_SPECIAL           8
#define FEEL_MAX               9


/*** Macro Definitions ***/


/**
 * Hack -- The main "screen"
 */
#define term_screen	(angband_term[0])

/**
 * Some monster types are different.
 */
#define monster_is_unusual(R) \
	(flags_test((R)->flags, RF_SIZE, RF_DEMON, RF_UNDEAD, RF_STUPID, FLAG_END) || \
	strchr("Evg", (R)->d_char))

/**
 * Convert a "location" (Y,X) into a "grid" (G)
 */
#define GRID(Y,X) \
	(256 * (Y) + (X))

/**
 * Convert a "grid" (G) into a "location" (Y)
 */
#define GRID_Y(G) \
	((int)((G) / 256U))

/**
 * Convert a "grid" (G) into a "location" (X)
 */
#define GRID_X(G) \
	((int)((G) % 256U))


/**
 * Convert a "key event" into a "location" (Y)
 */
#define KEY_GRID_Y(K) \
  ((int) (((K.mouse.y - ROW_MAP) / tile_height) + Term->offset_y))

/**
 * Convert a "key event" into a "location" (X)
 */
#define KEY_GRID_X(K) \
	((int) (((K.mouse.x - COL_MAP) / tile_width) + Term->offset_x))

/**
 * Determines if a map location is "meaningful"
 */
#define in_bounds(Y,X) \
	(((unsigned)(Y) < (unsigned)(DUNGEON_HGT)) && \
	 ((unsigned)(X) < (unsigned)(DUNGEON_WID)))

/**
 * Determines if a map location is fully inside the outer walls
 * This is more than twice as expensive as "in_bounds()", but
 * often we need to exclude the outer walls from calculations.
 */
#define in_bounds_fully(Y,X) \
	(((Y) > 0) && ((Y) < DUNGEON_HGT-1) && \
	 ((X) > 0) && ((X) < DUNGEON_WID-1))


/*** Hack ***/

/**
 * Max number of terminal windows -CN-
 */
#define TERM_WIN_MAX 8

/**
 * Max number of lines of notes
 */
#define DUMP_MAX_LINES 5000

#define SCAN_INSTANT ((u32b) -1)
#define SCAN_OFF 0


/** Max number of items in the itemlist */
#define MAX_ITEMLIST 256

/*
 * Action types (for remembering what the player did)
 * A system horribly stolen from Sil
 */
#define ACTION_MAX		6	// number of actions stored

#define ACTION_NOTHING	0	// the default, showing that no action has yet been written

#define ACTION_MISC		10  // Currently accounts only for movement, as that's the only thing we care about. 1 for each direction.

#endif /* !INCLUDED_DEFINES_H */
