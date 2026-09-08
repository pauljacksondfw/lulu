#include "wled.h"

/*
 * LULU Dashboard usermod
 *
 * Stores four dashboard button labels and preset assignments in WLED cfg.json.
 * The web dashboard reads/writes the values through WLED's /json/state API.
 *
 * Persistent cfg.json shape:
 * "um": {
 *   "LULU": {
 *     "labels":  ["Preset 1", "Preset 2", "Preset 3", "Preset 4"],
 *     "presets": [1, 2, 3, 4]
 *   }
 * }
 */

class LULUDashboardUsermod : public Usermod {
  private:
    static const char _name[];

    static constexpr uint8_t BUTTON_COUNT = 4;
    static constexpr uint8_t LABEL_LENGTH = 25; // 24 characters + null terminator

    char labels[BUTTON_COUNT][LABEL_LENGTH] = {
      "Preset 1",
      "Preset 2",
      "Preset 3",
      "Preset 4"
    };

    uint8_t presets[BUTTON_COUNT] = {1, 2, 3, 4};

    bool savePending = false;
    unsigned long saveAfter = 0;

    bool setLabel(uint8_t index, const char* value) {
      if (index >= BUTTON_COUNT || value == nullptr || value[0] == '\0') return false;

      char next[LABEL_LENGTH];
      strlcpy(next, value, sizeof(next));

      if (strncmp(labels[index], next, LABEL_LENGTH) == 0) return false;

      strlcpy(labels[index], next, LABEL_LENGTH);
      return true;
    }

    bool setPreset(uint8_t index, int value) {
      if (index >= BUTTON_COUNT || value < 1 || value > 250) return false;

      uint8_t next = static_cast<uint8_t>(value);
      if (presets[index] == next) return false;

      presets[index] = next;
      return true;
    }

    void queueSave() {
      savePending = true;
      saveAfter = millis() + 500;
    }

  public:
    void loop() override {
      if (!savePending) return;
      if ((long)(millis() - saveAfter) < 0) return;

      // WLED usermod guidance requires serializeConfig() to be called
      // from loop(), not from the network/JSON callback.
      savePending = false;
      serializeConfig();
    }

    void addToJsonState(JsonObject& root) override {
      JsonObject lulu = root.createNestedObject(FPSTR(_name));
      JsonArray buttons = lulu.createNestedArray(F("buttons"));

      for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        JsonObject button = buttons.createNestedObject();
        button[F("label")] = labels[i];
        button[F("preset")] = presets[i];
      }
    }

    void readFromJsonState(JsonObject& root) override {
      JsonObject lulu = root[FPSTR(_name)];
      if (lulu.isNull()) return;

      JsonArray buttons = lulu[F("buttons")].as<JsonArray>();
      if (buttons.isNull()) return;

      bool changed = false;
      uint8_t index = 0;

      for (JsonObject button : buttons) {
        if (index >= BUTTON_COUNT) break;

        if (button.containsKey(F("label"))) {
          const char* label = button[F("label")].as<const char*>();
          changed |= setLabel(index, label);
        }

        if (button.containsKey(F("preset"))) {
          changed |= setPreset(index, button[F("preset")].as<int>());
        }

        index++;
      }

      if (changed) queueSave();
    }

    void addToConfig(JsonObject& root) override {
      JsonObject lulu = root.createNestedObject(FPSTR(_name));
      JsonArray savedLabels = lulu.createNestedArray(F("labels"));
      JsonArray savedPresets = lulu.createNestedArray(F("presets"));

      for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        savedLabels.add(labels[i]);
        savedPresets.add(presets[i]);
      }
    }

    bool readFromConfig(JsonObject& root) override {
      JsonObject lulu = root[FPSTR(_name)];

      // Keep built-in defaults if this is the first boot with the usermod.
      // The first dashboard save will persist them.
      if (lulu.isNull()) return true;

      JsonArray savedLabels = lulu[F("labels")].as<JsonArray>();
      JsonArray savedPresets = lulu[F("presets")].as<JsonArray>();

      if (!savedLabels.isNull()) {
        for (uint8_t i = 0; i < BUTTON_COUNT && i < savedLabels.size(); i++) {
          const char* value = savedLabels[i].as<const char*>();
          if (value && value[0]) strlcpy(labels[i], value, LABEL_LENGTH);
        }
      }

      if (!savedPresets.isNull()) {
        for (uint8_t i = 0; i < BUTTON_COUNT && i < savedPresets.size(); i++) {
          int value = savedPresets[i].as<int>();
          if (value >= 1 && value <= 250) presets[i] = static_cast<uint8_t>(value);
        }
      }

      return true;
    }
};

const char LULUDashboardUsermod::_name[] PROGMEM = "LULU";

static LULUDashboardUsermod luluDashboardUsermod;
REGISTER_USERMOD(luluDashboardUsermod);
