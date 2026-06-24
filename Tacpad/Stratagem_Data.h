#pragma once
#include <Arduino.h>

#define MAX_STRATAGEMS 120
#define MAX_LOADOUTS 20
#define LOADOUT_NAME_MAXLEN 24

struct Stratagem {
  uint8_t id;
  char name[36];
  char border[8];
  char command[12];
  char img_path[28];
  bool icon_exists;   // checked once at load time, not on every grid refresh
};

struct Loadout {
  char name[LOADOUT_NAME_MAXLEN];
  uint8_t ids[9];
};

extern Loadout loadouts[MAX_LOADOUTS];
extern uint8_t loadout_count;

bool Stratagem_LoadAllLoadouts(const char* path);
bool Stratagem_SelectLoadout(uint8_t index);


extern Stratagem stratagems[MAX_STRATAGEMS];
extern uint8_t stratagem_count;
extern uint8_t grid_loadout[9];   // grid slot -> stratagem ID, loaded from SD

bool Stratagem_LoadCSV(const char* path);
bool Stratagem_LoadLoadout(const char* path);
bool Stratagem_ParseLoadoutString(const char* str);
Stratagem* Stratagem_GetById(uint8_t id);