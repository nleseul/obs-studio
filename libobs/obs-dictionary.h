/******************************************************************************
Copyright (C) 2014 by Hugh Bailey <obs.jim@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#pragma once

#include "util/c99defs.h"
#include "obs-data.h"

#ifdef __cplusplus
extern "C" {
#endif

enum obs_dictionary_entry_type {
	OBS_DICTIONARY_ENTRY_STRING,
	OBS_DICTIONARY_ENTRY_INT,
	OBS_DICTIONARY_ENTRY_FLOAT,
	OBS_DICTIONARY_ENTRY_BOOL,
	OBS_DICTIONARY_ENTRY_COLOR
};

/* Helper API for formatting the data storage used in dictionary-type
 * properties.
 *
 * Dictionaries are basically a wrapper around obs_data stores which retain
 * user-configurable type information. This is different from the internal
 * types used in the obs_data system because a dictionary entry type preserves
 * information about how the user interacts with a value (e.g., a color value is
 * different from a just number). It also differs from the obs_properties
 * mechanism because property typing is transient while an editor window is
 * open, and cannot be modified or stored by the user.
 *
 * To use this, you should have an existing obs_data structure which stores
 * the entire dictionary. You can then call obs_dictionary_setup_entry() on that
 * structure to add an entry with a specified name. That entry structure (which
 * is an obs_data_item_t) can then be interacted with through the
 * obs_dictionary_set*() and obs_dictionary_get*() APIs.
 */

EXPORT obs_data_item_t *obs_dictionary_setup_entry(obs_data_t *data,
	const char *name);

EXPORT void obs_dictionary_set_string(obs_data_item_t *entry, const char *value);
EXPORT void obs_dictionary_set_int(obs_data_item_t *entry, long long value);
EXPORT void obs_dictionary_set_float(obs_data_item_t *entry, double value);
EXPORT void obs_dictionary_set_bool(obs_data_item_t *entry, bool value);
EXPORT void obs_dictionary_set_color(obs_data_item_t *entry, long long value);

EXPORT enum obs_dictionary_entry_type obs_dictionary_get_type(
	obs_data_item_t *entry);

EXPORT const char *obs_dictionary_get_string(obs_data_item_t *entry);
EXPORT long long obs_dictionary_get_int(obs_data_item_t *entry);
EXPORT double obs_dictionary_get_float(obs_data_item_t *entry);
EXPORT bool obs_dictionary_get_bool(obs_data_item_t *entry);
EXPORT long long obs_dictionary_get_color(obs_data_item_t *entry);

#ifdef __cplusplus
}
#endif
