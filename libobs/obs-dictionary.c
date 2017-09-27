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

#include "obs-dictionary.h"

/* All this does internally is store a subobject inside the main dictionary
 * object associated with the entry name. The subobject contains "type"
 * and "value" properties which these helper APIs look up.
 */

#define S_TYPE "type"
#define S_VALUE "value"

EXPORT obs_data_item_t *obs_dictionary_setup_entry(obs_data_t *data,
	const char *name)
{
	obs_data_t *entry = obs_data_create();
	obs_data_set_obj(data, name, entry);

	return obs_data_item_byname(data, name);
}

void obs_dictionary_set_string(obs_data_item_t *entry, const char *value)
{
	obs_data_t *entry_obj = obs_data_item_get_obj(entry);
	obs_data_set_int(entry_obj, S_TYPE, OBS_DICTIONARY_ENTRY_STRING);
	obs_data_set_string(entry_obj, S_VALUE, value);
}
void obs_dictionary_set_int(obs_data_item_t *entry, long long value)
{
	obs_data_t *entry_obj = obs_data_item_get_obj(entry);
	obs_data_set_int(entry_obj, S_TYPE, OBS_DICTIONARY_ENTRY_INT);
	obs_data_set_int(entry_obj, S_VALUE, value);
}

void obs_dictionary_set_float(obs_data_item_t *entry, double value)
{
	obs_data_t *entry_obj = obs_data_item_get_obj(entry);
	obs_data_set_int(entry_obj, S_TYPE, OBS_DICTIONARY_ENTRY_FLOAT);
	obs_data_set_double(entry_obj, S_VALUE, value);
}

void obs_dictionary_set_bool(obs_data_item_t *entry, bool value)
{
	obs_data_t *entry_obj = obs_data_item_get_obj(entry);
	obs_data_set_int(entry_obj, S_TYPE, OBS_DICTIONARY_ENTRY_BOOL);
	obs_data_set_bool(entry_obj, S_VALUE, value);
}

void obs_dictionary_set_color(obs_data_item_t *entry, long long value)
{
	obs_data_t *entry_obj = obs_data_item_get_obj(entry);
	obs_data_set_int(entry_obj, S_TYPE, OBS_DICTIONARY_ENTRY_COLOR);
	obs_data_set_int(entry_obj, S_VALUE, value);
}

enum obs_dictionary_entry_type obs_dictionary_get_type(obs_data_item_t *entry)
{
	obs_data_t *entry_obj = obs_data_item_get_obj(entry);
	return obs_data_get_int(entry_obj, S_TYPE);
}

const char *obs_dictionary_get_string(obs_data_item_t *entry)
{
	obs_data_t *entry_obj = obs_data_item_get_obj(entry);
	return obs_data_get_string(entry_obj, S_VALUE);
}

long long obs_dictionary_get_int(obs_data_item_t *entry)
{
	obs_data_t *entry_obj = obs_data_item_get_obj(entry);
	return obs_data_get_int(entry_obj, S_VALUE);
}

double obs_dictionary_get_float(obs_data_item_t *entry)
{
	obs_data_t *entry_obj = obs_data_item_get_obj(entry);
	return obs_data_get_double(entry_obj, S_VALUE);
}

bool obs_dictionary_get_bool(obs_data_item_t *entry)
{
	obs_data_t *entry_obj = obs_data_item_get_obj(entry);
	return obs_data_get_bool(entry_obj, S_VALUE);
}

long long obs_dictionary_get_color(obs_data_item_t *entry)
{
	obs_data_t *entry_obj = obs_data_item_get_obj(entry);
	return obs_data_get_int(entry_obj, S_VALUE);
}
