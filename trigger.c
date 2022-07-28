#include "trigger.h"

struct trigger {
  uint pin;
  bool debounce;
  uint frequency;
  void (*callback)();
};

Trigger_t trigger_init(
  uint pin,
  uint frequency,
  void (*callback)
) {

}