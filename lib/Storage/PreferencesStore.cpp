#include "PreferencesStore.h"

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

void PreferencesStore::end() {
    preferences_.end();
}

}  // namespace eyesee
