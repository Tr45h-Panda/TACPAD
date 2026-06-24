#include "Stratagem_Data.h"
#include "SD_Card.h"

Stratagem stratagems[MAX_STRATAGEMS];
uint8_t stratagem_count = 0;
uint8_t grid_loadout[9] = {1,2,3,4,5,6,7,8,9};   // fallback until a real loadout loads

bool Stratagem_LoadCSV(const char* path)
{
  File f = SD_MMC.open(path);
  if (!f) {
    printf("Stratagem : could not open %s\r\n", path);
    return false;
  }

  f.readStringUntil('\n');   // skip header row
  stratagem_count = 0;

  while (f.available() && stratagem_count < MAX_STRATAGEMS) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    int p1 = line.indexOf(',');
    int p2 = line.indexOf(',', p1 + 1);
    int p3 = line.indexOf(',', p2 + 1);
    int p4 = line.indexOf(',', p3 + 1);
    if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0) {
      printf("Stratagem : skipping malformed CSV line: %s\r\n", line.c_str());
      continue;
    }

    Stratagem &s = stratagems[stratagem_count];
    s.id = (uint8_t)line.substring(0, p1).toInt();
    strlcpy(s.name,    line.substring(p1 + 1, p2).c_str(), sizeof(s.name));
    strlcpy(s.border,  line.substring(p2 + 1, p3).c_str(), sizeof(s.border));
    strlcpy(s.command, line.substring(p3 + 1, p4).c_str(), sizeof(s.command));

    String filename = line.substring(p4 + 1);
    snprintf(s.img_path, sizeof(s.img_path), "S:/images/%s", filename.c_str());
    s.icon_exists = File_Search("/images", filename.c_str());

    stratagem_count++;
  }
  f.close();
  printf("Stratagem : loaded %d entries from %s\r\n", stratagem_count, path);
  return stratagem_count > 0;
}

bool Stratagem_ParseLoadoutString(const char* str)
{
  String line(str);
  line.trim();

  int start = 0;
  for (int i = 0; i < 9; i++) {
    int comma = line.indexOf(',', start);
    String tok = (comma == -1) ? line.substring(start) : line.substring(start, comma);
    int id = tok.toInt();

    if (id < 1 || id > stratagem_count) {
      printf("Stratagem : loadout slot %d ('%s') invalid, defaulting to ID 1\r\n", i, tok.c_str());
      id = 1;
    }
    grid_loadout[i] = (uint8_t)id;

    if (comma == -1) break;
    start = comma + 1;
  }
  printf("Stratagem : loadout updated: %s\r\n", str);
  return true;
}

bool Stratagem_LoadLoadout(const char* path)
{
  File f = SD_MMC.open(path);
  if (!f) {
    printf("Stratagem : could not open loadout file %s, using default 1-9\r\n", path);
    return false;
  }
  String line = f.readStringUntil('\n');
  f.close();
  return Stratagem_ParseLoadoutString(line.c_str());
}

Loadout loadouts[MAX_LOADOUTS];
uint8_t loadout_count = 0;

bool Stratagem_LoadAllLoadouts(const char* path)
{
  File f = SD_MMC.open(path);
  if (!f) {
    printf("Stratagem : could not open %s\r\n", path);
    return false;
  }

  loadout_count = 0;
  while (f.available() && loadout_count < MAX_LOADOUTS) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    int comma = line.indexOf(',');
    if (comma < 0) {
      printf("Stratagem : skipping malformed loadout line: %s\r\n", line.c_str());
      continue;
    }

    Loadout &lo = loadouts[loadout_count];
    strlcpy(lo.name, line.substring(0, comma).c_str(), sizeof(lo.name));

    String rest = line.substring(comma + 1);
    int start = 0;
    for (int i = 0; i < 9; i++) {
      int c2 = rest.indexOf(',', start);
      String tok = (c2 == -1) ? rest.substring(start) : rest.substring(start, c2);
      int id = tok.toInt();
      if (id < 1 || id > stratagem_count) {
        printf("Stratagem : loadout '%s' slot %d ('%s') invalid, using 1\r\n", lo.name, i, tok.c_str());
        id = 1;
      }
      lo.ids[i] = (uint8_t)id;
      if (c2 == -1) break;
      start = c2 + 1;
    }

    printf("Stratagem : loaded preset '%s'\r\n", lo.name);
    loadout_count++;
  }
  f.close();
  printf("Stratagem : %d named loadouts loaded from %s\r\n", loadout_count, path);
  return loadout_count > 0;
}

bool Stratagem_SelectLoadout(uint8_t index)
{
  if (index >= loadout_count) {
    printf("Stratagem : invalid loadout index %d\r\n", index);
    return false;
  }
  memcpy(grid_loadout, loadouts[index].ids, sizeof(grid_loadout));
  printf("Stratagem : selected loadout '%s'\r\n", loadouts[index].name);
  return true;
}

Stratagem* Stratagem_GetById(uint8_t id)
{
  if (id < 1 || id > stratagem_count) return NULL;
  return &stratagems[id - 1];   // IDs are sequential 1..N matching array index
}