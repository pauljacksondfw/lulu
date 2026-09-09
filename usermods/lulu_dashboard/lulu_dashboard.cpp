#include "wled.h"

/*
 * LULU Dashboard usermod v4
 *
 * Stores five dashboard buttons on the controller:
 * - label
 * - WLED preset number
 * - icon
 *
 * Button 5 defaults to Emergency / preset 5.
 * Existing v3 labels/presets remain compatible. If no icon data exists yet,
 * built-in defaults are used until the next save.
 */

class LULUDashboardUsermod : public Usermod {
  private:
    static const char _name[];

    static constexpr uint8_t BUTTON_COUNT = 5;
    static constexpr uint8_t LABEL_LENGTH = 25; // 24 chars + terminator
    static constexpr uint8_t ICON_LENGTH = 13;  // enough for common UTF-8 emoji

    char labels[BUTTON_COUNT][LABEL_LENGTH] = {
      "Preset 1",
      "Preset 2",
      "Preset 3",
      "Preset 4",
      "Emergency"
    };

    uint8_t presets[BUTTON_COUNT] = {1, 2, 3, 4, 5};

    char icons[BUTTON_COUNT][ICON_LENGTH] = {
      "✨",
      "🌅",
      "🌌",
      "🌊",
      "⚠️"
    };

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

    bool setIcon(uint8_t index, const char* value) {
      if (index >= BUTTON_COUNT || value == nullptr || value[0] == '\0') return false;

      char next[ICON_LENGTH];
      strlcpy(next, value, sizeof(next));

      if (strncmp(icons[index], next, ICON_LENGTH) == 0) return false;

      strlcpy(icons[index], next, ICON_LENGTH);
      return true;
    }

    void queueSave() {
      savePending = true;
      saveAfter = millis() + 500;
    }

  public:
    void setup() override {}

    void loop() override {
      if (!savePending) return;
      if ((long)(millis() - saveAfter) < 0) return;

      savePending = false;
      configNeedsWrite = true;
    }

    void addToJsonState(JsonObject& root) override {
      JsonObject lulu = root.createNestedObject(FPSTR(_name));
      JsonArray buttons = lulu.createNestedArray(F("buttons"));

      for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        JsonObject button = buttons.createNestedObject();
        button[F("label")] = labels[i];
        button[F("preset")] = presets[i];
        button[F("icon")] = icons[i];
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
          changed |= setLabel(index, button[F("label")].as<const char*>());
        }

        if (button.containsKey(F("preset"))) {
          changed |= setPreset(index, button[F("preset")].as<int>());
        }

        if (button.containsKey(F("icon"))) {
          changed |= setIcon(index, button[F("icon")].as<const char*>());
        }

        index++;
      }

      if (changed) queueSave();
    }

    void addToConfig(JsonObject& root) override {
      JsonObject lulu = root.createNestedObject(FPSTR(_name));
      JsonArray savedLabels = lulu.createNestedArray(F("labels"));
      JsonArray savedPresets = lulu.createNestedArray(F("presets"));
      JsonArray savedIcons = lulu.createNestedArray(F("icons"));

      for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        savedLabels.add(labels[i]);
        savedPresets.add(presets[i]);
        savedIcons.add(icons[i]);
      }
    }

    uint16_t getId() override {
      return USERMOD_ID_UNSPECIFIED;
    }

    bool readFromConfig(JsonObject& root) override {
      JsonObject lulu = root[FPSTR(_name)];
      if (lulu.isNull()) return true;

      JsonArray savedLabels = lulu[F("labels")].as<JsonArray>();
      JsonArray savedPresets = lulu[F("presets")].as<JsonArray>();
      JsonArray savedIcons = lulu[F("icons")].as<JsonArray>();

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

      if (!savedIcons.isNull()) {
        for (uint8_t i = 0; i < BUTTON_COUNT && i < savedIcons.size(); i++) {
          const char* value = savedIcons[i].as<const char*>();
          if (value && value[0]) strlcpy(icons[i], value, ICON_LENGTH);
        }
      }

      return true;
    }
};

const char LULUDashboardUsermod::_name[] PROGMEM = "LULU";

static LULUDashboardUsermod luluDashboardUsermod;
REGISTER_USERMOD(luluDashboardUsermod);
