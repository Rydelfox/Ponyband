/*
 * File: x-spell.c
 * Purpose: Spell effect definitions and information about them
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "effects.h"
#include "target.h"
#include "tvalsval.h"
#include "spells.h"

/*
 * The defines below must match the spell numbers in spell.txt
 * if they don't, "interesting" things will probably happen.
 *
 * It would be nice if we could get rid of this dependency.
 */
/* Original entries 
#define SPELL_FIRE_BOLT                           0
#define SPELL_DETECT_MONSTERS                     1
#define SPELL_PHASE_DOOR                          2
#define SPELL_DETECT_TRAPS_DOORS_STAIRS           3
#define SPELL_LIGHT_AREA                          4
#define SPELL_STINKING_CLOUD                      5
#define SPELL_REDUCE_CUTS_AND_POISON              6
#define SPELL_RESIST_MAGIC                        7
#define SPELL_IDENTIFY                            8
#define SPELL_LIGHTNING_BOLT                      9
#define SPELL_CONFUSE_MONSTER                    10
#define SPELL_TELEKINESIS                        11
#define SPELL_SLEEP_MONSTER                      12
#define SPELL_TELEPORT_SELF                      13
#define SPELL_SPEAR_OF_LIGHT                     14
#define SPELL_FROST_BEAM                         15
#define SPELL_MAGICAL_THROW                      16
#define SPELL_SATISFY_HUNGER                     17
#define SPELL_DETECT_INVISIBLE                   18
#define SPELL_MAGIC_DISARM                       19
#define SPELL_BLINK_MONSTER                      20
#define SPELL_CURE                               21
#define SPELL_DETECT_ENCHANTMENT                 22
#define SPELL_STONE_TO_MUD                       23
#define SPELL_MINOR_RECHARGE                     24
#define SPELL_SLEEP_MONSTERS                     25
#define SPELL_THRUST_AWAY                        26
#define SPELL_FIRE_BALL                          27
#define SPELL_TAP_MAGICAL_ENERGY                 28
#define SPELL_SLOW_MONSTER                       29
#define SPELL_TELEPORT_OTHER                     30
#define SPELL_HASTE_SELF                         31
#define SPELL_HOLD_MONSTERS                      32
#define SPELL_CLEAR_MIND                         33
#define SPELL_RESIST_ELEMENT                     34
#define SPELL_SHIELD                             35
#define SPELL_RESISTANCE                         36
#define SPELL_ESSENCE_OF_SPEED                   37
#define SPELL_STRENGTHEN_DEFENCES                38
#define SPELL_DOOR_CREATION                      39
#define SPELL_STAIR_CREATION                     40
#define SPELL_TELEPORT_LEVEL                     41
#define SPELL_WORD_OF_RECALL                     42
#define SPELL_WORD_OF_DESTRUCTION                43
#define SPELL_DIMENSION_DOOR                     44
#define SPELL_ACID_BOLT                          45
#define SPELL_POLYMORPH_OTHER                    46
#define SPELL_EARTHQUAKE                         47
#define SPELL_BEGUILING                          48
#define SPELL_STARBURST                          49
#define SPELL_MAJOR_RECHARGE                     50
#define SPELL_CLOUD_KILL                         51
#define SPELL_ICE_STORM                          52
#define SPELL_METEOR_SWARM                       53
#define SPELL_CACOPHONY                          54
#define SPELL_UNLEASH_CHAOS                      55
#define SPELL_WALL_OF_FORCE                      56
#define SPELL_RUNE_OF_THE_ELEMENTS               57
#define SPELL_RUNE_OF_MAGIC_DEFENCE              58
#define SPELL_RUNE_OF_INSTABILITY                59
#define SPELL_RUNE_OF_MANA                       60
#define SPELL_RUNE_OF_PROTECTION                 61
#define SPELL_RUNE_OF_POWER                      62
#define SPELL_RUNE_OF_SPEED                      63
#define PRAYER_DETECT_EVIL                       64
#define PRAYER_CURE_LIGHT_WOUNDS                 65
#define PRAYER_BLESS                             66
#define PRAYER_REMOVE_FEAR                       67
#define PRAYER_CALL_LIGHT                        68
#define PRAYER_FIND_TRAPS                        69
#define PRAYER_DETECT_DOORS_STAIRS               70
#define PRAYER_SLOW_POISON                       71
#define PRAYER_CURE_SERIOUS_WOUNDS               72
#define PRAYER_SCARE_MONSTER                     73
#define PRAYER_PORTAL                            74
#define PRAYER_CHANT                             75
#define PRAYER_SANCTUARY                         76
#define PRAYER_SATISFY_HUNGER                    77
#define PRAYER_REMOVE_CURSE                      78
#define PRAYER_RESIST_HEAT_AND_COLD              79
#define PRAYER_NEUTRALIZE_POISON                 80
#define PRAYER_ORB_OF_DRAINING                   81
#define PRAYER_SENSE_INVISIBLE                   82
#define PRAYER_PROTECTION_FROM_EVIL              83
#define PRAYER_CURE_MORTAL_WOUNDS                84
#define PRAYER_EARTHQUAKE                        85
#define PRAYER_SENSE_SURROUNDINGS                86
#define PRAYER_TURN_UNDEAD                       87
#define PRAYER_PRAYER                            88
#define PRAYER_DISPEL_UNDEAD                     89
#define PRAYER_HEAL                              90
#define PRAYER_DISPEL_EVIL                       91
#define PRAYER_SACRED_SHIELD                     92
#define PRAYER_GLYPH_OF_WARDING                  93
#define PRAYER_HOLY_WORD                         94
#define PRAYER_BLINK                             95
#define PRAYER_TELEPORT_SELF                     96
#define PRAYER_TELEPORT_OTHER                    97
#define PRAYER_TELEPORT_LEVEL                    98
#define PRAYER_WORD_OF_RECALL                    99
#define PRAYER_ALTER_REALITY                    100
#define PRAYER_DETECT_MONSTERS                  101
#define PRAYER_DETECTION                        102
#define PRAYER_PERCEPTION                       103
#define PRAYER_PROBING                          104
#define PRAYER_CLAIRVOYANCE                     105
#define PRAYER_BANISHMENT                       106
#define PRAYER_HEALING                          107
#define PRAYER_SACRED_KNOWLEDGE                 108
#define PRAYER_RESTORATION                      109
#define PRAYER_REMEMBRANCE                      110
#define PRAYER_UNBARRING_WAYS                   111
#define PRAYER_RECHARGING                       112
#define PRAYER_DISPEL_CURSE                     113
#define PRAYER_DISARM_TRAP                      114
#define PRAYER_HOLDING                          115
#define PRAYER_ENCHANT_WEAPON_OR_ARMOUR         116
#define PRAYER_LIGHT_OF_MANWE                   117
#define PRAYER_LANCE_OF_OROME                   118
#define PRAYER_HAMMER_OF_AULE                   119
#define PRAYER_STRIKE_OF_MANDOS                 120
#define PRAYER_CALL_ON_VARDA                    121
#define PRAYER_ELEMENTAL_INFUSION               122
#define PRAYER_SANCTIFY_FOR_BATTLE              123
#define PRAYER_HORN_OF_WRATH                    124
#define SPELL_HIT_AND_RUN                       125
#define SPELL_DAY_OF_MISRULE                    126
#define SPELL_DETECT_TREASURE                   127
#define LORE_DETECT_LIFE                        128
#define LORE_CALL_LIGHT                         129
#define LORE_FORAGING                           130
#define LORE_BLINK                              131
#define LORE_COMBAT_POISON                      132
#define LORE_LIGHTNING_SPARK                    133
#define LORE_DOOR_DESTRUCION                    134
#define LORE_TURN_STONE_TO_MUD                  135
#define LORE_RAY_OF_SUNLIGHT                    136
#define LORE_CURE_POISON                        137
#define LORE_FROST_BOLT                         138
#define LORE_SLEEP_CREATURE                     139
#define LORE_FRIGHTEN_CREATURE                  140
#define LORE_DETECT_TRAPS_DOORS                 141
#define LORE_SNUFF_SMALL_LIFE                   142
#define LORE_FIRE_BOLT                          143
#define LORE_HEROISM                            144
#define LORE_REMOVE_CURSE                       145
#define LORE_ACID_BOLT                          146
#define LORE_TELEPORT_MONSTER                   147
#define LORE_GRAVITY_BOLT                       148
#define LORE_RESIST_POISON                      149
#define LORE_EARTHQUAKE                         150
#define LORE_RESIST_FIRE_AND_COLD               151
#define LORE_DETECT_ALL                         152
#define LORE_NATURAL_VITALITY                   153
#define LORE_RESIST_ACID_AND_LIGHTNING          154
#define LORE_WITHER_FOE                         155
#define LORE_DISARM_TRAP                        156
#define LORE_IDENTIFY                           157
#define LORE_CREATE_ATHELAS                     158
#define LORE_RAGING_STORM                       159
#define LORE_THUNDERCLAP                        160
#define LORE_BECOME_MOUSE                       161
#define LORE_BECOME_FERRET                      162
#define LORE_BECOME_HOUND                       163
#define LORE_BECOME_GAZELLE                     164
#define LORE_BECOME_LION                        165
#define LORE_DETECT_EVIL                        166
#define LORE_SONG_OF_FRIGHTENING                167
#define LORE_SENSE_SURROUNDINGS                 168
#define LORE_SIGHT_BEYOND_SIGHT                 169
#define LORE_HERBAL_HEALING                     170
#define LORE_BLIZZARD                           171
#define LORE_TRIGGER_TSUNAMI                    172
#define LORE_VOLCANIC_ERUPTION                  173
#define LORE_MOLTEN_LIGHTNING                   174
#define LORE_STARBURST                          175
#define LORE_SONG_OF_LULLING                    176
#define LORE_SONG_OF_PROTECTION                 177
#define LORE_SONG_OF_DISPELLING                 178
#define LORE_SONG_OF_WARDING                    179
#define LORE_SONG_OF_RENEWAL                    180
#define LORE_WEB_OF_VAIRE                       181
#define LORE_SPEED_OF_NESSA                     182
#define LORE_RENEWAL_OF_VANA                    183
#define LORE_SERVANT_OF_YAVANNA                 184
#define LORE_TEARS_OF_NIENNA                    185
#define LORE_HEALING_OF_ESTE                    186
#define LORE_CREATURE_KNOWLEDGE                 187
#define LORE_NATURES_VENGEANCE                  188
#define LORE_SONG_OF_GROWTH                     189
#define LORE_SONG_OF_PRESERVATION               190
#define LORE_TREMOR                             191
#define RITUAL_NETHER_BOLT                      192
#define RITUAL_DETECT_EVIL                      193
#define RITUAL_ENHANCED_INFRAVISION             194
#define RITUAL_BREAK_CURSE                      195
#define RITUAL_SLOW_MONSTER                     196
#define RITUAL_SLEEP_MONSTER                    197
#define RITUAL_HORRIFY                          198
#define RITUAL_BECOME_BAT                       199
#define RITUAL_DOOR_DESTRUCTION                 200
#define RITUAL_DARK_BOLT                        201
#define RITUAL_NOXIOUS_FUMES                    202
#define RITUAL_TURN_UNDEAD                      203
#define RITUAL_TURN_EVIL                        204
#define RITUAL_CURE_POISON                      205
#define RITUAL_DISPEL_UNDEAD                    206
#define RITUAL_DISPEL_EVIL                      207
#define RITUAL_SEE_INVISIBLE                    208
#define RITUAL_SHADOW_SHIFTING                  209
#define RITUAL_DETECT_TRAPS                     210
#define RITUAL_DETECT_DOORS_STAIRS              211
#define RITUAL_SLEEP_MONSTERS                   212
#define RITUAL_SLOW_MONSTERS                    213
#define RITUAL_DETECT_MAGIC                     214
#define RITUAL_DEATH_BOLT                       215
#define RITUAL_RESIST_POISON                    216
#define RITUAL_EXORCISE_DEMONS                  217
#define RITUAL_DARK_SPEAR                       218
#define RITUAL_CHAOS_STRIKE                     219
#define RITUAL_GENOCIDE                         220
#define RITUAL_DARK_BALL                        221
#define RITUAL_STENCH_OF_DEATH                  222
#define RITUAL_PROBING                          223
#define RITUAL_SHADOW_MAPPING                   224
#define RITUAL_IDENTIFY                         225
#define RITUAL_SHADOW_WARPING                   226
#define RITUAL_POISON_AMMO                      227
#define RITUAL_RESIST_ACID_AND_COLD             228
#define RITUAL_HEAL_ANY_WOUND                   229
#define RITUAL_PROTECTION_FROM_EVIL             230
#define RITUAL_BLACK_BLESSING                   231
#define RITUAL_BANISH_EVIL                      232
#define RITUAL_SHADOW_BARRIER                   233
#define RITUAL_DETECT_ALL_MONSTERS              234
#define RITUAL_STRIKE_AT_LIFE                   235
#define RITUAL_ORB_OF_DEATH                     236
#define RITUAL_DISPEL_LIFE                      237
#define RITUAL_VAMPIRIC_DRAIN                   238
#define RITUAL_RECHARGING                       239
#define RITUAL_BECOME_WEREWOLF                  240
#define RITUAL_DISPEL_CURSE                     241
#define RITUAL_BECOME_VAMPIRE                   242
#define RITUAL_HASTE_SELF                       243
#define RITUAL_PREPARE_BLACK_BREATH             244
#define RITUAL_WORD_OF_DESTRUCTION              245
#define RITUAL_TELEPORT_AWAY                    246
#define RITUAL_SMASH_UNDEAD                     247
#define RITUAL_BIND_UNDEAD                      248
#define RITUAL_DARKNESS_STORM                   249
#define RITUAL_MENTAL_AWARENESS                 250
#define RITUAL_SLIP_INTO_THE_SHADOWS            251
#define RITUAL_BLOODWRATH                       252
#define RITUAL_REBALANCE_WEAPON                 253
#define RITUAL_DARK_POWER                       254
*/

#define SPELL_ACID_BOLT							0
#define SPELL_DETECT_MONSTERS					1
#define	SPELL_PHASE_DOOR						2
#define SPELL_FIND_TRAPS_DOORS_STAIRS			3
#define SPELL_LIGHT_AREA						4
#define SPELL_STINKING_CLOUD					5
#define SPELL_REDUCE_CUTS_POISON				6
#define SPELL_RESIST_MAGIC						7
#define SPELL_IDENTIFY							8
#define SPELL_FIRE_BOLT							9
#define SPELL_CONFUSE_MONSTER					10
#define SPELL_TELEKINESIS						11
#define SPELL_SLEEP_MONSTER						12
#define SPELL_TELEPORT_SELF						13
#define	SPELL_HEROISM							14
#define SPELL_HEAT_BEAM							15
#define SPELL_MAGICAL_THROW						16
#define SPELL_SATISFY_HUNGER					17
#define SPELL_DETECT_INVISIBLE					18
#define SPELL_PROBING							19
#define SPELL_BLINK_MONSTER						20
#define SPELL_CURE								21
#define SPELL_DETECT_ENCHANTMENT				22
#define SPELL_STONE_TO_MUD						23
#define SPELL_MINOR_RECHARGE					24
#define SPELL_SLEEP_MONSTERS					25
#define SPELL_THRUST_AWAY						26
#define SPELL_FIRE_BALL							27
#define SPELL_TAP_MAGICAL_ENERGY				28
#define SPELL_SLOW_MONSTER						29
#define SPELL_TELEPORT_OTHER					30
#define SPELL_HASTE_SELF						31
#define SPELL_HOLD_MONSTERS						32
#define SPELL_FRIENDSHIP						33
#define SPELL_RESIST_ELEMENT					34
#define SPELL_SHIELD							35
#define SPELL_RESISTANCE						36
#define SPELL_STRENGTHEN_DEFENSES				37
#define SPELL_DOOR_CREATION						38
#define SPELL_SHINING_ARMORS_SHIELD				39
#define SPELL_STAIR_CREATION					40
#define SPELL_TELEPORT_LEVEL					41
#define SPELL_WORD_OF_RECALL					42
#define SPELL_DIMENSION_DOOR					43
#define SPELL_HIT_AND_RUN						44
#define SPELL_POLYMORPH_OTHER					45
#define SPELL_EARTHQUAKE						46
#define SPELL_BEGUILING							47
#define SPELL_FOCUS								48
#define SPELL_MAJOR_RECHARGE					49
#define SPELL_CLOUD_KILL						50
#define SPELL_FORCE_BEAM						51
#define SPELL_WORD_OF_DESTRUCTION				52
#define SPELL_METEOR_SWARM						53
#define SPELL_ROYAL_CANTERLOT_VOICE				54
#define SPELL_WALL_OF_FORCE						55
#define SPELL_RUNE_OF_THE_ELEMENTS				56
#define SPELL_RUNE_OF_MAGIC_INFLUENCE			57
#define SPELL_EXPLOSIVE_RUNE					58
#define SPELL_RUNE_OF_INSTABILITY				59
#define SPELL_RUNE_OF_MANA						60
#define SPELL_RUNE_OF_PROTECTION				61
#define SPELL_RUNE_OF_POWER						62
#define SPELL_RUNE_OF_SPEED						63
#define PRAYER_DETECT_CHAOS						64
#define PRAYER_CURE_MINOR_WOUNDS				65
#define PRAYER_CELESTIAS_BLESSING				66
#define PRAYER_REMOVE_FEAR						67
#define PRAYER_SUNLANCE							68
#define PRAYER_PURIFY							69
#define PRAYER_RESIST_FIRE						70
#define PRAYER_HEROISM							71
#define PRAYER_CURE_MEDIUM_WOUNDS				72
#define PRAYER_ABSORB_LIGHT						73
#define PRAYER_DETECT_UNDEAD					74
#define PRAYER_SATISFY_HUNGER					75
#define PRAYER_TURN_UNDEAD						76
#define PRAYER_SENSE_MINDS						77
#define PRAYER_REMOVE_CURSE						78
#define PRAYER_EULOGY							79
#define PRAYER_SOLAR_RAY						80
#define PRAYER_DISPEL_NIGHT						81
#define PRAYER_TORCH							82
#define PRAYER_BRIGHTEN							83
#define PRAYER_CURE_MORTAL_WOUNDS				84
#define PRAYER_SOLAR_FLARE						85
#define PRAYER_JUDGMENT							86
#define PRAYER_FINAL_REST						87
#define PRAYER_HEAL								88
#define PRAYER_BREAKING							89
#define PRAYER_HOLDING							90
#define PRAYER_BANISHMENT						91
#define PRAYER_HORN_OF_WRATH					92
#define PRAYER_HEALING							93
#define PRAYER_RENEWING_DAWN					94
#define PRAYER_CELESTIAS_GRACE					95
#define PRAYER_BLINK							96
#define PRAYER_TELEPORT_SELF					97
#define PRAYER_PORTAL							98
#define PRAYER_WORD_OF_RECALL					99
#define PRAYER_TELEPORT_LEVEL					100
#define PRAYER_TELEPORT_OTHER					101
#define PRAYER_ALTER_REALITY					102
#define PRAYER_DETECT_DOORS_STAIRS				103
#define PRAYER_PERCEPTION						104
#define PRAYER_PROBING							105
#define PRAYER_SACRED_KNOWLEDGE					106
#define PRAYER_DETECTION						107
#define PRAYER_CLAIRVOYANCE						108
#define PRAYER_SACRED_SHIELD					109
#define PRAYER_RIGHTEOUS_FURY					110
#define PRAYER_CURING							111
#define PRAYER_STEADFAST						112
#define PRAYER_SUSTAINING						113
#define PRAYER_RESTORATION						114
#define PRAYER_REMEMBERANCE						115
#define PRAYER_GLORY							116
#define PRAYER_RESIST_HEAT_COLD					117
#define PRAYER_PROTECTION_FROM_CHAOS			118
#define PRAYER_GLYPH_OF_WARDING					119
#define PRAYER_RALLY_ALLY						120
#define PRAYER_CIRCLE_OF_HARMONY				121
#define PRAYER_SANCTIFY_FOR_BATTLE				122
#define PRAYER_CHALLENGE						123
#define PRAYER_CONCENTRATE_LIGHT				124
#define PRAYER_HOLY_WORD						125
#define PRAYER_RADIANCE							126
#define PRAYER_ANNIHILATE_UNDEAD				127
#define LORE_LIGHTNING_SPARK					128
#define LORE_NATURE_AWARENESS					129
#define LORE_GRAZE								130
#define LORE_NATURAL_LIGHT						131
#define LORE_CLEAN_WOUND						132
#define LORE_RESIST_WEATHER						133
#define LORE_STONE_TO_TREE						134
#define LORE_FROST_BOLT							135
#define LORE_RESIST_BURNING_CORROSION			136
#define LORE_FOREST_CREATION					137
#define LORE_FLOOD								138
#define LORE_ENTANGLE							139
#define LORE_HERBAL_HEALING						140
#define LORE_SUNBOLT							141
#define LORE_CRUSHING_VINES						142
#define LORE_ICE_BEAM							143
#define LORE_FRIGHTEN_CREATURE					144
#define LORE_ROOT								145
#define LORE_FREEDOM_OF_MOVEMENT				146
#define LORE_CAMOUFLAGE							147
#define LORE_BARKSKIN							148
#define LORE_WOOD_WALL							149
#define LORE_HEROISM							150
#define LORE_NATURE_LORE						151
#define LORE_SPEED_OF_THE_CHEETAH				152
#define LORE_WALL_OF_STONE						153
#define LORE_SNUFF_SMALL_LIFE					154
#define LORE_CALM_ANIMALS						155
#define LORE_ICE_PATCH							156
#define LORE_STARBURST							157
#define LORE_SLEEP_POLLEN						158
#define LORE_THORNS								159
#define LORE_CYCLONE							160
#define LORE_POLYMORPH_MOUSE					161
#define LORE_POLYMORPH_FERRET					162
#define LORE_POLYMORPH_HOUND					163
#define LORE_POLYMORPH_GAZELLE					164
#define LORE_POLYMORPH_LION						165
#define LORE_POLYMORPH_TREE						166
#define LORE_INFRAVISION						167
#define LORE_CALL_LIGHT							168
#define LORE_DETECT_LIFE						169
#define LORE_DETECT_TRAPS_DOORS					170
#define LORE_CREATURE_KNOWLEDGE					171
#define LORE_SIGHT_BEYOND_SIGHT					172
#define LORE_TREMOR								173
#define LORE_ELEMENTAL_BRANDING					174
#define LORE_CHAIN_LIGHTNING					175
#define LORE_EARTHQUAKE							176
#define LORE_BLIZZARD							177
#define LORE_TRIGGER_TSUNAMI					178
#define LORE_SLEEP_CREATURE						179
#define LORE_NATURE_FRIEND						180
#define LORE_ANIMAL_TAMING						181
#define LORE_HEAL_ANIMALS						182
#define LORE_STARE								183
#define LORE_NATURAL_RENEWAL					184
#define LORE_NATURAL_VITALITY					185
#define LORE_WITHER_FOE							186
#define LORE_TELEPORT_MONSTER					187
#define LORE_WOOD_WALK							188
#define LORE_VOLCANO							189
#define LORE_NATURES_WRATH						190
#define SPELL_DETECT_TREASURE					191
#define SPELL_SLIP_INTO_SHADOWS					192
#define SPELL_SENSE_INVISIBLE					193
#define SPELL_MAGIC_MAPPING						194

/**
 * Same thing goes for innate abilities
 */
#define ABILITY_TELEKINESIS                     0
#define ABILITY_BR_FIRE                         1
#define ABILITY_DETECT_MONSTERS                 2
#define ABILITY_DETECT_TREASURE                 3



int get_spell_index(const object_type * o_ptr, int index)
{
	int spell = mp_ptr->book_start_index[o_ptr->sval] + index;
	if (spell > mp_ptr->book_start_index[o_ptr->sval + 1])
		return -1;

	return spell;
}

const char *get_spell_name(int sindex)
{
	return s_info[sindex].name;
}

/**
 * Extra information on a spell		-DRS-
 *
 * We can use up to 20 characters of the buffer 'p'
 *
 * The strings in this function were extracted from the code in the
 * functions "do_cmd_cast()" and "do_cmd_pray()" and are up to date 
 * (as of 0.4.0). -LM-
 */
void get_spell_info(int tval, int sindex, char *p, size_t len)
{
	int plev = p_ptr->lev;

	int beam, beam_low;

	/* Specialty Ability */
	if (player_has(PF_HEIGHTEN_MAGIC))
		plev += 1 + ((p_ptr->heighten_power + 5) / 10);
	if (player_has(PF_CHANNELING))
		plev += get_channeling_boost();
	if (p_ptr->timed[TMD_FOCUS])
	{
		plev *= 5;
		plev /= 4;
	}

	/* Beaming chance */
	beam = (((player_has(PF_BEAM))) ? plev : (plev / 2));
	beam_low = (beam - 10 > 0 ? beam - 10 : 0);

	/* Default */
	strcpy(p, "");

	/* Analyze the spell */
	switch (sindex) {
		/* Sorcery */
	case SPELL_ACID_BOLT:
		strnfmt(p, len, " dam %dd3", 4 + plev / 10);
		break;
	case SPELL_PHASE_DOOR:
		strnfmt(p, len, " range 10");
		break;
	case SPELL_LIGHT_AREA:
		strnfmt(p, len, " dam 2d%d, rad %d", 1 + (plev / 5),
				(plev / 10) + 1);
		break;
	case SPELL_STINKING_CLOUD:
		strnfmt(p, len, " dam %d, rad 2", 5 + (plev / 3));
		break;
	case SPELL_REDUCE_CUTS_POISON:
		strnfmt(p, len, " halve cuts and poison");
		break;
	case SPELL_RESIST_MAGIC:
		strnfmt(p, len, " dur 10+d5");
		break;
	case SPELL_FIRE_BOLT:
		strnfmt(p, len, " dam %dd8, beam %d%%", (3 + ((plev - 5) / 5)),
				beam);
		break;
	case SPELL_TELEPORT_SELF:
		strnfmt(p, len, " range %d", 50 + plev * 2);
		break;
	case SPELL_HEROISM:
 	    strnfmt(p, len, " dur 20+d20");
 	    break;
	case SPELL_HEAT_BEAM:
		strnfmt(p, len, " dam %d", 5 + plev);
		break;
	case SPELL_BLINK_MONSTER:
		strnfmt(p, len, " range %d", 5 + plev / 10);
		break;
	case SPELL_STONE_TO_MUD:
		strnfmt(p, len, " dam 2d%d", plev / 2);
		break;
	case SPELL_THRUST_AWAY:
		strnfmt(p, len, " dam %dd%d, length %d", 6 + (plev / 10), 8,
				1 + (plev / 10));
		break;
	case SPELL_FIRE_BALL:
		strnfmt(p, len, " dam %d, rad 2", 55 + plev);
		break;
	case SPELL_TELEPORT_OTHER:
		strnfmt(p, len, " dist %d", 45 + (plev / 2));
		break;
	case SPELL_HASTE_SELF:
		strnfmt(p, len, " dur %d+d20", plev);
		break;
	case SPELL_RESIST_ELEMENT:
		strnfmt(p, len, " dur %d+d%d", plev, plev);
		break;
	case SPELL_SHIELD:
		strnfmt(p, len, " dur 30+d20");
		break;
	case SPELL_RESISTANCE:
		strnfmt(p, len, " dur 25+d25");
		break;
	case SPELL_STRENGTHEN_DEFENSES:
		strnfmt(p, len, " dur 30");
		break;
	case SPELL_SHINING_ARMORS_SHIELD:
		strnfmt(p, len, " dur %d+d30", 10 + plev);
		break;
	case SPELL_EARTHQUAKE:
		strnfmt(p, len, " rad 12");
		break;
	case SPELL_FOCUS:
		strnfmt(p, len, " dur 10+d10");
		break;
	case SPELL_CLOUD_KILL:
		strnfmt(p, len, " dam %d,%d, rad 3,2", 10 + plev, 2 * plev);
		break;
	case SPELL_FORCE_BEAM:
		strnfmt(p, len, " dam %d", 3 * plev);
		break;
	case SPELL_WORD_OF_DESTRUCTION:
		strnfmt(p, len, " rad 15");
		break;
	case SPELL_METEOR_SWARM:
		strnfmt(p, len, " dam %d, rad 1", 80 + plev * 2);
		break;
	case SPELL_ROYAL_CANTERLOT_VOICE:
		strnfmt(p, len, " dam %d+3d%d", plev, plev);
		break;
	case SPELL_WALL_OF_FORCE:
		strnfmt(p, len, " dam %d, rad %d", 4 * plev, 3 + plev / 15);
		break;
	case SPELL_EXPLOSIVE_RUNE:
		strnfmt(p, len, " dam %d+3d%d, rad 3", 3 * plev, plev);
		break;


		/* Piety */
	case PRAYER_CURE_MINOR_WOUNDS:
		strnfmt(p, len, " heal 2d%d", plev / 4 + 5);
		break;
	case PRAYER_CELESTIAS_BLESSING:
		strnfmt(p, len, " dur 12+d12");
		break;
	case PRAYER_SUNLANCE:
		strnfmt(p, len, " dam %dd6, beam %d%%", 1 + (plev/8), beam);
		break;
	case PRAYER_RESIST_FIRE:
		strnfmt(p, len, " dur %d+d30", 2 * plev / 30);
		break;
	case PRAYER_HEROISM:
		strnfmt(p, len, " dur %d+d30", 2 * plev / 30);
		break;
	case PRAYER_CURE_MEDIUM_WOUNDS:
		strnfmt(p, len, " heal 4d%d each", plev / 4 + 6);
		break;
	case PRAYER_ABSORB_LIGHT:
		strnfmt(p, len, " heal 1d%d each", plev/6 + 3);
		break;
	case PRAYER_SENSE_MINDS:
		strnfmt(p, len, " dur 20+d20");
		break;
	case PRAYER_EULOGY:
		strnfmt(p, len, " dam 3d%d", plev);
		break;
	case PRAYER_SOLAR_RAY:
		strnfmt(p, len, " dam %d", plev / 2 + (plev / (player_has(PF_STRONG_MAGIC) ? 2 : 4)));
		break;
	case PRAYER_DISPEL_NIGHT:
		strnfmt(p, len, " dam d%d", 3 * plev);
		break;
	case PRAYER_TORCH:
		strnfmt(p, len, " dam 2d%d+%d, d%d, rad 2", plev / 2, plev/5, plev / 5 + plev/((player_has(PF_STRONG_MAGIC) ? 3 : 5)));
		break;
	case PRAYER_BRIGHTEN:
		strnfmt(p, len, " dam %dd6", plev/6);
		break;
	case PRAYER_CURE_MORTAL_WOUNDS:
		strnfmt(p, len, " heal 9d%d, any cut", plev / 3 + 12);
		break;
	case PRAYER_SOLAR_FLARE:
		strnfmt(p, len, " dam %dd8", 1 + plev / 5);
		break;
	case PRAYER_JUDGMENT:
		strnfmt(p, len, " dam %d, x1.5 vs Chaos", 3 * plev / 2);
		break;
	case PRAYER_HEAL:
		strnfmt(p, len, " 9d%d, any cut", plev / 3 + 12);
		break;
	case PRAYER_HORN_OF_WRATH:
		strnfmt(p, len, " dam %d+d%d, rad %d", plev, 60 + 2 * plev, 1 + plev / 10);
		break;
	case PRAYER_HEALING:
		strnfmt(p, len, " heal 700, any cut");
		break;
	case PRAYER_RENEWING_DAWN:
		strnfmt(p, len, " dam/heal 5d%d", plev);
		break;
	case PRAYER_CELESTIAS_GRACE:
		strnfmt(p, len, " dam %d, heal 400", 4 * plev);
		break;
	case PRAYER_BLINK:
		strnfmt(p, len, " range 10");
		break;
	case PRAYER_TELEPORT_SELF:
		strnfmt(p, len, " range %d", 4 * plev);
		break;
	case PRAYER_TELEPORT_OTHER:
		strnfmt(p, len, " dist %d", 45 + (plev / 3));
		break;
	case PRAYER_SACRED_SHIELD:
		strnfmt(p, len, " dur %d+d20", plev / 2);
		break;
	case PRAYER_RIGHTEOUS_FURY:
		strnfmt(p, len, " dur %d+d%d", 2 * plev / 5, 2 * plev / 5);
		break;
	case PRAYER_SUSTAINING:
		strnfmt(p, len, " dur 20+d20");
		break;
	case PRAYER_GLORY:
		strnfmt(p, len, " dur 50+d50");
		break;
	case PRAYER_RESIST_HEAT_COLD:
		strnfmt(p, len, " dur %d+d10", plev / 2);
		break;
	case PRAYER_PROTECTION_FROM_CHAOS:
		strnfmt(p, len, " dur %d+d24", 3 * plev / 2);
		break;
	case PRAYER_RALLY_ALLY:
		strnfmt(p, len, " heal %dd10", 25+plev/2);
		break;
	case PRAYER_CIRCLE_OF_HARMONY:
		strnfmt(p, len, " rad %d", plev / 15);
		break;
	case PRAYER_CONCENTRATE_LIGHT:
		strnfmt(p, len, " %dd4 each", plev/10);
		break;
	case PRAYER_HOLY_WORD:
		strnfmt(p, len, " dam d%d, heal 300", plev * 4);
		break;
	case PRAYER_RADIANCE:
		strnfmt(p, len, " dam %d, heal 500", 5 * plev);
		break;


		/* Nature Magics */

	case LORE_LIGHTNING_SPARK:
		strnfmt(p, len, " dam %dd3", 2 + plev / 8);
		break;
	case LORE_NATURAL_LIGHT:
		strnfmt(p, len, " dam %d", 2 + plev / 3);
		break;
	case LORE_CLEAN_WOUND:
		strnfmt(p, len, " heal any cut or poison");
		break;
	case LORE_RESIST_WEATHER:
		strnfmt(p, len, " dur 20+d20");
		break;
	case LORE_STONE_TO_TREE:
		strnfmt(p, len, " dam %d+d30", plev / 3);
		break;
	case LORE_FROST_BOLT:
		strnfmt(p, len, " dam %dd8, beam %d%%", 2 + plev / 5, beam_low);
		break;
	case LORE_RESIST_BURNING_CORROSION:
		strnfmt(p, len, " dur 20+d20");
		break;
	case LORE_FLOOD:
		strnfmt(p, len, " dam %d, rad %d", 10 + plev, 1 + plev / 11);
		break;
	case LORE_HERBAL_HEALING:
		strnfmt(p, len, " heal %dd12, any cut", 25 + plev / 2);
		break;
	case LORE_SUNBOLT:
		strnfmt(p, len, " dam 1d%d", 3 * plev);
		break;
	case LORE_CRUSHING_VINES:
		strnfmt(p, len, " dam 4/3/2d%d", plev);
		break;
	case LORE_ICE_BEAM:
		strnfmt(p, len, " dam %d", 5 + plev);
		break;
	case LORE_FREEDOM_OF_MOVEMENT:
		strnfmt(p, len, " dur 20+d20");
		break;
	case LORE_CAMOUFLAGE:
		strnfmt(p, len, " dur 40");
		break;
	case LORE_BARKSKIN:
		strnfmt(p, len, " dur 30+d30");
		break;
	case LORE_WOOD_WALL:
		strnfmt(p, len, " dam plev / 2");
		break;
	case LORE_HEROISM:
		strnfmt(p, len, " dur %d+d30", 2 * plev / 3);
		break;
	case LORE_SPEED_OF_THE_CHEETAH:
		strnfmt(p, len, " dur 10+d20");
		break;
	case LORE_SNUFF_SMALL_LIFE:
		strnfmt(p, len, " dam %d", 5 + plev / 5);
		break;
	case LORE_ICE_PATCH:
		strnfmt(p, len, " dam %d+2d%d, rad %d", plev, plev, 2 + plev / 11);
		break;
	case LORE_STARBURST:
		strnfmt(p, len, " dam %d+d%d, rad %d", 40 + (3 * plev / 2),
				plev * 3, plev / 10);
		break;
	case LORE_THORNS:
		strnfmt(p, len, " dam 3d%d", plev);
		break;
	case LORE_CYCLONE:
		strnfmt(p, len, " dam %d, rad %d", 2 * plev + 10, plev / 15);
		break;
	case LORE_INFRAVISION:
		strnfmt(p, len, " dur %d", plev + 10);
		break;
	case LORE_CALL_LIGHT:
		strnfmt(p, len, " dam %d, rad %d", 1 + plev / 5, plev / 10 + 1);
		break;
	case LORE_CHAIN_LIGHTNING:
		strnfmt(p, len, " dam %d+d%d", plev, 30 + (5 * plev / 6));
		break;
	case LORE_EARTHQUAKE:
		strnfmt(p, len, " radius 10");
		break;
	case LORE_BLIZZARD:
		strnfmt(p, len, " dam %d+d%d, rad %d", plev, 50 + plev * 2,
				1 + plev / 12);
		break;
	case LORE_TRIGGER_TSUNAMI:
		strnfmt(p, len, " dam %d+d%d, rad %d", 30 + ((4 * plev) / 5),
				30 + plev * 2, plev / 7);
		break;
	case LORE_HEAL_ANIMALS:
		strnfmt(p, len, " heal 4d%d", plev / 4 + 7);
		break;
	case LORE_WITHER_FOE:
		strnfmt(p, len, " dam %dd8, conf/slow", plev / 7);
		break;
	case LORE_TELEPORT_MONSTER:
		strnfmt(p, len, " dist %d", 45 + (plev / 3));
		break;
	case LORE_VOLCANO:
		strnfmt(p, len, " rad 12");
		break;
	case LORE_NATURES_WRATH:
		strnfmt(p, len, " dam %d+8d8", plev/2);
		break;
		
		/* Rogue */
	case SPELL_SLIP_INTO_SHADOWS:
		strnfmt(p, len, " dur 40");
		break;
	case SPELL_SENSE_INVISIBLE:
		strnfmt(p, len, " dur %d+d24", plev);
		break;
	}
}


bool spell_needs_aim(int tval, int spell)
{
	const magic_type *mt_ptr = &mp_ptr->info[spell];

	switch (mt_ptr->index) {
	case SPELL_ACID_BOLT:
	case SPELL_STINKING_CLOUD:
	case SPELL_FIRE_BOLT:
	case SPELL_CONFUSE_MONSTER:
	case SPELL_SLEEP_MONSTER:
	case SPELL_HEAT_BEAM:
	case SPELL_BLINK_MONSTER:
	case SPELL_STONE_TO_MUD:
	case SPELL_THRUST_AWAY:
	case SPELL_FIRE_BALL:
	case SPELL_SLOW_MONSTER:
	case SPELL_TELEPORT_OTHER:
	case SPELL_HOLD_MONSTERS:
	case SPELL_FRIENDSHIP:
	case SPELL_POLYMORPH_OTHER:
	case SPELL_CLOUD_KILL:
	case SPELL_FORCE_BEAM:
	case SPELL_METEOR_SWARM:
	case SPELL_WALL_OF_FORCE:
	case PRAYER_SUNLANCE:
	case PRAYER_SOLAR_RAY:
	case PRAYER_TORCH:
	case PRAYER_SOLAR_FLARE:
	case PRAYER_JUDGMENT:
	case PRAYER_FINAL_REST:
	case PRAYER_HOLDING:
	case PRAYER_TELEPORT_OTHER:
	case PRAYER_RALLY_ALLY:
	case PRAYER_CHALLENGE:
	case PRAYER_CONCENTRATE_LIGHT:
	case LORE_STONE_TO_TREE:
	case LORE_FROST_BOLT:
	case LORE_FLOOD:
	case LORE_SUNBOLT:
	case LORE_ICE_BEAM:
	case LORE_FRIGHTEN_CREATURE:
	case LORE_ROOT:
	case LORE_CYCLONE:
	case LORE_CHAIN_LIGHTNING:
	case LORE_BLIZZARD:
	case LORE_SLEEP_CREATURE:
	case LORE_ANIMAL_TAMING:
	case LORE_STARE:
	case LORE_WITHER_FOE:
	case LORE_TELEPORT_MONSTER:
		return TRUE;

	default:
		return FALSE;
	}
}

int align_cost_adjust(int spell)
{
	const magic_type *mt_ptr = &mp_ptr->info[spell];
	
	int multiplier = 100;
	
	switch (mt_ptr->index) {
	/* Spells that benefit from harmony */
	case SPELL_PROBING:
	case SPELL_FRIENDSHIP:
	case SPELL_SHIELD:
	case SPELL_STRENGTHEN_DEFENSES:
	case SPELL_SHINING_ARMORS_SHIELD:
	case SPELL_DIMENSION_DOOR:
	case SPELL_RUNE_OF_PROTECTION:
	case PRAYER_DETECT_CHAOS:
	case PRAYER_CURE_MINOR_WOUNDS:
	case PRAYER_CELESTIAS_BLESSING:
	case PRAYER_CURE_MEDIUM_WOUNDS:
	case PRAYER_CURE_MORTAL_WOUNDS:
	case PRAYER_JUDGMENT:
	case PRAYER_FINAL_REST:
	case PRAYER_HEAL:
	case PRAYER_HEALING:
	case PRAYER_PORTAL:
	case PRAYER_PROBING:
	case PRAYER_SACRED_SHIELD:
	case PRAYER_STEADFAST:
	case PRAYER_SUSTAINING:
	case PRAYER_RESTORATION:
	case PRAYER_REMEMBERANCE:
	case PRAYER_PROTECTION_FROM_CHAOS:
	case PRAYER_GLYPH_OF_WARDING:
	case PRAYER_RALLY_ALLY:
	case PRAYER_CIRCLE_OF_HARMONY:
	case PRAYER_HOLY_WORD:
	case PRAYER_ANNIHILATE_UNDEAD:
	case LORE_ROOT:
	case LORE_CALM_ANIMALS:
	case LORE_DETECT_LIFE:
	case LORE_NATURE_FRIEND:
	case LORE_ANIMAL_TAMING:
	case LORE_HEAL_ANIMALS:
	case LORE_NATURAL_RENEWAL:
	case LORE_NATURAL_VITALITY:
		{
	        multiplier = p_ptr->alignment * 100 / 3333 + 100;
		    break;
		}
	/* Spells that benefit from chaos */
	case SPELL_CONFUSE_MONSTER:
	case SPELL_TELEPORT_SELF:
	case SPELL_BLINK_MONSTER:
	case SPELL_TELEPORT_OTHER:
	case SPELL_WORD_OF_DESTRUCTION:
	case SPELL_EARTHQUAKE:
	case SPELL_BEGUILING:
	case SPELL_METEOR_SWARM:
	case PRAYER_BREAKING:
	case PRAYER_CELESTIAS_GRACE:
	case PRAYER_BLINK:
	case PRAYER_TELEPORT_SELF:
	case PRAYER_TELEPORT_OTHER:
	case PRAYER_ALTER_REALITY:
	case PRAYER_CONCENTRATE_LIGHT:
	case LORE_NATURAL_LIGHT:
	case LORE_FOREST_CREATION:
	case LORE_FRIGHTEN_CREATURE:
	case LORE_FREEDOM_OF_MOVEMENT:
	case LORE_CYCLONE:
	case LORE_POLYMORPH_MOUSE:
	case LORE_POLYMORPH_FERRET:
	case LORE_POLYMORPH_HOUND:
	case LORE_POLYMORPH_GAZELLE:
	case LORE_POLYMORPH_TREE:
	case LORE_POLYMORPH_LION:
	case LORE_TREMOR:
	case LORE_CHAIN_LIGHTNING:
	case LORE_EARTHQUAKE:
	case LORE_TELEPORT_MONSTER:
	case LORE_VOLCANO:
	case SPELL_SLIP_INTO_SHADOWS:
		{
		    multiplier = (-1 * p_ptr->alignment) * 100 / 3333 + 100;
		    break;
		}
	}
	
	return multiplier;
}

int spell_cost(int spell)
{
	const magic_type *s_ptr = &mp_ptr->info[spell];
	
	return (s_ptr->smana * align_cost_adjust(spell) / 100);
}

int spell_fail(int spell)
{
	const magic_type *s_ptr = &mp_ptr->info[spell];
	int fail_adjust = align_cost_adjust(spell);
	
	/* Invert it (i.e. 130% becomes 70%) */
	fail_adjust -= 100;
	fail_adjust = 100 - fail_adjust;
	
	return (s_ptr->sfail * fail_adjust / 100);
}

bool cast_spell(int tval, int sindex, int dir, int plev)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int shape = SHAPE_NORMAL;
	int beam;
	int beam_low;

	/* Hack -- higher chance of "beam" instead of "bolt" for mages and necros. */
	beam = ((player_has(PF_BEAM)) ? plev : (plev / 2));
	beam_low = (beam - 10 > 0 ? beam - 10 : 0);

	/* Spell Effects.  Spells are mostly listed by realm, each using a block of 
	 * 64 spell slots, but there are a certain number of spells that are used
	 * by more than one realm (this allows more neat class- specific magics) */
	switch (sindex) {
		/* Sorcerous Spells */

	case SPELL_ACID_BOLT:
		{
			fire_bolt(GF_ACID, dir, damroll(4 + plev / 10, 3));
			break;
		}
	case SPELL_DETECT_MONSTERS:
		{
			(void) detect_monsters_normal(DETECT_RAD_DEFAULT, TRUE);
			break;
		}
	case SPELL_PHASE_DOOR:
		{
			teleport_player(10, TRUE);
			break;
		}
	case SPELL_FIND_TRAPS_DOORS_STAIRS:
		{
			/* Hack - 'show' effected region only with the first detect */
			(void) detect_traps(DETECT_RAD_DEFAULT, TRUE);
			(void) detect_doors(DETECT_RAD_DEFAULT, FALSE);
			(void) detect_stairs(DETECT_RAD_DEFAULT, FALSE);
			break;
		}
	case SPELL_LIGHT_AREA:
		{
			(void) light_area(damroll(2, 1 + (plev / 5)), (plev / 10) + 1);
			break;
		}
	case SPELL_STINKING_CLOUD:
		{
			fire_ball(GF_POIS, dir, 5 + plev / 3, 2, FALSE);
			break;
		}
	case SPELL_REDUCE_CUTS_POISON:
		{
			(void) dec_timed(TMD_POISONED, p_ptr->timed[TMD_POISONED] / 2,
							 TRUE);
			(void) dec_timed(TMD_CUT, p_ptr->timed[TMD_CUT] / 2, TRUE);
			break;
		}
	case SPELL_RESIST_MAGIC:
		{
			if (!p_ptr->timed[TMD_INVULN]) {
				(void) inc_timed(TMD_INVULN, 10 + randint1(5), TRUE);
			} else {
				(void) inc_timed(TMD_INVULN, randint1(5), TRUE);
			}

			break;
		}
	case SPELL_IDENTIFY:
		{
			if (!ident_spell())
				return FALSE;
			break;
		}
	case SPELL_FIRE_BOLT:
		{
			fire_bolt_or_beam(beam, GF_FIRE, dir,
							  damroll(3 + ((plev - 5) / 5), 8));
			break;
		}
	case SPELL_CONFUSE_MONSTER:
		{
			(void) confuse_monster(dir, plev + 10);
			break;
		}
	case SPELL_TELEKINESIS:
		{
			s16b ty, tx;
			if (!target_set_interactive(TARGET_OBJ | TARGET_GRID, -1, -1))
				return FALSE;
			target_get(&tx, &ty);
			if (!py_pickup(1, ty, tx))
				return FALSE;
			break;
		}
	case SPELL_SLEEP_MONSTER:
		{
			(void) sleep_monster(dir, plev + 10);
			break;
		}
	case SPELL_TELEPORT_SELF:
		{
			teleport_player(50 + plev * 2, TRUE);
			break;
		}
	case SPELL_HEROISM:
		{
			(void) hp_player(20);
			if (!p_ptr->timed[TMD_HERO]) {
				(void) inc_timed(TMD_HERO, randint1(20) + 20, TRUE);
			} else {
				(void) inc_timed(TMD_HERO, randint1(10) + 10, TRUE);
			}
			(void) clear_timed(TMD_AFRAID, TRUE);

			break;
		}
	case SPELL_HEAT_BEAM:
		{
			fire_beam(GF_FIRE, dir, 5 + plev);
			break;
		}
	case SPELL_MAGICAL_THROW:
		{
			magic_throw = TRUE;
			textui_cmd_throw();
			magic_throw = FALSE;
			break;
		}
	case SPELL_SATISFY_HUNGER:
		{
			(void) set_food(PY_FOOD_MAX - 1);
			break;
		}
	case SPELL_DETECT_INVISIBLE:
		{
			(void) detect_monsters_invis(DETECT_RAD_DEFAULT, TRUE);
			break;
		}
	case SPELL_PROBING:
		{
			(void) probing();
			break;
		}
	case SPELL_BLINK_MONSTER:
		{
			(void) teleport_monster(dir, 5 + (plev / 10));
			break;
		}
	case SPELL_CURE:
		{
			(void) clear_timed(TMD_POISONED, TRUE);
			(void) clear_timed(TMD_CUT, TRUE);
			(void) clear_timed(TMD_STUN, TRUE);
			break;
		}
	case SPELL_DETECT_ENCHANTMENT:
		{
			(void) detect_objects_magic(DETECT_RAD_DEFAULT, TRUE);
			break;
		}
	case SPELL_STONE_TO_MUD:
		{
			(void) wall_to_mud(dir);
			break;
		}
	case SPELL_MINOR_RECHARGE:
		{
			if (!recharge(120))
				return FALSE;
			break;
		}
	case SPELL_SLEEP_MONSTERS:
		{
			(void) sleep_monsters(plev + 10);
			break;
		}
	case SPELL_THRUST_AWAY:
		{
			fire_arc(GF_FORCE, dir, damroll(6 + (plev / 10), 8),
					 (1 + plev / 10), 0);
			break;
		}
	case SPELL_FIRE_BALL:
		{
			fire_ball(GF_FIRE, dir, 55 + plev, 2, FALSE);
			break;
		}
	case SPELL_TAP_MAGICAL_ENERGY:
		{
			tap_magical_energy();
			break;
		}
	case SPELL_SLOW_MONSTER:
		{
			(void) slow_monster(dir, plev + 10);
			break;
		}
	case SPELL_TELEPORT_OTHER:
		{
			(void) teleport_monster(dir, 45 + (plev / 2));
			break;
		}
	case SPELL_HASTE_SELF:
		{
			if (!p_ptr->timed[TMD_FAST]) {
				(void) set_timed(TMD_FAST, randint1(20) + plev, TRUE);
			} else {
				(void) set_timed(TMD_FAST, randint1(5), TRUE);
			}
			break;
		}
	case SPELL_HOLD_MONSTERS:
		{
			fire_ball(GF_HOLD, dir, 0, 2, FALSE);
			break;
		}
	case SPELL_FRIENDSHIP:
		{
			(void) charm_monster(dir, plev + 15);
			break;
		}
	case SPELL_RESIST_ELEMENT:
		{
			if (!choose_ele_resist())
				return FALSE;
			break;
		}
	case SPELL_SHIELD:
		{
			if (!p_ptr->timed[TMD_SHIELD]) {
				(void) inc_timed(TMD_SHIELD, randint1(20) + 30, TRUE);
			} else {
				(void) inc_timed(TMD_SHIELD, randint1(10) + 15, TRUE);
			}
			break;
		}
	case SPELL_RESISTANCE:
		{
			int time = randint1(25) + 25;
			(void) inc_timed(TMD_OPP_ACID, time, TRUE);
			(void) inc_timed(TMD_OPP_ELEC, time, TRUE);
			(void) inc_timed(TMD_OPP_FIRE, time, TRUE);
			(void) inc_timed(TMD_OPP_COLD, time, TRUE);
			(void) inc_timed(TMD_OPP_POIS, time, TRUE);
			break;
		}
	case SPELL_STRENGTHEN_DEFENSES:
		{
			if (!p_ptr->timed[TMD_INVULN]) {
				(void) inc_timed(TMD_INVULN, 40, TRUE);
			} else {
				(void) inc_timed(TMD_INVULN, randint1(20), TRUE);
			}

			break;
		}
	case SPELL_DOOR_CREATION:
		{
			(void) door_creation();
			break;
		}
	case SPELL_SHINING_ARMORS_SHIELD:
		{
			if (!p_ptr->timed[TMD_MANASHIELD])
			{
				(void) inc_timed(TMD_MANASHIELD, 10 + plev + randint1(30), TRUE);
			} else {
				(void) inc_timed(TMD_MANASHIELD, randint1(15), TRUE);
			}
			
			break;
		}
	case SPELL_STAIR_CREATION:
		{
			(void) stair_creation();
			break;
		}
	case SPELL_TELEPORT_LEVEL:
		{
			(void) teleport_player_level(TRUE);
			break;
		}
	case SPELL_WORD_OF_RECALL:
		{
			if (!word_recall(randint0(20) + 15))
				break;
			break;
		}
	case SPELL_WORD_OF_DESTRUCTION:
		{
			destroy_area(py, px, 15, TRUE);
			break;
		}
	case SPELL_DIMENSION_DOOR:
		{
			msg("Choose a location to teleport to.");
			message_flush();
			dimen_door();
			break;
		}
	case SPELL_HIT_AND_RUN:
		{
			p_ptr->special_attack |= (ATTACK_FLEE);

			/* Redraw the state */
			p_ptr->redraw |= (PR_STATUS);

			break;
		}
	case SPELL_POLYMORPH_OTHER:
		{
			(void) poly_monster(dir);
			break;
		}
	case SPELL_EARTHQUAKE:
		{
			earthquake(py, px, 12, FALSE);
			break;
		}
	case SPELL_BEGUILING:
		{
			(void) slow_monsters(5 * plev / 3);
			(void) confu_monsters(5 * plev / 3);
			(void) sleep_monsters(5 * plev / 3);
			break;
		}
	case SPELL_FOCUS:
		{
			if (!p_ptr->timed[TMD_FOCUS])
			{
				(void) inc_timed(TMD_FOCUS, 10 + randint1(10), TRUE);
			} else {
				(void) inc_timed(TMD_FOCUS, randint1(8), TRUE);
			}
			
			break;
		}
	case SPELL_MAJOR_RECHARGE:
		{
			recharge(220);
			break;
		}
	case SPELL_CLOUD_KILL:
		{
			fire_ball(GF_POIS, dir, 10 + plev, 3, FALSE);
			fire_ball(GF_ACID, dir, 2 * plev, 2, FALSE);
			break;
		}
	case SPELL_FORCE_BEAM:
		{
			fire_beam(GF_FORCE, dir, 3 * plev);
			break;
		}
	case SPELL_METEOR_SWARM:
		{
			fire_ball(GF_METEOR, dir, 80 + (plev * 2), 1, FALSE);
			break;
		}
	case SPELL_ROYAL_CANTERLOT_VOICE:
		{
			(void) cacophony(plev + damroll(3, plev));
			break;
		}
	case SPELL_WALL_OF_FORCE:
		{
			fire_arc(GF_FORCE, dir, 4 * plev, 3 + plev / 15, 60);
			break;
		}
	case SPELL_RUNE_OF_THE_ELEMENTS:
		{
			lay_rune(RUNE_ELEMENTS);
			break;
		}
	case SPELL_RUNE_OF_MAGIC_INFLUENCE:
		{
			lay_rune(RUNE_MAGDEF);
			break;
		}
	case SPELL_EXPLOSIVE_RUNE:
		{
			lay_rune(RUNE_EXPLOSIVE);
			break;
		}
	case SPELL_RUNE_OF_INSTABILITY:
		{
			lay_rune(RUNE_QUAKE);
			break;
		}
	case SPELL_RUNE_OF_MANA:
		{
			lay_rune(RUNE_MANA);
			break;
		}
	case SPELL_RUNE_OF_PROTECTION:
		{
			lay_rune(RUNE_PROTECT);
			break;
		}
	case SPELL_RUNE_OF_POWER:
		{
			lay_rune(RUNE_POWER);
			break;
		}
	case SPELL_RUNE_OF_SPEED:
		{
			lay_rune(RUNE_SPEED);
			break;
		}

		/* Holy Prayers */

	case PRAYER_DETECT_CHAOS:
		{
			(void) detect_monsters_evil(DETECT_RAD_DEFAULT, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_CURE_MINOR_WOUNDS:
		{
			(void) hp_player(damroll(2, plev / 4 + 5));
			(void) dec_timed(TMD_CUT, 10, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_CELESTIAS_BLESSING:
		{
			if (!p_ptr->timed[TMD_BLESSED]) {
				(void) inc_timed(TMD_BLESSED, randint1(12) + 12, TRUE);
			} else {
				(void) inc_timed(TMD_BLESSED, randint1(4) + 4, TRUE);
			}
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_REMOVE_FEAR:
		{
			(void) clear_timed(TMD_AFRAID, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_SUNLANCE:
		{
			fire_bolt_or_beam(beam, GF_SUN, dir, damroll(1 + (plev / 8), 6));
			break;
		}
	case PRAYER_PURIFY:
		{
			(void) clear_timed(TMD_POISONED, TRUE);
			(void) clear_timed(TMD_CUT, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_RESIST_FIRE:
		{
			(void) inc_timed(TMD_OPP_FIRE, randint1(30) + 2 * plev / 3, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_HEROISM:
		{
			(void) hp_player(20);
			if (!p_ptr->timed[TMD_HERO]) {
				(void) inc_timed(TMD_HERO, randint1(30) + 2 * plev / 3, TRUE);
			} else {
				(void) inc_timed(TMD_HERO, randint1(20) + 10, TRUE);
			}
			(void) clear_timed(TMD_AFRAID, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);

			break;
		}
	case PRAYER_CURE_MEDIUM_WOUNDS:
		{
			(void) hp_player(damroll(4, plev / 4 + 6));
			(void) dec_timed(TMD_CUT, p_ptr->timed[TMD_CUT] + 5, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_ABSORB_LIGHT:
		{
			int count = unlight_room(p_ptr->py, p_ptr->px);
			(void) hp_player(damroll(count, (plev / 6) + 3));
			break;
		}
	case PRAYER_DETECT_UNDEAD:
		{
			(void) detect_monsters_undead(DETECT_RAD_DEFAULT, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_SATISFY_HUNGER:
		{
			(void) set_food(PY_FOOD_MAX - 1);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_TURN_UNDEAD:
		{
			(void) turn_undead((3 * plev / 2) + 10);
			break;
		}
	case PRAYER_SENSE_MINDS:
		{
			if(!p_ptr->timed[TMD_TELEPATHY])
			{
				inc_timed(TMD_TELEPATHY, randint1(20)+20, TRUE);
			} else {
				inc_timed(TMD_TELEPATHY, randint1(10), TRUE);
			}
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_REMOVE_CURSE:
		{
			if (remove_curse())
				msg("You feel kindly hands aiding you.");
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_EULOGY:
		{
			(void) dispel_undead(damroll(3, plev));
			break;
		}	
	case PRAYER_SOLAR_RAY:
		{
			fire_beam(GF_SUN, dir, plev / 2 + (plev / (player_has(PF_STRONG_MAGIC) ? 2 : 4)));
			break;
		}
	case PRAYER_DISPEL_NIGHT:
		{
			dispel_undead(randint1(3 * plev));
			dispel_demons(randint1(3 * plev));
			(void) turn_evil(plev * 2);
			(void) light_area(randint1(1 + (plev/4)), 1 + (plev / 10));
			break;
		}
	case PRAYER_TORCH:
		{
			fire_ball(GF_FIRE, dir, damroll(2, plev / 2) + plev / 5, 0, TRUE);
			fire_ball(GF_LIGHT_WEAK, dir, randint1(plev / 5) + (plev / ((player_has(PF_STRONG_MAGIC)) ? 3 : 5)), 2, TRUE);
			break;
		}
	case PRAYER_BRIGHTEN:
		{
			brighten(damroll(plev / 5, 6));
			break;
		}
	case PRAYER_CURE_MORTAL_WOUNDS:
		{
			(void) hp_player(damroll(9, plev / 3 + 12));
			(void) clear_timed(TMD_STUN, TRUE);
			(void) clear_timed(TMD_CUT, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_SOLAR_FLARE:
		{
			fire_arc(GF_SUN, dir, damroll(1 + plev / 5, 8), 3 + plev / 15, 60);
			break;
		}
	case PRAYER_JUDGMENT:
		{
			fire_beam(GF_FORCE_CHAOS, dir, 3 * plev / 2);
			break;
		}
	case PRAYER_FINAL_REST:
		{
			fire_ball(GF_KILL_UNDEAD, dir, 1, 0, TRUE);
			break;
		}
	case PRAYER_HEAL:
		{
			(void) hp_player(300);
			(void) clear_timed(TMD_STUN, TRUE);
			(void) clear_timed(TMD_CUT, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_BREAKING:
		{
			breaking(dir);
			break;
		}
	case PRAYER_HOLDING:
		{
			/* Spell will hold any monster or door in one square. */
			fire_ball(GF_HOLD, dir, 0, 0, FALSE);

			break;
		}
	case PRAYER_BANISHMENT:
		{
			if (banish_evil(80)) {
				msg("The power of friendship banishes chaos!");
			}
			break;
		}
	case PRAYER_HORN_OF_WRATH:
		{
			fire_ball(GF_SOUND, 0, plev + randint1(60 + 2 * plev), 1 + plev / 10, FALSE);
			break;
		}
	case PRAYER_HEALING:
		{
			(void) hp_player(700);
			(void) clear_timed(TMD_STUN, TRUE);
			(void) clear_timed(TMD_CUT, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_RENEWING_DAWN:
		{
			(void) light_area(randint1(1 + (plev/8)), 1 + (plev / 10));
			(void) dispel_undead(damroll(5, plev));
			(void) undispel_living(damroll(5, plev));
			break;
		}
	case PRAYER_CELESTIAS_GRACE:
		{
			(void) dispel_monsters(4 * plev);
			fire_ball(GF_HOLY_ORB, 0, 2 * plev, 3, FALSE);
			(void) hp_player(400);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_BLINK:
		{
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			teleport_player(10, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_TELEPORT_SELF:
		{
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			teleport_player(plev * 4, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_PORTAL:
		{
			msg("Choose a location in sight to teleport to.");
			message_flush();
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			dimen_door_los();
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_WORD_OF_RECALL:
		{
			if (!word_recall(randint0(20) + 15))
				break;
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_TELEPORT_LEVEL:
		{
			(void) teleport_player_level(TRUE);
			break;
		}
	case PRAYER_TELEPORT_OTHER:
		{
			(void) teleport_monster(dir, 45 + (plev / 3));
			break;
		}
	case PRAYER_ALTER_REALITY:
		{
			if (MODE(IRONMAN))
				msg("Nothing happens.");
			else {
				msg("The world changes!");

				/* Leaving */
				p_ptr->leaving = TRUE;
			}

			break;
		}
	case PRAYER_DETECT_DOORS_STAIRS:
		{
			/* Hack - 'show' effected region only with the first detect */
			(void) detect_doors(DETECT_RAD_DEFAULT, TRUE);
			(void) detect_stairs(DETECT_RAD_DEFAULT, FALSE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_PERCEPTION:
		{
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			if (!ident_spell())
				return FALSE;
			break;
		}
	case PRAYER_PROBING:
		{
			(void) probing();
			break;
		}
	case PRAYER_SACRED_KNOWLEDGE:
		{
			(void) identify_fully();
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_DETECTION:
		{
			(void) detect_all(DETECT_RAD_DEFAULT, TRUE);
			break;
		}
	case PRAYER_CLAIRVOYANCE:
		{
			wiz_light(FALSE);
			break;
		}
	case PRAYER_SACRED_SHIELD:
		{
			if (!p_ptr->timed[TMD_SHIELD]) {
				(void) inc_timed(TMD_SHIELD, randint1(20) + plev / 2,
								 TRUE);
			} else {
				(void) inc_timed(TMD_SHIELD, randint1(10) + plev / 4,
								 TRUE);
			}
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_RIGHTEOUS_FURY:
		{
			int dur = (2 * plev / 5) + randint1(2 * plev / 5);
			if(!p_ptr->timed[TMD_HERO])
			{
				(void) inc_timed(TMD_HERO, dur, TRUE);
			} else {
				(void) inc_timed(TMD_HERO, dur / 3, TRUE);
			}
			if(!p_ptr->timed[TMD_BLESSED])
			{
				(void) inc_timed(TMD_BLESSED, dur, TRUE);
			} else {
				(void) inc_timed(TMD_BLESSED, dur / 3, TRUE);
			}
			if(!p_ptr->timed[TMD_SHERO])
			{
				(void) inc_timed(TMD_SHERO, dur, TRUE);
			} else {
				(void) inc_timed(TMD_SHERO, dur / 3, TRUE);
			}
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_CURING:
		{
			clear_timed(TMD_BLIND, TRUE);
			clear_timed(TMD_POISONED, TRUE);
			clear_timed(TMD_CONFUSED, TRUE);
			clear_timed(TMD_STUN, TRUE);
			clear_timed(TMD_CUT, TRUE);
			clear_timed(TMD_AFRAID, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_STEADFAST:
		{
			if(!p_ptr->timed[TMD_STEADFAST])
			{
				inc_timed(TMD_STEADFAST, 20, TRUE);
			} else {
				msg("Nothing happens");
			}
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_SUSTAINING:
		{
			int dur = 20 + randint1(20);
			if(!p_ptr->timed[TMD_SWIS])
			{
			    inc_timed(TMD_SWIS, dur, TRUE);
			} else {
				inc_timed(TMD_SWIS, dur / 3, TRUE);
			}
			if (plev > 35)
			{
				if(!p_ptr->timed[TMD_SCON])
				{
				    inc_timed(TMD_SCON, dur, TRUE);
				} else {
					inc_timed(TMD_SCON, dur / 3, TRUE);
				}
			}
			if (plev > 38)
			{
				if(!p_ptr->timed[TMD_SDEX])
				{
				    inc_timed(TMD_SDEX, dur, TRUE);
				} else {
					inc_timed(TMD_SDEX, dur / 3, TRUE);
				}
			}
			if (plev > 41)
			{
				if(!p_ptr->timed[TMD_SINT])
				{
				    inc_timed(TMD_SINT, dur, TRUE);
				} else {
					inc_timed(TMD_SINT, dur / 3, TRUE);
				}
			}
			if (plev > 44)
			{
				if(!p_ptr->timed[TMD_SSTR])
				{
				    inc_timed(TMD_SSTR, dur, TRUE);
				} else {
					inc_timed(TMD_SSTR, dur / 3, TRUE);
				}
			}
			if (plev > 47)
			{
				if(!p_ptr->timed[TMD_SCHA])
				{
				    inc_timed(TMD_SCHA, dur, TRUE);
				} else {
					inc_timed(TMD_SCHA, dur / 3, TRUE);
				}
			}
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_RESTORATION:
		{
			(void) do_res_stat(A_STR);
			(void) do_res_stat(A_INT);
			(void) do_res_stat(A_WIS);
			(void) do_res_stat(A_DEX);
			(void) do_res_stat(A_CON);
			(void) do_res_stat(A_CHR);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_REMEMBERANCE:
		{
			(void) restore_level();
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_GLORY:
		{
			int dur = 50 + randint1(50);
			if(!p_ptr->timed[TMD_HERO])
			{
				inc_timed(TMD_HERO, dur, TRUE);
			} else {
				inc_timed(TMD_HERO, dur / 3, TRUE);
			}
			if(!p_ptr->timed[TMD_BLESSED])
			{
				inc_timed(TMD_BLESSED, dur, TRUE);
			} else {
				inc_timed(TMD_BLESSED, dur / 3, TRUE);
			}
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_RESIST_HEAT_COLD:
		{
			int dur = plev / 2 + randint1(10);
			if(!p_ptr->timed[TMD_OPP_FIRE])
			{
				inc_timed(TMD_OPP_FIRE, dur, TRUE);
			} else {
				inc_timed(TMD_OPP_FIRE, dur / 3, TRUE);
			}
			if(!p_ptr->timed[TMD_OPP_COLD])
			{
				inc_timed(TMD_OPP_COLD, dur, TRUE);
			} else {
				inc_timed(TMD_OPP_COLD, dur / 3, TRUE);
			}
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_PROTECTION_FROM_CHAOS:
		{
			if (!p_ptr->timed[TMD_PROTEVIL]) {
				(void) inc_timed(TMD_PROTEVIL, randint1(24) + 3 * plev / 2,
								 TRUE);
			} else {
				(void) inc_timed(TMD_PROTEVIL, randint1(30), TRUE);
			}
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_GLYPH_OF_WARDING:
		{
			(void) lay_rune(RUNE_PROTECT);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_RALLY_ALLY:
		{
			fire_ball(GF_RALLY, dir, 25 + damroll(plev / 2, 10), 0, FALSE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}
	case PRAYER_CIRCLE_OF_HARMONY:
		{
			(void) create_harmony_land(plev / 15);
			break;
		}
	case PRAYER_SANCTIFY_FOR_BATTLE:
		{
			/* Can't have black breath and holy attack (doesn't happen anyway) */
			if (p_ptr->special_attack & ATTACK_BLKBRTH)
				p_ptr->special_attack &= ~ATTACK_BLKBRTH;

			if (!(p_ptr->special_attack & ATTACK_HOLY))
				msg("Your blows will strike with Holy might!");
			p_ptr->special_attack |= (ATTACK_HOLY);

			/* Redraw the state */
			p_ptr->redraw |= (PR_STATUS);
			
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);

			break;
		}
    case PRAYER_CHALLENGE:
    	{
    		fire_ball(GF_CHALLENGE, dir, 0, 0, FALSE);
    		break;
    	}
   	case PRAYER_CONCENTRATE_LIGHT:
   		{
			int count = unlight_room(p_ptr->py, p_ptr->px);
			fire_ball(GF_LIGHT, dir, count * damroll(plev / 10, 4), 0,  TRUE);
			break;
		}
    case PRAYER_HOLY_WORD:
    	{
			(void) dispel_evil(randint1(plev * 4));
			(void) hp_player(300);
			(void) clear_timed(TMD_AFRAID, TRUE);
			(void) dec_timed(TMD_POISONED, 200, TRUE);
			(void) clear_timed(TMD_STUN, TRUE);
			(void) clear_timed(TMD_CUT, TRUE);
			sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
			break;
		}   
	case PRAYER_RADIANCE:
	    {
		    fire_ball(GF_LIGHT, 0, 5 * plev, plev / 6, FALSE);
		    (void) hp_player(200);
		    (void) fear_monsters(2 * plev);
		    sqinfo_on(cave_info[p_ptr->py][p_ptr->px], SQUARE_GLOW);
		    break;
		}
	case PRAYER_ANNIHILATE_UNDEAD:
		{
			remove_undead(2 *plev / 3);
			break;
		}
	case SPELL_DETECT_TREASURE:
		{
			/* Hack - 'show' affected region only with the first detect */
			(void) detect_treasure(DETECT_RAD_DEFAULT, TRUE);
			(void) detect_objects_gold(DETECT_RAD_DEFAULT, FALSE);
			(void) detect_objects_normal(DETECT_RAD_DEFAULT, TRUE);
			break;
		}


		/* Nature Magics */

	case LORE_LIGHTNING_SPARK:
		{
			if (!get_rep_dir(&dir))
				break;
			fire_arc(GF_ELEC, dir, damroll(2 + (plev / 8), 3),
					 (1 + plev / 5), 0);
			break;
		}
	case LORE_NATURE_AWARENESS:
		{
			(void) detect_nature_awareness(DETECT_RAD_MAP, TRUE);
			break;
		}
	case LORE_GRAZE:
		{
			(void) set_food(PY_FOOD_MAX - 1);
			break;
		}
	case LORE_NATURAL_LIGHT:
		{
			(void) natural_light(damroll(4, 3 + plev/ 10), 8);
			break;
		}
	case LORE_CLEAN_WOUND:
		{
			(void) clear_timed(TMD_POISONED, TRUE);
			(void) clear_timed(TMD_CUT, TRUE);
			break;
		}
	case LORE_RESIST_WEATHER:
		{
			int dur = 20 + randint1(20);
			if(!p_ptr->timed[TMD_OPP_COLD])
			{
				inc_timed(TMD_OPP_COLD, dur, TRUE);
			} else {
				inc_timed(TMD_OPP_COLD, dur / 3, TRUE);
			}
			if(!p_ptr->timed[TMD_OPP_ELEC])
			{
				inc_timed(TMD_OPP_ELEC, dur, TRUE);
			} else {
				inc_timed(TMD_OPP_ELEC, dur / 3, TRUE);
			}
			break;
		}
	case LORE_STONE_TO_TREE:
		{
			(void) wall_to_tree(dir);
			break;
		}
	case LORE_FROST_BOLT:
		{
			fire_bolt_or_beam(beam_low, GF_COLD, dir, damroll(2 + plev / 5, 8));
			break;
		}
	case LORE_RESIST_BURNING_CORROSION:
		{
			int dur = 20 + randint1(20);
			if(!p_ptr->timed[TMD_OPP_FIRE])
			{
				inc_timed(TMD_OPP_FIRE, dur, TRUE);
			} else {
				inc_timed(TMD_OPP_FIRE, dur / 3, TRUE);
			}
			if(!p_ptr->timed[TMD_OPP_ACID])
			{
				inc_timed(TMD_OPP_ACID, dur, TRUE);
			} else {
				inc_timed(TMD_OPP_ACID, dur / 3, TRUE);
			}
			break;
		}
	case LORE_FOREST_CREATION:
		{
			grow_trees_and_grass(FALSE);
			break;
		}
	case LORE_FLOOD:
		{
			fire_ball(GF_WATER, dir, 10 + plev, 1 + plev / 11, FALSE);
			break;
		}
	case LORE_ENTANGLE:
		{
			slow_monsters(5 * plev / 3);
			break;
		}
	case LORE_HERBAL_HEALING:
		{
			(void) hp_player(damroll(25 + plev / 2, 12));
			(void) clear_timed(TMD_CUT, TRUE);
			(void) dec_timed(TMD_POISONED, 200, TRUE);
			break;
		}
	case LORE_SUNBOLT:
		{
			(void) fire_bolt(GF_SUN, dir, randint1(3 * plev));
			break;
		}
	case LORE_CRUSHING_VINES:
		{
			(void) dispel_slowed(damroll(2, plev));
			(void) dispel_stunned(damroll(3, plev));
			(void) dispel_root_para(damroll(4, plev));
			break;
		}
	case LORE_ICE_BEAM:
		{
			fire_beam(GF_ICE, dir, 5 + plev);
			break;
		}
	case LORE_FRIGHTEN_CREATURE:
		{
			fear_monster(dir, plev + 15);
			break;
		}
	case LORE_ROOT:
		{
			root_monster(dir, plev + 15);
			break;
		}
	case LORE_FREEDOM_OF_MOVEMENT:
		{
			if(!p_ptr->timed[TMD_FREEACT])
			{
				inc_timed(TMD_FREEACT, 20 + randint1(20), TRUE);
			} else {
				inc_timed(TMD_FREEACT, randint1(10), TRUE);
			}
			break;
		}
	case LORE_CAMOUFLAGE:
		{
			if(!p_ptr->timed[TMD_SSTEALTH])
			{
				inc_timed(TMD_SSTEALTH, 40, TRUE);
			} else {
				inc_timed(TMD_SSTEALTH, 15, TRUE);
			}
			break;
		}
	case LORE_BARKSKIN:
		{
			if(!p_ptr->timed[TMD_SHIELD])
			{
				inc_timed(TMD_SHIELD, 30 + randint1(30), TRUE);
			} else {
				inc_timed(TMD_SHIELD, randint1(20), TRUE);
			}
			break;
		}
	case LORE_WOOD_WALL:
		{
			(void) wood_wall(1);
			break;
		}
	case LORE_HEROISM:
		{
			(void) hp_player(20);
			if (!p_ptr->timed[TMD_HERO]) {
				(void) inc_timed(TMD_HERO, randint1(30) + plev / 3, TRUE);
			} else {
				(void) inc_timed(TMD_HERO, randint1(10) + plev / 9, TRUE);
			}
			(void) clear_timed(TMD_AFRAID, TRUE);

			break;
		}
    case LORE_NATURE_LORE:
    	{
    		(void) nature_lore();
    		break;
    	}
   	case LORE_SPEED_OF_THE_CHEETAH:
   		{
   			if(!(p_ptr->timed[TMD_FAST]))
   			{
   				inc_timed(TMD_FAST, randint1(30) + 2 * plev / 3, TRUE);
   			} else {
   				inc_timed(TMD_FAST, randint1(20), TRUE);
   			}
   			break;
   		}
	case LORE_WALL_OF_STONE:
		{
			(void) create_wall();
			break;
		}
	case LORE_SNUFF_SMALL_LIFE:
		{
			(void) dispel_small_monsters(5 + plev / 5);
			break;
		}
	case LORE_CALM_ANIMALS:
		{
			sleep_animals(plev + 25);
			break;
		}
	case LORE_ICE_PATCH:
		{
			fire_ball(GF_ICE, 0, plev + damroll(2, plev), 2 + plev / 11, TRUE);
			break;
		}
	case LORE_STARBURST:
		{
			fire_sphere(GF_LIGHT, 0, 40 + 3 * plev / 2 + randint1(3 * plev), plev / 12, 20);
			break;
		}
	case LORE_SLEEP_POLLEN:
		{
			sleep_monsters(plev + 10);
			break;
		}
	case LORE_THORNS:
		{
			nature_strike(damroll(3, plev));
			break;
		}
	case LORE_CYCLONE:
		{
			fire_ball(GF_STRONG_WIND, dir, 2 * + 10, plev / 15, FALSE);
			break;
		}
	case LORE_POLYMORPH_MOUSE:
		{
			shape = SHAPE_MOUSE;
			break;
		}
	case LORE_POLYMORPH_FERRET:
		{
			shape = SHAPE_FERRET;
			break;
		}
	case LORE_POLYMORPH_HOUND:
		{
			shape = SHAPE_HOUND;
			break;
		}
	case LORE_POLYMORPH_GAZELLE:
		{
			shape = SHAPE_GAZELLE;
			break;
		}
	case LORE_POLYMORPH_LION:
		{
			shape = SHAPE_LION;
			break;
		}
	case LORE_POLYMORPH_TREE:
		{
			shape = SHAPE_ENT;
		}
	case LORE_INFRAVISION:
		{
			if(!p_ptr->timed[TMD_SINFRA])
			{
				inc_timed(TMD_SINFRA, plev + 10, TRUE);
			} else {
				inc_timed(TMD_SINFRA, plev / 2 + 5, TRUE);
			}
			break;
		}
	case LORE_CALL_LIGHT:
		{
			(void) light_area(damroll(2, 1 + (plev / 4)), (plev / 10) + 1);
			break;
		}
	case LORE_DETECT_LIFE:
		{
			(void) detect_monsters_living(DETECT_RAD_DEFAULT, TRUE);
			break;
		}
	case LORE_DETECT_TRAPS_DOORS:
		{
			(void) detect_traps(DETECT_RAD_DEFAULT, FALSE);
			(void) detect_doors(DETECT_RAD_DEFAULT, FALSE);
			(void) detect_stairs(DETECT_RAD_DEFAULT, TRUE);
			break;
		}
	case LORE_CREATURE_KNOWLEDGE:
		{
			(void) probing();
			break;
		}
	case LORE_SIGHT_BEYOND_SIGHT:
		{
			/* Hack - 'show' effected region only with the first detect */
			(void) detect_monsters_normal(DETECT_RAD_DEFAULT, TRUE);
			(void) detect_monsters_invis(DETECT_RAD_DEFAULT, FALSE);
			(void) detect_traps(DETECT_RAD_DEFAULT, FALSE);
			(void) detect_doors(DETECT_RAD_DEFAULT, FALSE);
			(void) detect_stairs(DETECT_RAD_DEFAULT, FALSE);
			if (!p_ptr->timed[TMD_SINVIS]) {
				(void) inc_timed(TMD_SINVIS, randint1(24) + 24, TRUE);
			} else {
				(void) inc_timed(TMD_SINVIS, randint1(12) + 12, TRUE);
			}
			break;
		}
	case LORE_TREMOR:
		{
			if (!tremor())
				return FALSE;
			break;
		}
	case LORE_ELEMENTAL_BRANDING:
		{
			if (!choose_ele_attack())
				return FALSE;
			break;
		}
	case LORE_CHAIN_LIGHTNING:
		{
			chain_lightning(GF_ELEC, dir, plev + randint1(30 + 5 * plev / 6));
			break;
		}
	case LORE_EARTHQUAKE:
		{
			earthquake(py, px, 10, FALSE);
			break;
		}
	case LORE_BLIZZARD:
		{
			fire_ball(GF_COLD, dir, plev + randint1(50 + plev * 2),
					  1 + plev / 12, FALSE);
			break;
		}
	case LORE_TRIGGER_TSUNAMI:
		{
			msg("You hurl mighty waves at your foes!");
			fire_sphere(GF_WATER, 0,
						30 + ((4 * plev) / 5) + randint1(30 + plev * 2),
						plev / 7, 20);
			break;
		}
	case LORE_SLEEP_CREATURE:
		{
			(void) sleep_monster(dir, plev + 10);
			break;
		}
	case LORE_NATURE_FRIEND:
		{
			(void) summon_animals();
			break;
		}
	case LORE_ANIMAL_TAMING:
		{
			(void) charm_animal(dir, plev + 20);
			break;
		}
	case LORE_HEAL_ANIMALS:
		{
			heal_animals(damroll(4, plev / 4 + 7));
			break;
		}
	case LORE_STARE:
		{
			if(!charm_monster(dir, plev + 15))
			{
				if(!stun_monster(dir, plev + 15))
				{
					(void) fear_monster(dir, plev + 15);
				}
			}
			break;
		}
	case LORE_NATURAL_RENEWAL:
		{
			(void) do_res_stat(A_STR);
			(void) do_res_stat(A_INT);
			(void) do_res_stat(A_WIS);
			(void) do_res_stat(A_DEX);
			(void) do_res_stat(A_CON);
			(void) do_res_stat(A_CHR);
			(void) restore_level();
			break;
		}
	case LORE_NATURAL_VITALITY:
		{
			(void) dec_timed(TMD_POISONED,
							 (3 * p_ptr->timed[TMD_POISONED] / 4) + 5,
							 TRUE);
			(void) hp_player(damroll(2, plev / 5));
			(void) dec_timed(TMD_CUT, p_ptr->timed[TMD_CUT] + plev / 2,
							 TRUE);
			break;
		}
	case LORE_WITHER_FOE:
		{
			(void) confuse_monster(dir, plev + 10);
			(void) slow_monster(dir, plev + 10);
			fire_bolt(GF_HOLY_ORB, dir, damroll(plev / 7, 8));
			break;
		}
	case LORE_TELEPORT_MONSTER:
		{
			(void) teleport_monster(dir, 45 + (plev / 3));
			break;
		}
	case LORE_WOOD_WALK:
		{
			msg("Choose a tree to teleport to.");
			message_flush();
			wood_walk();
			break;
		}
	case LORE_VOLCANO:
		{
			
			msg("The earth convulses and erupts in fire!");
			earthquake(py, px, 10, TRUE);
			break;
		}
	case LORE_NATURES_WRATH:
		{
			nature_strike(damroll(8, 8) + plev / 2);
			break;
		}

		/* Misc Rogue Spells */

	
	case SPELL_SLIP_INTO_SHADOWS:
		{
			if (!p_ptr->timed[TMD_SSTEALTH]) {
				(void) inc_timed(TMD_SSTEALTH, 40, TRUE);
			} else {
				(void) inc_timed(TMD_SSTEALTH, randint1(20), TRUE);
			}
			break;
		}
	case SPELL_SENSE_INVISIBLE:
		{
			if (!p_ptr->timed[TMD_SINVIS]) {
				inc_timed(TMD_SINVIS, 20 + randint1(plev / 2), TRUE);
			} else {
				inc_timed(TMD_SINVIS, 10 + randint1(plev / 4), TRUE);
			}
			break;
		}
	case SPELL_MAGIC_MAPPING:
		{
			map_area(0, 0, FALSE);
			break;
		}

	default:					/* No Spell */
		{
			msg("Undefined Spell");
			break;
		}
	}

	/* Alter shape, if necessary. */
	if (shape)
		shapechange(shape);

	/* Success */
	return (TRUE);

}

/**
 * Count the total number of abilities the player has
 */
int player_ability_count(void)
{
   int i, count = 0;
   
   for(i = 0; i < z_info->ability_max; i++)
   {
       if(player_contains(ability_info[i].pflags)) {
           count++;
       }
   }
   
   return count;
}

/**
 * Use an ability
 */
bool ability_use(int index, int dir, int plev)
{
	
	if(!plev)
	    plev = p_ptr->lev;
    switch(index) {
	    case ABILITY_TELEKINESIS:
        {
			s16b ty, tx;
			if (!target_set_interactive(TARGET_OBJ | TARGET_GRID, -1, -1))
				return FALSE;
			target_get(&tx, &ty);
			if (!py_pickup(1, ty, tx))
				return FALSE;
			break;
		}
		case ABILITY_BR_FIRE:
        {
            fire_arc(GF_FIRE, dir, p_ptr->chp / 3, 4, 20);
            break;
        }
        case ABILITY_DETECT_MONSTERS:
        {
            (void) detect_monsters_normal(DETECT_RAD_DEFAULT, TRUE);
			break;
		}
		case ABILITY_DETECT_TREASURE:
        {
			/* Hack - 'show' affected region only with the first detect */
			(void) detect_treasure(DETECT_RAD_DEFAULT, TRUE);
			(void) detect_objects_gold(DETECT_RAD_DEFAULT, FALSE);
			(void) detect_objects_normal(DETECT_RAD_DEFAULT, TRUE);
			break;
		}
    }
    /* Success */
	return (TRUE);
}

/* See if an ability needs to be aimed */
bool ability_needs_aim(int ability)
{
    switch(ability){
    case ABILITY_TELEKINESIS:
    case ABILITY_BR_FIRE:
        return TRUE;
    default:
        return FALSE;
    }
}
