#pragma once
#include <Arduino.h>

#define MAX_STRATAGEMS 120

struct Stratagem {
  uint8_t id;
  char name[36];
  char border[8];
  char command[12];
  char img_path[28];   // precomputed "S:/images/<filename>" — see note below
};

extern Stratagem stratagems[MAX_STRATAGEMS];
extern uint8_t stratagem_count;
extern uint8_t grid_loadout[9];   // grid slot -> stratagem ID, loaded from SD

bool Stratagem_LoadCSV(const char* path);
bool Stratagem_LoadLoadout(const char* path);
Stratagem* Stratagem_GetById(uint8_t id);