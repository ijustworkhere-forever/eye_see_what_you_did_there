#include "PreferencesStore.h"

#ifdef ARDUINO

namespace eyesee {

bool PreferencesStore::begin(const char* namespaceName) {
    return preferences_.begin(namespaceName, false);
}

bool PreferencesStore::getUInt16(const char* key, uint16_t& outValue) const {
    if (!preferences_.isKey(key)) {
        return false;
    }
    outValue = preferences_.getUShort(key);
    return true;
}

bool PreferencesStore::putUInt16(const char* key, uint16_t value) {
    return preferences_.putUShort(key, value) > 0;
}

bool PreferencesStore::getInt16(const char* key, int16_t& outValue) const {
    if (!preferences_.isKey(key)) {
        return false;
    }
    outValue = preferences_.getShort(key);
    return true;
}

bool PreferencesStore::putInt16(const char* key, int16_t value) {
    return preferences_.putShort(key, value) > 0;
}

bool PreferencesStore::getBool(const char* key, bool& outValue) const {
    if (!preferences_.isKey(key)) {
        return false;
    }
    outValue = preferences_.getBool(key);
    return true;
}

bool PreferencesStore::putBool(const char* key, bool value) {
    return preferences_.putBool(key, value) > 0;
}

bool PreferencesStore::getFloat(const char* key, float& outValue) const {
    if (!preferences_.isKey(key)) {
        return false;
    }
    outValue = preferences_.getFloat(key);
    return true;
}

bool PreferencesStore::putFloat(const char* key, float value) {
    return preferences_.putFloat(key, value) > 0;
}

void PreferencesStore::end() {
    preferences_.end();
}

}  // namespace eyesee

#endif  // ARDUINO
