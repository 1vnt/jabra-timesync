#include <jabra/Common.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define DEVICES_LENGTH 32
#define SLEEP_DURATION 60 * 60 * 24 // 24 hours

typedef struct device_t {
  unsigned short id;
  char *name;
} Device;

Device *devices[DEVICES_LENGTH];
static volatile bool running = true;

static void shutdown() {
  running = false;
  for (int i = 0; i < DEVICES_LENGTH; i++) {
    if (devices[i] == NULL)
      continue;
    free(devices[i]);
  }

  Jabra_Uninitialize();
}

static void updateTime(unsigned short deviceID) {
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  timedate_t td = {timeinfo->tm_sec,  timeinfo->tm_min, timeinfo->tm_hour,
                   timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year,
                   timeinfo->tm_wday};
  Jabra_SetDateTime(deviceID, &td);
}

static void updateTimes() {
  puts("Sync times");
  for (int i = 0; i < DEVICES_LENGTH; i++) {
    if (devices[i] == NULL)
      continue;
    updateTime(devices[i]->id);
  }
}

void intHandler(int dummy) {
  alarm(0);
  shutdown();
}
void alrmHandler(int dummy) {
  updateTimes();
  alarm(SLEEP_DURATION);
}

static void deviceAttachedHandler(Jabra_DeviceInfo deviceInfo) {
  printf("%i: Attached (%s)\n", deviceInfo.deviceID, deviceInfo.deviceName);
  if (!Jabra_IsSetDateTimeSupported(deviceInfo.deviceID)) {
    printf("%i: No support for setting Date/Time\n", deviceInfo.deviceID);
  }
  Device *dev = malloc(sizeof(Device));
  dev->id = deviceInfo.deviceID;
  dev->name = deviceInfo.deviceName;
  devices[deviceInfo.deviceID] = dev;
  updateTime(dev->id);
}

static void deviceRemovedHandler(unsigned short deviceID) {
  printf("%i: Removed\n", deviceID);
  if (devices[deviceID] != NULL)
    free(devices[deviceID]);
  devices[deviceID] = NULL;
}

int main() {
  signal(SIGINT, intHandler);
  signal(SIGALRM, alrmHandler);

  puts("Starting Jabra timesync demon");

  Jabra_SetAppID("49d8-feecf6d7-f352-40e7-8924-607427445679");

  bool initialized =
      Jabra_InitializeV2(NULL, deviceAttachedHandler, deviceRemovedHandler,
                         NULL, NULL, true, NULL);

  if (!initialized) {
    puts("Library failed to initialize!");
    return -1;
  }

  // Set initial alarm
  alarm(SLEEP_DURATION);

  while (running)
    pause();

  return 0;
}
